# slow5lib

## NAME
slow5_idx_unload - Unloads the index file from the memory.

## SYNOPSYS
`void slow5_idx_unload(slow5_file_t *s5p)`

## DESCRIPTION
`slow5_idx_unload()` is used to unload an index file loaded into memory using `slow5_idx_load()`

## RETURN VALUE
No return value

## NOTES
Also see [`slow5_idx_create()`](slow5_idx_create.md) and [`slow5_idx_load()`](slow5_idx_load.md).

## EXAMPLES

```
#include <stdio.h>
#include <stdlib.h>
#include <slow5/slow5.h>

#define FILE_PATH "examples/example.slow5"

#define TO_PICOAMPS(RAW_VAL,DIGITISATION,OFFSET,RANGE) (((RAW_VAL)+(OFFSET))*((RANGE)/(DIGITISATION)))

int main(){

    slow5_file_t *sp = slow5_open(FILE_PATH,"r");
    if(sp==NULL){
       fprintf(stderr,"Error in opening file\n");
       exit(EXIT_FAILURE);
    }
    slow5_rec_t *rec = NULL;
    int ret=0;

    ret = slow5_idx_load(sp);
    if(ret<0){
        fprintf(stderr,"Error in loading index\n");
        exit(EXIT_FAILURE);
    }

    ret = slow5_get("r3", &rec, sp);
    if(ret < 0){
        fprintf(stderr,"Error in locating read\n");
    }
    else{
        printf("%s\t",rec->read_id);
        uint64_t len_raw_signal = rec->len_raw_signal;
        for(uint64_t i=0;i<len_raw_signal;i++){
            double pA = TO_PICOAMPS(rec->raw_signal[i],rec->digitisation,rec->offset,rec->range);
            printf("%f ",pA);
        }
        printf("\n");
    }

    slow5_rec_free(rec);

    slow5_idx_unload(sp);

    slow5_close(sp);

}
```