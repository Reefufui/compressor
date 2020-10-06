#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "ari.h"

typedef struct {
    char ch;
    uint32_t count;
} Table;

Table *constructTable(FILE **inputFilePtr, FILE **outputFilePtr, int *len)
{
    Table *table = calloc(8192, sizeof(Table));
    
    while(1)
    {
        char c = fgetc(*inputFilePtr);
        if( feof(*inputFilePtr) ) { 
            break ;
        }
        int i;
        for (i = 0; i < 8192; i++)
        {
            if (table[i].ch == 0)
            {
                table[i].ch = c;
                table[i].count = 1;
                
                *len = (*len > i)? (*len) : i;
                
                break;
            }
            else if (table[i].ch == c)
            {
                table[i].count += 1;
                
                int j = i;
                if (j)
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
    (*len)++;
    
    fwrite(table, sizeof(Table) * (*len), 1, *outputFilePtr);
    rewind(*inputFilePtr);
    
    return table;
}

void bitsPlusFollow(int bit, int *bitsToFollow)
{
    printf("%d in bitsPlusFollow", bit);
}

void compressAri(char *inputFile, char *outputFile)
{
    FILE *inputFilePtr = (FILE *)fopen(inputFile, "rb");
    FILE *outputFilePtr = (FILE *)fopen(outputFile, "wb");
    
    int len = 0;
    Table *table = constructTable(&inputFilePtr, &outputFilePtr, &len);
    
    int b[len + 1];
    {
        b[0] = 0;
        int i = 1;
        for (i = 1; i < len + 1; i++)
        {
            b[i] = b[i - 1] + table[i - 1].count;
        }
    }
    
    uint32_t div = b[len];
    
    uint32_t left = 0;
    uint32_t right = 4294967295; //32bit
    
    uint32_t firstQuater = (left + 1) / 4;
    uint32_t half = firstQuater * 2;
    uint32_t thirdQuater = firstQuater * 3;
    
    uint32_t bitsToFollow = 0; // Сколько бит сбрасывать
    
    while(1)
    {
        char c = fgetc(inputFilePtr);
        if( feof(inputFilePtr) ) break;
        
        int index;
        {
            int i = 0;
            for (i = 0; i < len; i++)
            {
                if (table[i].ch == c)
                {
                    index = i + 1;
                    break;
                }
            }
        }
        
        left += b[index - 1] * ((right - left + 1) / div);
        right = left + b[index] * ((right - left + 1) / div - 1);
        
        while(1)
        {
            if (right < half)
            {
                bitsPlusFollow(0, &bitsToFollow);
            }
            else if (left >= half)
            {
                bitsPlusFollow(1, &bitsToFollow);
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
            right += 1;
        }
        
        printf("%u - %u\n", left, right);
    }
    
    fclose(inputFilePtr);
    fclose(outputFilePtr);
}

Table *reconstructTable(FILE **inputFilePtr, FILE **outputFilePtr)
{
    
    
    return NULL;
}

void decompressAri(char *compressedFile, char *dataFile)
{
    FILE *compressedFilePtr = (FILE *)fopen(compressedFile, "rb");
    FILE *dataFilePtr = (FILE *)fopen(dataFile, "wb");
    
    uint32_t *div = NULL;
    
    uint32_t left = 0;
    uint32_t right = 65535; //16bit
    uint32_t i = 0;
    
    uint32_t firstQuater = (left + 1) / 4;  // 16384
    uint32_t half = firstQuater * 2;        // 32768
    uint32_t thirdQuater = firstQuater * 3; // 49152
    
    uint32_t bitsToFollow = 0; // Сколько бит сбрасывать
    
    
    
    fclose(compressedFilePtr);
    fclose(dataFilePtr);
}
