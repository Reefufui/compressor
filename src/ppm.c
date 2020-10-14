#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "ari.h"
#include "ppm.h"

typedef struct cList {
    uint8_t ch;
    uint32_t **charArray;
} cList;

void compressPpm(char *inputFile, char *outputFile)
{
    FILE *inputFilePtr = (FILE *)fopen(inputFile, "rb");
    FILE *outputFilePtr = (FILE *)fopen(outputFile, "wb");
    
    cList *charList = calloc(255, sizeof(cList));
    int tableID = 0;
    int dummy = 0;
    {
        int i = 0;
        for (; i < 256; i++)
        {
            (charList[i]).charArray = calloc(2, sizeof(uint32_t*));
            {
                for (; tableID < 2; tableID++)
                {
                    ((charList[i]).charArray)[tableID] = calloc(258, sizeof(uint32_t));
                }
                tableID = 0;
                touchCharList(&((charList[i]).charArray), &dummy, DEFAULT, 0);
            }
        }
    }
    
    ariInt left = 0;
    ariInt right = 0xFFFFFFFF;
    const ariInt firstQuater = (right + 1) / 4;
    const ariInt half = firstQuater * 2;
    const ariInt thirdQuater = firstQuater * 3;
    
    ariInt bitsToFollow = 0;
    ariInt mask = 1 << 31;
    
    uint8_t memC = 0;
    
    while(1)
    {
        ariInt div = (charList[memC]).charArray[tableID][256];
        uint8_t c = fgetc(inputFilePtr);
        if( feof(inputFilePtr) ) break;
        
        ++((charList[0]).charArray[0][257]); //recording length of file in lenOfFile cell
        
        {
            ariInt oldLeft = left;
            left  = oldLeft + ((double)((charList[memC]).charArray[tableID][c])     * (right - oldLeft + 1)) / div;
            right = oldLeft + ((double)((charList[memC]).charArray[tableID][c + 1]) * (right - oldLeft + 1)) / div - 1;
        }
        
        while(1)
        {
            //printf("%d -- %08X : %08X < %08X < %08X\n", charList[257], c, left, half, right);
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
            else
            {
                break;
            }
            left += left;
            right += right + 1;
        }
        
        touchCharList(&((charList[memC]).charArray), &tableID, BASIC_AGRESSION, c);
        memC = c;
    }
    
    bitsPlusFollow(1, &bitsToFollow, &outputFilePtr);
    writeBit(-1, &outputFilePtr);
    //fwrite(&bitsToFollow, sizeof(ariInt), 1, outputFilePtr);
    fwrite(&((charList[0]).charArray[0][257]), sizeof(uint32_t), 1, outputFilePtr);
    
    fclose(inputFilePtr);
    fclose(outputFilePtr);
    printf("OK\n");
}

void decompressPpm(char *compressedFile, char *dataFile)
{
    FILE *compressedFilePtr = (FILE *)fopen(compressedFile, "rb");
    FILE *dataFilePtr = (FILE *)fopen(dataFile, "wb");
    
    cList *charList = calloc(255, sizeof(cList));
    int tableID = 0;
    int dummy = 0;
    {
        int i = 0;
        for (; i < 256; i++)
        {
            (charList[i]).charArray = calloc(2, sizeof(uint32_t*));
            {
                for (; tableID < 2; tableID++)
                {
                    ((charList[i]).charArray)[tableID] = calloc(258, sizeof(uint32_t));
                }
                tableID = 0;
                touchCharList(&((charList[i]).charArray), &dummy, DEFAULT, 0);
            }
        }
    }
    
    ariInt left = 0;
    ariInt right = 0xFFFFFFFF;
    const ariInt firstQuater = (right + 1) / 4;
    const ariInt half = firstQuater * 2;
    const ariInt thirdQuater = firstQuater * 3;
    
    //read length of file
    fseek(compressedFilePtr, -sizeof(uint32_t), SEEK_END);
    fread(&((charList[0]).charArray[0][257]), sizeof(uint32_t), 1, compressedFilePtr);
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
    
    uint8_t memC = 0;
    
    int j = 0;
    while (((charList[0]).charArray[0][257])--)
    {
        ariInt div = (charList[memC]).charArray[tableID][256];
        j++;
        ariInt oldLeft = left;
        ariInt oldRight = right;
        uint8_t c;
        for (c = 0; c < 256; c++) //for all possible 255 chars
        {
            left = oldLeft + ((double)((charList[memC]).charArray[tableID][c]) * (oldRight - oldLeft + 1)) / div;
            right = oldLeft + ((double)((charList[memC]).charArray[tableID][c + 1]) * (oldRight - oldLeft + 1)) / div - 1;
            if ((left <= value) && (value <= right)) break;
        }
        
        while(1)
        {
            //printf("%d -- %08X : %08X < %08X < %08X\n", j, c, left, value, right);
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
            value += value;
            right += right + 1;
            
            value += readBit(&compressedFilePtr);
            
        }
        
        touchCharList(&((charList[memC]).charArray), &tableID, BASIC_AGRESSION, c);
        memC = c;
        fwrite(&c, 1, 1, dataFilePtr);
    }
    
    fclose(compressedFilePtr);
    fclose(dataFilePtr);
    printf("OK\n");
}
