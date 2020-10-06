#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

CompressOptions *parse_args(int argc, char **argv) {
    char help[] = 
        "Usage\n\n\
          compress.exe [options]\n\n\
        Options\n\n\
          --help                     = Print usage information and exit.\n\
          --input  <file>            = Specify input file to compress/decompress.\n\
          --output <file>            = Specify output file to write a result.\n\
          --mode   {c | d}           = Use specified mode, `c` to compress and `d` to decompress.\n\
          --method {ari | ppm | bwt} = Use specified method of data compression/decompression.\n";
    
    CompressOptions *opts = default_config();
    
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--help")) {
            printf("%s\n", help);
            free_compress_opts(opts);
            return NULL;
        }
        else if (!strcmp(argv[i], "--input")) {
            if (++i < argc) {
                opts->inputFile = realloc(opts->inputFile, sizeof(char) * (strlen(argv[i]) + 1));
                strcpy(opts->inputFile, argv[i]);
                continue;
            }
            else {
                printf("Error: too few arguments\n");
                free_compress_opts(opts);
                return NULL;
            }
        }
        else if (!strcmp(argv[i], "--output")) {
            if (++i < argc) {
                opts->outputFile = realloc(opts->outputFile, sizeof(char) * (strlen(argv[i]) + 1));
                strcpy(opts->outputFile, argv[i]);
                continue;
            }
            else {
                printf("Error: too few arguments\n");
                free_compress_opts(opts);
                return NULL;
            }
        }
        else if (!strcmp(argv[i], "--mode")) {
            if (++i < argc) {
                if (!strcmp(argv[i], "c")) {
                    opts->mode = 'c';
                    continue;
                }
                else if (!strcmp(argv[i], "d")) {
                    opts->mode = 'd';
                    continue;
                }
                else {
                    printf("Error: unknown mode `%s`\n", argv[i]);
                    free_compress_opts(opts);
                    return NULL;
                }
            }
            else {
                printf("Error: too few arguments\n");
                free_compress_opts(opts);
                return NULL;
            }
        }
        else if (!strcmp(argv[i], "--method")) {
            if (++i < argc) {
                if (!strcmp(argv[i], "ari")) {
                    opts->method = ARI;
                    continue;
                }
                else if (!strcmp(argv[i], "ppm")) {
                    opts->method = PPM;
                    continue;
                }
                else if (!strcmp(argv[i], "bwt")) {
                    opts->method = BWT;
                    continue;
                }
                else {
                    printf("Error: unknown method `%s`\n", argv[i]);
                    free_compress_opts(opts);
                    return NULL;
                }
            }
            else {
                printf("Error: too few arguments\n");
                free_compress_opts(opts);
                return NULL;
            }
        }
        else {
            printf("Error: unexpected option `%s`\n", argv[i]);
            free_compress_opts(opts);
            return NULL;
        }
    }
    print_config(opts);
    
    if (!can_open_file(opts->inputFile)) {
        return NULL;
    }
    
    return opts;
}

CompressOptions *default_config(void) {
    const char DEFAULT_INPUT_FILENAME[] = "input.txt";
    const char DEFAULT_OUTPUT_FILENAME[] = "output.txt";
    
    CompressOptions *default_opts = calloc(1, sizeof(*default_opts));
    
    default_opts->inputFile = calloc(sizeof(char), sizeof(DEFAULT_INPUT_FILENAME));
    strcpy(default_opts->inputFile, DEFAULT_INPUT_FILENAME);
    
    default_opts->outputFile = calloc(sizeof(char), sizeof(DEFAULT_OUTPUT_FILENAME));
    strcpy(default_opts->outputFile, DEFAULT_OUTPUT_FILENAME);
    
    default_opts->mode = 'c';
    default_opts->method = ARI;
    
    return default_opts;
}

void print_config(CompressOptions *opts) {
    if (opts != NULL) {
        if (opts->mode == 'c') {
            printf("Compression from `%s` to `%s`\n", opts->inputFile, opts->outputFile);
        }
        else if (opts->mode == 'd') {
            printf("Decompression from `%s` to `%s`\n", opts->inputFile, opts->outputFile);
        }
        
        if (opts->method == ARI) {
            printf("Method: arithmetic encoding\n");
        }
        else if (opts->method == PPM) {
            printf("Method: PPM\n");
        }
        else if (opts->method == BWT) {
            printf("Method: arithmetic encoding with Burrows-Wheeler transform\n");
        }
        
        printf("\n");
    }
}

void free_compress_opts(CompressOptions *opts) {
    if (opts != NULL) {
        if (opts->inputFile != NULL) {
            free(opts->inputFile);
        }
        if (opts->outputFile != NULL) {
            free(opts->outputFile);
        }
        free(opts);
    }
}

int can_open_file(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        return 0;
    }
    
    fclose(f);
    return 1;
}
