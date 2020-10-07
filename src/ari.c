#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "ari.h"

typedef unsigned short ariInt; //16 bit
//typedef uint32_t ariInt; //32 bit

typedef struct {
    uint8_t count;
    uint8_t ch;
} Table;

Table *constructTable(FILE **inputFilePtr, FILE **outputFilePtr)
{
    uint8_t c = 0;
    const uint8_t charmax = ~c;
    Table *table = calloc(charmax, sizeof(Table));
    ariInt fileLen = 0;
    
    while (1)
    {
        c = fgetc(*inputFilePtr);
        if (feof(*inputFilePtr)) break;
        
        ++fileLen;
        
        uint8_t i;
        for (i = 1; i < charmax; i++)
        {
            if (table[i].ch == 0)
            {
                table[i].ch = c;
                table[i].count = 1;
                table[0].ch = i; // len of table storage
                break;
            }
            else if (table[i].ch == c)
            {
                table[i].count += 1;
                int j = i;
                while ((table[j - 1].count < table[j].count) && (j > 1))
                {
                    Table temp = table[j];
                    table[j] = table[j - 1];
                    table[j - 1] = temp;
                    --j;
                }
                break;
            }
        }
    }
    fileLen--;
    
    // writing length of file in header
    fwrite(&fileLen, sizeof(fileLen), 1, *outputFilePtr);
    // writing character table in header
    fwrite(table, sizeof(Table) * table[0].ch, 1, *outputFilePtr);
    
    rewind(*inputFilePtr);
    return table;
}

int *buildPosList(Table *table)
{
    uint8_t lenTable = table[0].ch;
    int *b = calloc(lenTable, sizeof(int));
    
    int i;
    for (i = 1; i < lenTable; i++)
    {
        b[i] = b[i - 1] + table[i].count;
        printf("%d - %d\n", i, b[i]);
    }
    
    return b;
}

int indexForSymbol(uint8_t c, Table *table)
{
    int index;
    int i;
    for (i = 1; i < table[0].ch; i++)
    {
        if (table[i].ch == c)
        {
            index = i;
            break;
        }
    }
    return index;
}

void writeBit(int bit, FILE **outputFilePtr)
{
    static ariInt buff = 0;
    static int remaining = 0;
    
    
    
    printf("%u", bit);
}

void bitsPlusFollow(ariInt bit, ariInt *bitsToFollow, FILE **outputFilePtr)
{
    writeBit(bit, outputFilePtr);
    
    for (; (*bitsToFollow) > 0; (*bitsToFollow)--)
    {
        writeBit(!bit, outputFilePtr);
    }
}

void compressAri(char *inputFile, char *outputFile)
{
    FILE *inputFilePtr = (FILE *)fopen(inputFile, "rb");
    FILE *outputFilePtr = (FILE *)fopen(outputFile, "wb");
    
    Table *table = constructTable(&inputFilePtr, &outputFilePtr);
    int *b = buildPosList(table);
    uint8_t lenTable = table[0].ch;
    ariInt div = b[lenTable - 1];
    
    ariInt left = 0;
    ariInt right = ~left;
    ariInt firstQuater = (right + 1) / 4;
    ariInt half = firstQuater * 2;
    ariInt thirdQuater = firstQuater * 3;
    
    printf("%d %d %d %d %d\n", left, firstQuater, half, thirdQuater, right);
    
    ariInt bitsToFollow = 0; // Сколько бит сбрасывать
    
    while(1)
    {
        char c = fgetc(inputFilePtr);
        if( feof(inputFilePtr) ) break;
        
        int index = indexForSymbol(c, table);
        
        left += b[index - 1] * ((right - left + 1) / div);
        right = left + b[index] * ((right - left + 1) / div) - 1;
        
        while(1)
        {
            if (right < half)
            {
                bitsPlusFollow(0, &bitsToFollow, &outputFilePtr);
            }
            else if (left >= half)
            {
                bitsPlusFollow(1, &bitsToFollow, &outputFilePtr);
                left -= half;
                right -= half;
            }
            else if ((left >= firstQuater) && (right < thirdQuater))
            {
                bitsToFollow++;
                left -= firstQuater;
                right -= firstQuater;
            }
            else break;
            left += left;
            right += right + 1;
        }
        
        //printf("%u - %u\n", left, right);
    }
    
    free(table);
    free(b);
    fclose(inputFilePtr);
    fclose(outputFilePtr);
}

Table *reconstructTable(FILE **compressedFilePtr)
{
    uint8_t lenOfTable;
    fseek(*compressedFilePtr, sizeof(uint8_t), SEEK_CUR); //skip 0
    fread(&lenOfTable, sizeof(uint8_t), 1, *compressedFilePtr);
    fseek(*compressedFilePtr, sizeof(ariInt), SEEK_SET); //skip length
    
    Table *table = calloc(lenOfTable, sizeof(Table));
    
    fread(table, sizeof(Table) * lenOfTable, 1, *compressedFilePtr);
    
    return table;
}

void decompressAri(char *compressedFile, char *dataFile)
{
    FILE *compressedFilePtr = (FILE *)fopen(compressedFile, "rb");
    FILE *dataFilePtr = (FILE *)fopen(dataFile, "wb");
    
    ariInt lenFile;
    fread(&lenFile, sizeof(ariInt), 1, compressedFilePtr);
    
    Table *table = reconstructTable(&compressedFilePtr);
    
    int *b = buildPosList(table);
    uint8_t lenTable = table[0].ch;
    ariInt div = b[lenTable - 1];
    
    ariInt left = 0;
    ariInt right = ~left;
    ariInt firstQuater = (left + 1) / 4;
    ariInt half = firstQuater * 2;
    ariInt thirdQuater = firstQuater * 3;
    
    while (lenFile--)
    {
        printf("!\n");
    }
    
    fclose(compressedFilePtr);
    fclose(dataFilePtr);
}
