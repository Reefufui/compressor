#pragma once

enum {
    ARI,
    PPM,
    BWT
};

typedef struct CompressOptions {
    char *inputFile;
    char *outputFile;
    char mode;
    int  method;
} CompressOptions;

CompressOptions *parse_args(int argc, char **argv);

CompressOptions *default_config(void);

void print_config(CompressOptions *opts);

void free_compress_opts(CompressOptions *opts);

int can_open_file(const char *filename);
