/**
 * @file slow5.h
 * @brief SLOW5 API
 * @author Sasha Jenner (jenner.sasha@gmail.com), Hasindu Gamaarachchi (hasindu@garvan.org.au)
 * @date 27/02/2021
 */

// Header with slow5 file definitions
// TODO structure pack to min size
// TODO fix and add function descriptions

#ifndef SLOW5_H
#define SLOW5_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "klib/khash.h"
#include "klib/kvec.h"
#include "slow5_press.h"
#include "slow5_defs.h"
#include "slow5_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************
 ***  struct definitions **************************************************************************
 **************************************************************************************************/

/**
* @enum slow5_fmt
* File formats we are dealing with
*/
enum slow5_fmt {
    FORMAT_UNKNOWN, ///< the format is unknown, usually the case before parsing the file extension
    FORMAT_ASCII,   ///< the format is ASCII SLOW5
    FORMAT_BINARY   ///< the format is binary SLOW5 (that is, BLOW5)
};

/**
* @struct slow5_fmt_meta
* SLOW5 file meta data
*/
struct slow5_fmt_meta {
    const char *name;       ///< path of the SLOW5 file
    enum slow5_fmt format;  ///< the format of the SLOW5 file
};

static const struct slow5_fmt_meta SLOW5_FORMAT_META[] = {
    { ASCII_NAME,   FORMAT_ASCII    },
    { BINARY_NAME,  FORMAT_BINARY   }
};

/*** SLOW5 Header *********************************************************************************/

/**
* @struct slow5_version
* @brief SLOW5 file version
*/
struct slow5_version {
    uint8_t major;  ///< major version
    uint8_t minor;  ///< minor version
    uint8_t patch;  ///< patch version
};

// TODO is this ok or put somewhere else or get rid of?
static const struct slow5_version ASCII_VERSION_STRUCT = { .major = 0, .minor = 1, .patch = 0 };
static const struct slow5_version BINARY_VERSION_STRUCT = { .major = 0, .minor = 1, .patch = 0 };

// SLOW5 auxiliary types
// DO NOT rearrange! See subtracting INT8_T_ARRAY in TO_PRIM_TYPE
// if adding more in future, primitive types must be added after CHAR and arrays after STRING
// both the primitive type and the array type must be simultaneously added
enum aux_type {
    INT8_T = 0,
    INT16_T,
    INT32_T,
    INT64_T,
    UINT8_T,
    UINT16_T,
    UINT32_T,
    UINT64_T,
    FLOAT,
    DOUBLE,
    CHAR,

    INT8_T_ARRAY,
    INT16_T_ARRAY,
    INT32_T_ARRAY,
    INT64_T_ARRAY,
    UINT8_T_ARRAY,
    UINT16_T_ARRAY,
    UINT32_T_ARRAY,
    UINT64_T_ARRAY,
    FLOAT_ARRAY,
    DOUBLE_ARRAY,
    STRING
};

#define IS_PTR(type)        (type >= INT8_T_ARRAY)
#define TO_PRIM_TYPE(type)  ((enum aux_type) (type - INT8_T_ARRAY))

//NULL (missing value) representation
#define INT8_T_NULL     INT8_MAX
#define INT16_T_NULL    INT16_MAX
#define INT32_T_NULL    INT32_MAX
#define INT64_T_NULL    INT64_MAX
#define UINT8_T_NULL    UINT8_MAX
#define UINT16_T_NULL   UINT16_MAX
#define UINT32_T_NULL   UINT32_MAX
#define UINT64_T_NULL   UINT64_MAX
#define FLOAT_NULL      nanf("")
#define DOUBLE_NULL     nan("")
#define CHAR_NULL       0


// Type with corresponding size
struct aux_type_meta {
    enum aux_type type;
    uint8_t size;
    const char *type_str;
};

//any modifications to aux_type should follow by appropriate modifications to this.
//the order should be identical to that in aux_type
static const struct aux_type_meta AUX_TYPE_META[] = {
    // Needs to be the same order as the enum definition
    { INT8_T,           sizeof (int8_t),        "int8_t"    },
    { INT16_T,          sizeof (int16_t),       "int16_t"   },
    { INT32_T,          sizeof (int32_t),       "int32_t"   },
    { INT64_T,          sizeof (int64_t),       "int64_t"   },
    { UINT8_T,          sizeof (uint8_t),       "uint8_t"   },
    { UINT16_T,         sizeof (uint16_t),      "uint16_t"  },
    { UINT32_T,         sizeof (uint32_t),      "uint32_t"  },
    { UINT64_T,         sizeof (uint64_t),      "uint64_t"  },
    { FLOAT,            sizeof (float),         "float"     },
    { DOUBLE,           sizeof (double),        "double"    },
    { CHAR,             sizeof (char),          "char"      },

    { INT8_T_ARRAY,     sizeof (int8_t),        "int8_t*"   },
    { INT16_T_ARRAY,    sizeof (int16_t),       "int16_t*"  },
    { INT32_T_ARRAY,    sizeof (int32_t),       "int32_t*"  },
    { INT64_T_ARRAY,    sizeof (int64_t),       "int64_t*"  },
    { UINT8_T_ARRAY,    sizeof (uint8_t),       "uint8_t*"  },
    { UINT16_T_ARRAY,   sizeof (uint16_t),      "uint16_t*" },
    { UINT32_T_ARRAY,   sizeof (uint32_t),      "uint32_t*" },
    { UINT64_T_ARRAY,   sizeof (uint64_t),      "uint64_t*" },
    { FLOAT_ARRAY,      sizeof (float),         "float*"    },
    { DOUBLE_ARRAY,     sizeof (double),        "double*"   },
    { STRING,           sizeof (char),          "char*"     }
};

// Auxiliary attribute to position map: attribute string -> index position
KHASH_MAP_INIT_STR(s2ui32, uint32_t)

/**
* @struct slow5_aux_meta
* SLOW5 auxiliary field metadata (information avilable in the SLOW5 header)
*/
struct slow5_aux_meta {
    uint32_t num;                   ///< number of auxiliary fields
    size_t cap;                     ///< capacity of the arrays: attrs, types and sizes

    khash_t(s2ui32) *attr_to_pos;   ///< hash table that maps field name string -> index position in the following arrays.
    char **attrs;                   ///< field names
    enum aux_type *types;           ///< field datatype
    uint8_t *sizes;                 ///< field datatype sizes, for arrays this stores the size (in bytes) of the corresponding primitive type (TODO: this is probably redundant)
};
typedef struct slow5_aux_meta slow5_aux_meta_t;

// Header data map: attribute string -> data string
KHASH_MAP_INIT_STR(s2s, char *)
// Header data attributes set
KHASH_SET_INIT_STR(s)

/**
* @struct slow5_hdr_data
* SLOW5 header data (constant attributes in FAST5 files)
*/
struct slow5_hdr_data {
    uint32_t num_attrs;	            ///< Number of data attributes
    khash_t(s) *attrs;              ///< Set of the data attribute keys (incase of multiple read groups, the union of keys from all read groups)
    kvec_t(khash_t(s2s) *) maps;    ///< Dynamic vector of hash maps (attribute key string -> attribute value string). Length of the vector is requal to  num_read_groups. Index in the vector corresponds to the read group number. The keys that are not relevant to a particular read group are not stored in this hash map.
};
typedef struct slow5_hdr_data slow5_hdr_data_t;

/**
* @struct slow5_hdr
* SLOW5 header
*/
struct slow5_hdr {
	struct slow5_version version;       ///< SLOW5 file version
    uint32_t num_read_groups;           ///< Number of read groups
    slow5_hdr_data_t data;              ///< Header data (constant fields in FAST5 files). Not to be directly accessed, use provided functions instead.
    slow5_aux_meta_t *aux_meta;         ///< Auxiliary field metadata. Not to be directly accessed, use provided functions instead.
};
typedef struct slow5_hdr slow5_hdr_t;

/*** SLOW5 record *********************************************************************************/

// SLOW5 primary record columns stored as an enum to keep  the order of the columns.
// TODO: make the first one is set to zero
enum slow5_cols {
    SLOW5_COLS_FOREACH(GENERATE_ENUM)
    SLOW5_COLS_NUM
};

/**
* @struct slow5_rec_aux_data
* SLOW5 auxiliary field data (represents a single SLOW5 auxiliary field of a particular read record)
*/
struct slow5_rec_aux_data {
    uint64_t len;       ///< number of elements in a array (if a primitive type this is always 1)
    uint64_t bytes;     ///< total number of bytes in data (currently, the allocated size which is equal to the amount of data in it)
    enum aux_type type; ///< data type of the auxiliary attribute
    uint8_t *data;      ///< raw data
};

// Header data map: auxiliary attribute string -> auxiliary data
KHASH_MAP_INIT_STR(s2a, struct slow5_rec_aux_data)

typedef uint64_t slow5_rec_size_t; //size of the whole record (in bytes)
typedef uint16_t slow5_rid_len_t;  //length of the read ID string (does not include null character)
typedef khash_t(s2a) slow5_aux_data_t;  //Auxiliary field name string -> auxiliary field data value

/**
* @struct slow5_rec
* SLOW5 record data struct (represents a single SLOW5 record)
*/
struct slow5_rec {
    slow5_rid_len_t read_id_len;        ///< length of the read ID string (does not include null character)
    SLOW5_COLS_FOREACH(GENERATE_STRUCT) ///< macro magic that generates the primary fields (see below)
                                        ///< char* read_id;
                                        ///< uint32_t read_group;
                                        ///< double digitisation;
                                        ///< double offset;
                                        ///< double range;
                                        ///< double sampling_rate;
                                        ///< uint64_t len_raw_signal;
                                        ///< int16_t* raw_signal;
    slow5_aux_data_t *aux_map;               ///< Auxiliary field name string -> auxiliary field data value. Not to be directly accessed, use provided functions instead.
};
typedef struct slow5_rec slow5_rec_t;

/*** SLOW5 file handler ***************************************************************************/

/**
* @struct slow5_file
* SLOW5 file meta data
*/
struct slow5_file_meta {
    const char *pathname;       ///< file path
    int fd;                     ///< file descriptor
    uint64_t start_rec_offset;  ///< offset (in bytes) of the first SLOW5 record (skipping the SLOW5 header; used for indexing)
};
typedef struct slow5_file_meta slow5_file_meta_t;

typedef struct slow5_idx slow5_idx_t;

/**
* @struct slow5_file
* SLOW5 file structure
*/
struct slow5_file {
    FILE *fp;                   ///< file pointer
    enum slow5_fmt format;      ///< whether SLOW5, BLOW5 etc...
    press_t *compress;          ///< compression related metadata
    slow5_hdr_t *header;        ///< SLOW5 header
    slow5_idx_t *index;         ///< SLOW5 index (NULL if not applicable)
    slow5_file_meta_t meta;     ///< file metadata
};
typedef struct slow5_file slow5_file_t;


/**************************************************************************************************
 ***  High-level API ******************************************************************************
 **************************************************************************************************/

/**
 * Open a slow5 file with a specific mode given it's pathname.
 *
 * Attempt to guess the file's slow5 format from the pathname's extension.
 * Return NULL if pathname or mode is NULL,
 * or if the pathname's extension is not recognised,
 * of if the pathname is invalid.
 *
 * Otherwise, return a slow5 file structure with the header parsed.
 * slow5_close() should be called when finished with the structure.
 *
 * TODO: This function at the moment should only be used for opening a file for reading
 * The user at the moment is expected to give "r" as the mode for ASCII and "rb" for binary
 * This mode should better be programmatically done inside the function and the function be renamed to slow5_open_r to idicate this is only for reading
 *
 * @param   pathname    relative or absolute path to slow5 file
 * @param   mode        same mode as in fopen()
 * @return              slow5 file structure
 */
slow5_file_t *slow5_open(const char *pathname, const char *mode);

/**
 * Close a slow5 file and free its memory.
 *
 * @param   s5p slow5 file structure
 * @return      same as fclose()
 */
int slow5_close(slow5_file_t *s5p);

/**
 * Create the index file for slow5 file.
 * Overwrites if already exists.
 *
 * @param   s5p slow5 file structure
 * @return  0 if successful,  <-1> on error
 */
int slow5_idx_create(slow5_file_t *s5p);

/**
 * Loads the index file for slow5 file.
 * Creates the index if not found.
 *
 * Return -1 on error,
 * 0 on success.
 *
 * @param   s5p slow5 file structure
 * @return  error codes described above
 */
int slow5_idx_load(slow5_file_t *s5p);

void slow5_idx_unload(slow5_file_t *s5p);

/**
 * Get a header data attribute for a particular read_group.
 *
 * Returns NULL if the attribute name doesn't exist
 * or the read group is out of range
 * or an input parameter is NULL.
 *
 * @param   attr        attribute name
 * @param   read_group  the read group
 * @param   header      slow5 header
 * @return  the attribute's value, or NULL on error
 */
char *slow5_hdr_get(const char *attr, uint32_t read_group, const slow5_hdr_t *header);

/**
 * Get a read entry from a slow5 file corresponding to a read_id.
 *
 * Allocates memory for *read if it is NULL.
 * Otherwise, the data in *read is freed and overwritten.
 * slow5_rec_free() should always be called when finished with the structure.
 *
 * Require the slow5 index to be loaded using slow5_idx_load
 *
 * Return
 * TODO are these error codes too much?
 *  0   the read was successfully found and stored
 * -1   read_id, read or s5p is NULL
 * -2   the index has not been loaded
 * -3   read_id was not found in the index
 * -4   reading error when reading the slow5 file
 * -5   parsing error
 *
 * @param   read_id the read identifier
 * @param   read    address of a slow5_rec pointer
 * @param   s5p     slow5 file
 * @return  error code described above
 */

int slow5_get(const char *read_id, slow5_rec_t **read, slow5_file_t *s5p);

/**
 * Get the read entry under the current file pointer of a slow5 file.
 *
 * Allocates memory for *read if it is NULL.
 * Otherwise, the data in *read is freed and overwritten.
 * slow5_rec_free() should always be called when finished with the structure.
 *
 * Return
 * TODO are these error codes too much?
 *  0   the read was successfully found and stored
 * -1   read_id, read or s5p is NULL
 * -2   reading error when reading the slow5 file
 * -3   parsing error
 *
 * @param   read    address of a slow5_rec_t pointer
 * @param   s5p     slow5 file
 * @return  error code described above
 */
int slow5_get_next(slow5_rec_t **read, slow5_file_t *s5p);


/**
 * Free a slow5 record.
 *
 * @param   read    address of a slow5_rec_t pointer
 */
void slow5_rec_free(slow5_rec_t *read);


/**
 * Get an auxiliary field in a SLOW5 record as an 8-bit signed integer.
 *
 * @param   read    address of a slow5_rec_t pointer
 * @param   attr    auxiliary field name
 * @param   err     error code (non zero error code if failed)
 * @return  field data value
 */
int8_t slow5_aux_get_int8(const slow5_rec_t *read, const char *attr, int *err);
int16_t slow5_aux_get_int16(const slow5_rec_t *read, const char *attr, int *err);
int32_t slow5_aux_get_int32(const slow5_rec_t *read, const char *attr, int *err);
int64_t slow5_aux_get_int64(const slow5_rec_t *read, const char *attr, int *err);
uint8_t slow5_aux_get_uint8(const slow5_rec_t *read, const char *attr, int *err);
uint16_t slow5_aux_get_uint16(const slow5_rec_t *read, const char *attr, int *err);
uint32_t slow5_aux_get_uint32(const slow5_rec_t *read, const char *attr, int *err);
uint64_t slow5_aux_get_uint64(const slow5_rec_t *read, const char *attr, int *err);
float slow5_aux_get_float(const slow5_rec_t *read, const char *attr, int *err);
double slow5_aux_get_double(const slow5_rec_t *read, const char *attr, int *err);
char slow5_aux_get_char(const slow5_rec_t *read, const char *attr, int *err);

/**
 * Get an auxiliary field in a SLOW5 record as an 8-bit signed integer array.
 *
 * @param   read    address of a slow5_rec_t pointer
 * @param   attr    auxiliary field name
 * @param   err     error code
 * @return  pointer to the array of data values
 */
int8_t *slow5_aux_get_int8_array(const slow5_rec_t *read, const char *attr, uint64_t *len, int *err);
int16_t *slow5_aux_get_int16_array(const slow5_rec_t *read, const char *attr, uint64_t *len, int *err);
int32_t *slow5_aux_get_int32_array(const slow5_rec_t *read, const char *attr, uint64_t *len, int *err);
int64_t *slow5_aux_get_int64_array(const slow5_rec_t *read, const char *attr, uint64_t *len, int *err);
uint8_t *slow5_aux_get_uint8_array(const slow5_rec_t *read, const char *attr, uint64_t *len, int *err);
uint16_t *slow5_aux_get_uint16_array(const slow5_rec_t *read, const char *attr, uint64_t *len, int *err);
uint32_t *slow5_aux_get_uint32_array(const slow5_rec_t *read, const char *attr, uint64_t *len, int *err);
uint64_t *slow5_aux_get_uint64_array(const slow5_rec_t *read, const char *attr, uint64_t *len, int *err);
float *slow5_aux_get_float_array(const slow5_rec_t *read, const char *attr, uint64_t *len, int *err);
double *slow5_aux_get_double_array(const slow5_rec_t *read, const char *attr, uint64_t *len, int *err);
char *slow5_aux_get_string(const slow5_rec_t *read, const char *attr, uint64_t *len, int *err);




/**************************************************************************************************
 ***  Low-level API *******************************************************************************
 **************************************************************************************************/


/**
 * Open a slow5 file of a specific format with a mode given it's pathname.
 *
 * Return NULL if pathname or mode is NULL, or if the format specified doesn't match the file.
 * slow5_open_with(pathname, mode, FORMAT_UNKNOWN) is equivalent to slow5_open(pathname, mode).
 *
 * Otherwise, return a slow5 file structure with the header parsed.
 * slow5_close() should be called when finished with the structure.
 *
 * TODO: same issues as in slow5_open are applicable to this
 *
 * @param   pathname    relative or absolute path to slow5 file
 * @param   mode        same mode as in fopen()
 * @param   format      format of the slow5 file
 * @return              slow5 file structure
 */
slow5_file_t *slow5_open_with(const char *pathname, const char *mode, enum slow5_fmt format);


/**
 * Get an empty read structure.
 * To be freed with slow5_rec_free().
 *
 * @return  ptr to the record
 */
static inline slow5_rec_t *slow5_rec_init(void) {
    slow5_rec_t *read = (slow5_rec_t *) calloc(1, sizeof *read);

    return read;
}


/**
 * Add a read entry to the SLOW5 file while updating the SLOW5 index (not thread safe).
 *
 * Return
 *  0   the read was successfully stored
 * -1   read or s5p is NULL
 * -2   the index was not previously init and failed to init
 * -3   duplicate read id
 * -4   writing failure
 *
 * @param   read    slow5_rec ptr
 * @param   s5p     slow5 file
 * @return  error code described above
 */
int slow5_add_rec(slow5_rec_t *read, slow5_file_t *s5p);

/**
 * Remove a read entry at a read_id in a slow5 file while updating the SLOW5 index (not thread safe).
 *
 * Return
 *  0   the read was successfully stored
 * -1   an input parameter is NULL
 * -2   the index was not previously init and failed to init
 * -3   read_id was not found in the index
 *
 * @param   read_id the read identifier
 * @param   s5p     slow5 file
 * @return  error code described above
 */
int slow5_rm_rec(const char *read_id, slow5_file_t *s5p); // TODO



/**
 * Get the read entry in the specified format.
 *
 * Returns NULL if read is NULL,
 * or format is FORMAT_UNKNOWN,
 * or the read attribute values are invalid
 *
 * @param   read        slow5_rec pointer
 * @param   format      slow5 format to write the entry in
 * @param   written     number of bytes written to the returned buffer
 * @param   compress    compress structure
 * @return  malloced string to use free() on, NULL on error
 */
void *slow5_rec_to_mem(slow5_rec_t *read, struct slow5_aux_meta *aux_meta, enum slow5_fmt format, struct press *compress, size_t *n);

/**
 * Print a read entry in the specified format to a file pointer.
 *
 * On success, the number of bytes written is returned.
 * On error, -1 is returned.
 *
 * @param   fp      output file pointer
 * @param   read    slow5_rec pointer
 * @param   format  slow5 format to write entry in
 * @param   compress
 * @return  number of bytes written, -1 on error
 */
int slow5_rec_fwrite(FILE *fp, slow5_rec_t *read, struct slow5_aux_meta *aux_meta, enum slow5_fmt format, struct press *compress);
static inline int slow5_rec_print(slow5_rec_t *read, struct slow5_aux_meta *aux_meta, enum slow5_fmt format, struct press *compress) {
    return slow5_rec_fwrite(stdout, read, aux_meta, format, compress);
}


/**
 * Add a new header data attribute.
 *
 * Returns -1 if an input parameter is NULL.
 * Returns -2 if the attribute already exists.
 * Returns 0 other.
 *
 * @param   attr    attribute name
 * @param   header  slow5 header
 * @return  0 on success, <0 on error as described above
 */
int slow5_hdr_add_attr(const char *attr, slow5_hdr_t *header);

/**
 * Add a new header read group.
 *
 * All values are set to NULL for the new read group.
 *
 * Returns -1 if an input parameter is NULL.
 * Returns the new read group number otherwise.
 *
 * @param   header  slow5 header
 * @return  < 0 on error as described above
 */
// TODO check return type but should be large enough to return -1 and the largest read group
int64_t slow5_hdr_add_rg(slow5_hdr_t *header);

/**
 * Set a header data attribute for a particular read_group.
 *
 * Doesn't take memory ownership of the value given.
 *
 * Returns -1 if the attribute name doesn't exist
 * or the read group is out of range
 * or an input parameter is NULL.
 * Returns 0 other.
 *
 * @param   attr        attribute name
 * @param   value       new attribute value
 * @param   read_group  the read group
 * @param   header      slow5 header
 * @return  0 on success, -1 on error
 */
int slow5_hdr_set(const char *attr, const char *value, uint32_t read_group, slow5_hdr_t *header);

/**
 * Get the header in the specified format.
 *
 * Returns NULL if s5p is NULL
 * or format is FORMAT_UNKNOWN
 * or an internal error occurs.
 *
 * @param   header  slow5 header
 * @param   format  slow5 format to write the entry in
 * @param   comp    compression method
 * @param   written number of bytes written to the returned buffer
 * @return  malloced memory storing the slow5 header representation,
 *          to use free() on afterwards
 */
void *slow5_hdr_to_mem(slow5_hdr_t *header, enum slow5_fmt format, press_method_t comp, size_t *written);

/**
 * Print the header in the specified format to a file pointer.
 *
 * On success, the number of bytes written is returned.
 * On error, -1 is returned.
 *
 * @param   fp      output file pointer
 * @param   header  slow5 header
 * @param   format  slow5 format to write the entry in
 * @return  number of bytes written, -1 on error
 */
int slow5_hdr_fwrite(FILE *fp, slow5_hdr_t *header, enum slow5_fmt format, press_method_t comp);
static inline int slow5_hdr_print(slow5_hdr_t *header, enum slow5_fmt format, press_method_t comp) {
    return slow5_hdr_fwrite(stdout, header, format, comp);
}

/**
 * Print the binary end of file to a file pointer.
 *
 * On success, the number of bytes written is returned.
 * On error, -1 is returned.
 *
 * @param   fp      output file pointer
 * @return  number of bytes written, -1 on error
 */
ssize_t slow5_eof_fwrite(FILE *fp);
static inline ssize_t slow5_eof_print(void) {
    return slow5_eof_fwrite(stdout);
}

void slow5_set_log_level(enum slow5_log_level_opt log_level);
void slow5_set_exit_condition(enum slow5_exit_condition_opt exit_condition);


// Return
// 0    success
// -1   input invalid
// -2   failure
int slow5_convert(slow5_file_t *from, FILE *to_fp, enum slow5_fmt to_format, press_method_t to_compress);

// Merge slow5 files to another slow5 file
// TODO Just a merge for 2 -> 1?
// TODO compile time 2 args in ...
int8_t slow5_merge(slow5_file_t *s5p_to, ...); // TODO
int8_t slow5_vmerge(slow5_file_t *s5p_to, va_list ap); // TODO

// Split a slow5 file to a dir
// TODO split into multiple slow5 files from same rg
int8_t slow5_split(const char *dirname_to, slow5_file_t *s5p_from); // TODO


#ifdef __cplusplus
}
#endif


#endif