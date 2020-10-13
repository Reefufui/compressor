#pragma once

#include <stdint.h>

//typedef unsigned short ariInt; //16 bit
//typedef uint32_t ariInt; //32 bit
typedef uint64_t ariInt; //64 bit

void compressAri(char *inputFile, char *outputFile);
void decompressAri(char *compressedFile, char *dataFile);

void generateCSV(uint32_t *charList);

typedef enum {
    BASIC_AGRESSION,
    ENGLISH_TEXT,
    DEFAULT
} CharListTemplate;

void touchCharList(uint32_t ***charList, int *prevTableID, CharListTemplate type, uint8_t c);

void writeBit(int bit, FILE **outputFilePtr);

void bitsPlusFollow(ariInt bit, ariInt *bitsToFollow, FILE **outputFilePtr);

int readBit(FILE **compressedFilePtr);