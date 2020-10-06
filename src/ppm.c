#include <stdlib.h>
#include <stdio.h>

#include "ppm.h"

void compress_ppm(char *inputFile, char *outputFile) {
    FILE *inputFilePtr = (FILE *)fopen(inputFile, "rb");
    FILE *outputFilePtr = (FILE *)fopen(outputFile, "wb");
    
    /** PUT YOUR CODE HERE
      * implement a PPM algorithm for compression
      * don't forget to change header file `ppm.h`
    */
    
    // This is an implementation of simple copying
    size_t n, m;
    unsigned char buff[8192];
    
    do {
        n = fread(buff, 1, sizeof buff, inputFilePtr);
        if (n)
            m = fwrite(buff, 1, n, outputFilePtr);
        else 
            m = 0;
    } while ((n > 0) && (n == m));
    
    fclose(inputFilePtr);
    fclose(outputFilePtr);
}

void decompress_ppm(char *inputFile, char *outputFile) {
    FILE *inputFilePtr = (FILE *)fopen(inputFile, "rb");
    FILE *outputFilePtr = (FILE *)fopen(outputFile, "wb");
    
    /** PUT YOUR CODE HERE
      * implement a PPM algorithm for decompression
      * don't forget to change header file `ppm.h`
    */
    
    // This is an implementation of simple copying
    size_t n, m;
    unsigned char buff[8192];
    
    do {
        n = fread(buff, 1, sizeof buff, inputFilePtr);
        if (n)
            m = fwrite(buff, 1, n, outputFilePtr);
        else 
            m = 0;
    } while ((n > 0) && (n == m));
    
    fclose(inputFilePtr);
    fclose(outputFilePtr);
}
