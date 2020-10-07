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
    const uint8_t charmax = 255;
    Table *table = calloc(charmax, sizeof(Table));
    ariInt fileLen = 0;
    
    while (1)
    {
        char c = fgetc(*inputFilePtr);
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
                if (j > 1)
                {
                    while (table[j - 1].count < table[j].count)
                    {
                        Table temp = table[j];
                        table[j] = table[j - 1];
                        table[j - 1] = temp;
                        if (!(--j)) break;
                    }
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
        printf("%d\t", b[i]);
    }
    
    return b;
}

void bitsPlusFollow(ariInt bit, int *bitsToFollow, FILE **outputFilePtr)
{
    static ariInt buff = 0;
    static int recorded = 0;
    
    
    if (bit == 2)
    {
        buff = buff << (31 - recorded);
        fwrite(&buff, sizeof(ariInt), 1, *outputFilePtr);
        return;
    }
    
    recorded++;
    buff = buff << 1;
    buff += bit;
    
    if (recorded == 32)
    {
        fwrite(&buff, sizeof(ariInt), 1, *outputFilePtr);
        buff = recorded = 0;
    }
    
    for (; *bitsToFollow > 0; (*bitsToFollow)--)
    {
        ++recorded;
        if (recorded == 32)
        {
            fwrite(&buff, sizeof(ariInt), 1, *outputFilePtr);
            buff = recorded = 0;
        }
        
        buff = buff << 1;
        buff += !bit;
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
    
    printf("%d %d %d %d %d\n", left, right, firstQuater, half, thirdQuater);
    
    ariInt bitsToFollow = 0; // Сколько бит сбрасывать
    
#if 0    
    while(1)
    {
        char c = fgetc(inputFilePtr);
        if( feof(inputFilePtr) ) break;
        
        int index;
        {
            int i = 0;
            for (i = 1; i < len; i++)
            {
                if (table[i].ch == c)
                {
                    index = i;
                    break;
                }
            }
        }
        
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
        
        printf("%X - %X\n", left, right);
    }
    
    {
        int i = 0;
        ariInt mask = 1;
        mask = mask << 31;
        for (i = 0; i < 32; i++)
        {
            bitsPlusFollow(left & mask, &bitsToFollow, &outputFilePtr);
            left = left << 1;
        }
        bitsPlusFollow(2, &bitsToFollow, &outputFilePtr);
    }
#endif
    
    fclose(inputFilePtr);
    fclose(outputFilePtr);
}

Table *reconstructTable(FILE **compressedFilePtr)
{
    ariInt lenFile;
    fread(&lenFile, sizeof(ariInt), 1, *compressedFilePtr);
    
    char len;
    fread(&len, sizeof(char), 1, *compressedFilePtr);
    
    Table *table = calloc(8192, sizeof(Table));
    
    rewind(*compressedFilePtr);
    fread(table, sizeof(Table) * len, 1, *compressedFilePtr);
    
    printf("initial len is %u\n", lenFile);
    
    return table;
}

void decompressAri(char *compressedFile, char *dataFile)
{
    FILE *compressedFilePtr = (FILE *)fopen(compressedFile, "rb");
    FILE *dataFilePtr = (FILE *)fopen(dataFile, "wb");
    
    Table *table = reconstructTable(&compressedFilePtr);
    
    ariInt value;
    fread(&value, sizeof(ariInt), 1, dataFilePtr);
    printf("%u\n", value);
    
    char len = table[0].ch;
    ariInt lenFile = table[0].count;
    int b[table[0].ch];
    {
        b[0] = 0;
        int i = 1;
        for (i = 1; i < len; i++)
        {
            b[i] = b[i - 1] + table[i].count;
        }
    }
    
    ariInt *div = NULL;
    
    ariInt left = 0;
    ariInt right = 65535; //16bit
    ariInt i = 0;
    
    ariInt firstQuater = (left + 1) / 4;  // 16384
    ariInt half = firstQuater * 2;        // 32768
    ariInt thirdQuater = firstQuater * 3; // 49152
    
    while (lenFile--)
    {
        printf("!\n");
    }
    
    fclose(compressedFilePtr);
    fclose(dataFilePtr);
}
