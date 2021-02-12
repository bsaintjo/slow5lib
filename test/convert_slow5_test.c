#include "unit_test.h"
#include "../src/slow5.h"
#include "../src/press.h"

int slow5_to_blow5_uncomp(void) {
    struct slow5_file *from = slow5_open("test/data/exp/one_fast5/exp_1_default.slow5", "r");
    ASSERT(from != NULL);

    FILE *to = fopen("test/data/out/one_fast5/slow5_to_blow5_uncomp.blow5", "w");
    ASSERT(to != NULL);

    ASSERT(slow5_hdr_fprint(to, from, FORMAT_BINARY, COMPRESS_NONE) != -1);

    struct slow5_rec *read = NULL;
    int ret;
    while ((ret = slow5_get_next(&read, from)) == 0) {
        ASSERT(slow5_rec_fprint(to, read, FORMAT_BINARY, NULL) != -1);
    }
    ASSERT(ret == -2);

    ASSERT(slow5_eof_fprint(to) != -1);

    ASSERT(slow5_close(from) == 0);
    ASSERT(fclose(to) == 0);

    return EXIT_SUCCESS;
}

int slow5_to_blow5_gzip(void) {
    struct slow5_file *from = slow5_open("test/data/exp/one_fast5/exp_1_default.slow5", "r");
    ASSERT(from != NULL);

    FILE *to = fopen("test/data/out/one_fast5/slow5_to_blow5_gzip.blow5", "w");
    ASSERT(to != NULL);

    ASSERT(slow5_hdr_fprint(to, from, FORMAT_BINARY, COMPRESS_GZIP) != -1);

    struct slow5_rec *read = NULL;
    int ret;
    struct press *gzip = press_init(COMPRESS_GZIP);
    ASSERT(gzip != NULL);
    while ((ret = slow5_get_next(&read, from)) == 0) {
        ASSERT(slow5_rec_fprint(to, read, FORMAT_BINARY, gzip) != -1);
    }
    ASSERT(ret == -2);

    ASSERT(slow5_eof_fprint(to) != -1);

    press_free(gzip);
    ASSERT(slow5_close(from) == 0);
    ASSERT(fclose(to) == 0);

    return EXIT_SUCCESS;
}

int blow5_uncomp_to_slow5(void) {
    struct slow5_file *from = slow5_open("test/data/exp/one_fast5/exp_1_default.blow5", "r");
    ASSERT(from != NULL);

    FILE *to = fopen("test/data/out/one_fast5/blow5_uncomp_to_slow5.slow5", "w");
    ASSERT(to != NULL);

    ASSERT(slow5_hdr_fprint(to, from, FORMAT_ASCII, COMPRESS_NONE) != -1);

    struct slow5_rec *read = NULL;
    int ret;
    while ((ret = slow5_get_next(&read, from)) == 0) {
        ASSERT(slow5_rec_fprint(to, read, FORMAT_ASCII, NULL) != -1);
    }
    ASSERT(ret == -2);

    ASSERT(slow5_close(from) == 0);
    ASSERT(fclose(to) == 0);

    return EXIT_SUCCESS;
}

int blow5_gzip_to_slow5(void) {
    struct slow5_file *from = slow5_open("test/data/exp/one_fast5/exp_1_default_gzip.blow5", "r");
    ASSERT(from != NULL);

    FILE *to = fopen("test/data/out/one_fast5/blow5_gzip_to_slow5.slow5", "w");
    ASSERT(to != NULL);

    ASSERT(slow5_hdr_fprint(to, from, FORMAT_ASCII, COMPRESS_NONE) != -1);

    struct slow5_rec *read = NULL;
    int ret;
    while ((ret = slow5_get_next(&read, from)) == 0) {
        ASSERT(slow5_rec_fprint(to, read, FORMAT_ASCII, NULL) != -1);
    }
    ASSERT(ret == -2);

    ASSERT(slow5_close(from) == 0);
    ASSERT(fclose(to) == 0);

    return EXIT_SUCCESS;
}

int blow5_gzip_to_blow5_uncomp(void) {
    struct slow5_file *from = slow5_open("test/data/exp/one_fast5/exp_1_default_gzip.blow5", "r");
    ASSERT(from != NULL);

    FILE *to = fopen("test/data/out/one_fast5/blow5_gzip_to_blow5_uncomp.blow5", "w");
    ASSERT(to != NULL);

    ASSERT(slow5_hdr_fprint(to, from, FORMAT_BINARY, COMPRESS_NONE) != -1);

    struct slow5_rec *read = NULL;
    int ret;
    while ((ret = slow5_get_next(&read, from)) == 0) {
        ASSERT(slow5_rec_fprint(to, read, FORMAT_BINARY, COMPRESS_NONE) != -1);
    }
    ASSERT(ret == -2);

    ASSERT(slow5_eof_fprint(to) != -1);

    ASSERT(slow5_close(from) == 0);
    ASSERT(fclose(to) == 0);

    return EXIT_SUCCESS;
}

int blow5_uncomp_to_blow5_gzip(void) {
    struct slow5_file *from = slow5_open("test/data/exp/one_fast5/exp_1_default.blow5", "r");
    ASSERT(from != NULL);

    FILE *to = fopen("test/data/out/one_fast5/blow5_uncomp_to_blow5_gzip.blow5", "w");
    ASSERT(to != NULL);

    ASSERT(slow5_hdr_fprint(to, from, FORMAT_BINARY, COMPRESS_GZIP) != -1);

    struct slow5_rec *read = NULL;
    int ret;
    struct press *gzip = press_init(COMPRESS_GZIP);
    ASSERT(gzip != NULL);
    while ((ret = slow5_get_next(&read, from)) == 0) {
        ASSERT(slow5_rec_fprint(to, read, FORMAT_BINARY, gzip) != -1);
    }
    ASSERT(ret == -2);

    ASSERT(slow5_eof_fprint(to) != -1);

    press_free(gzip);
    ASSERT(slow5_close(from) == 0);
    ASSERT(fclose(to) == 0);

    return EXIT_SUCCESS;
}


int main(void) {

    struct command tests[] = {
        CMD(slow5_to_blow5_uncomp)
        CMD(slow5_to_blow5_gzip)
        CMD(blow5_uncomp_to_slow5)
        CMD(blow5_gzip_to_slow5)
        CMD(blow5_gzip_to_blow5_uncomp)
        CMD(blow5_uncomp_to_blow5_gzip)
    };

    return RUN_TESTS(tests);
}
