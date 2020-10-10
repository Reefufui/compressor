#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "ari.h"

//typedef unsigned short ariInt; //16 bit
//typedef uint32_t ariInt; //32 bit
typedef uint64_t ariInt; //64 bit

typedef enum {
    BASIC_AGRESSION,
    ENGLISH_TEXT,
    DEFAULT
} CharListTemplate;

void touchCharList(uint32_t **charList, CharListTemplate type, uint8_t c)
{
    if (!(*charList))
    {
        *charList = calloc(258, sizeof(uint32_t));
    }
    
    static uint32_t agression = 0;
    
    switch (type)
    {
        case BASIC_AGRESSION :
        {
            int i;
            (*charList)[c + 1] += agression;
            
            for (i = c + 2; i < 257; i++) //for all chars [ )
            {
                (*charList)[i] += agression;
            }
        }
        break;
        
        case ENGLISH_TEXT :
        {
            //(*charList) = {};
        }
        break;
        
        default :
        {
            int i;
            for (i = 1; i < 257; i++) //for all chars [ )
            {
                (*charList)[i] += (*charList)[i - 1] + 1;
            }
        }
    }
    
    int i;
    for (i = 0; i < 258; i++) //for all chars and len cell
    {
        //printf("%d - %u\n", i, charList[i]);
    }
}

void writeBit(int bit, FILE **outputFilePtr)
{
    static uint8_t buff = 0;
    static int shift = 7;
    
    if (shift < 0)
    {
        fwrite(&buff, 1, 1, *outputFilePtr);
        buff = 0;
        shift = 7;
    }
    
    buff += bit << shift--;
}

void bitsPlusFollow(ariInt bit, ariInt *bitsToFollow, FILE **outputFilePtr)
{
    writeBit(!(bit == 0), outputFilePtr);
    
    for (; (*bitsToFollow) > 0; (*bitsToFollow)--)
    {
        writeBit(!(bit == 1), outputFilePtr);
    }
}

void compressAri(char *inputFile, char *outputFile)
{
    FILE *inputFilePtr = (FILE *)fopen(inputFile, "rb");
    FILE *outputFilePtr = (FILE *)fopen(outputFile, "wb");
    
    uint32_t *charList = NULL;
    touchCharList(&charList, DEFAULT, 0);
    
    
    ariInt left = 0;
    ariInt right = 0xFFFFFFFF;
    const ariInt firstQuater = (right + 1) / 4;
    const ariInt half = firstQuater * 2;
    const ariInt thirdQuater = firstQuater * 3;
    
    ariInt bitsToFollow = 0;
    ariInt mask = 1 << 31;
    
    while(1)
    {
        ariInt div = charList[256];
        uint8_t c = fgetc(inputFilePtr);
        if( feof(inputFilePtr) ) break;
        
        ++charList[257]; //recording length of file in lenOfFile cell
        
        {
            ariInt oldLeft = left;
            left  = oldLeft + charList[c]     * (right - oldLeft + 1) / div;
            right = oldLeft + charList[c + 1] * (right - oldLeft + 1) / div - 1;
        }
        
        while(1)
        {
            if (right < half)
            {
                bitsPlusFollow(left & mask, &bitsToFollow, &outputFilePtr);
            }
            else if (left >= half)
            {
                bitsPlusFollow(left & mask, &bitsToFollow, &outputFilePtr);
                left -= half;
                right -= half;
            }
            else if ((left >= firstQuater) && (right < thirdQuater))
            {
                bitsToFollow++;
                left -= firstQuater;
                right -= firstQuater;
            }
            else
            {
                break;
            }
            left += left;
            right += right + 1;
        }
        
        touchCharList(&charList, BASIC_AGRESSION, c);
    }
    
    int i;
    for (i = 1; i < 8; i++)
    {
        mask = 1 << (32 - i);
        bitsPlusFollow(left & mask, &bitsToFollow, &outputFilePtr);
    }
    
    fwrite(&(charList[257]), sizeof(uint32_t), 1, outputFilePtr);
    
    free(charList);
    fclose(inputFilePtr);
    fclose(outputFilePtr);
}

int readBit(FILE **compressedFilePtr)
{
    static uint8_t buff = 0;
    static int remain = 0;
    
    if (!remain)
    {
        fread(&buff, 1, 1, *compressedFilePtr);
        remain = 8;
    }
    
    const uint8_t mask = 1 << 7;
    
    int bit = mask & buff;
    buff <<= 1;
    --remain;
    
    return !(bit == 0);
}

void decompressAri(char *compressedFile, char *dataFile)
{
    FILE *compressedFilePtr = (FILE *)fopen(compressedFile, "rb");
    FILE *dataFilePtr = (FILE *)fopen(dataFile, "wb");
    
    uint32_t *charList = NULL;
    touchCharList(&charList, DEFAULT, 0);
    
    ariInt left = 0;
    ariInt right = 0xFFFFFFFF;
    const ariInt firstQuater = (right + 1) / 4;
    const ariInt half = firstQuater * 2;
    const ariInt thirdQuater = firstQuater * 3;
    
    //read length of file
    fseek(compressedFilePtr, -sizeof(uint32_t), SEEK_END);
    fread(&(charList[257]), sizeof(uint32_t), 1, compressedFilePtr);
    //printf("len is %u\n", charList[257]);
    rewind(compressedFilePtr);
    
    uint32_t value;
    {
        uint32_t temp = 0;
        uint8_t bytes[4];
        int i = 3;
        for (; i > -1; i--)
        {
            fread(&(bytes[i]), 1, 1, compressedFilePtr);
        }
        memcpy(&temp, bytes, 4);
        value = temp;
    }
    
    int j = 0;
    while (charList[257]--)
    {
        ariInt div = charList[256];
        j++;
        ariInt oldLeft = left;
        ariInt oldRight = right;
        uint8_t c;
        for (c = 0; c < 256; c++) //for all possible 255 chars
        {
            left = oldLeft + charList[c] * (oldRight - oldLeft + 1) / div;
            right = oldLeft + charList[c + 1] * (oldRight - oldLeft + 1) / div - 1;
            if ((left <= value) && (value <= right)) break;
            //printf("%d -- %08X : %08X < %08X < %08X\n", j, c, left, value, right);
        }
        
        while(1)
        {
            if (right < half)
            {
            }
            else if (left >= half)
            {
                value -= half;
                left -= half;
                right -= half;
            }
            else if ((left >= firstQuater) && (right < thirdQuater))
            {
                value -= firstQuater;
                left -= firstQuater;
                right -= firstQuater;
            }
            else
            {
                break;
            }
            left += left;
            right += right + 1;
            
            value = value << 1;
            value += readBit(&compressedFilePtr);
        }
        
        fwrite(&c, 1, 1, dataFilePtr);
        touchCharList(&charList, BASIC_AGRESSION, c);
    }
    
    free(charList);
    fclose(compressedFilePtr);
    fclose(dataFilePtr);
}
