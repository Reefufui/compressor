#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "ari.h"
#include "ppm.h"
#include "bwt.h"

int main(int argc, char **argv) {
    CompressOptions *opts = parse_args(argc, argv);
    if (opts != NULL) {
        if (opts->mode == 'c') {
            if (opts->method == ARI) {
                compressAri(opts->inputFile, opts->outputFile);
            }
            else if (opts->method == PPM) {
                compress_ppm(opts->inputFile, opts->outputFile);
            }
            else if (opts->method == BWT) {
                compress_bwt(opts->inputFile, opts->outputFile);
            }
        }
        else if (opts->mode == 'd') {
            if (opts->method == ARI) {
                decompressAri(opts->inputFile, opts->outputFile);
            }
            else if (opts->method == PPM) {
                decompress_ppm(opts->inputFile, opts->outputFile);
            }
            else if (opts->method == BWT) {
                decompress_bwt(opts->inputFile, opts->outputFile);
            }
        }
    }
    free_compress_opts(opts);
    return 0;
}
