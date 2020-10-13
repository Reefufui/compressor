#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "ari.h"
#include "ppm.h"

typedef struct tree {
    struct tree **lower;
    uint64_t weight;
    uint8_t character;
    int level;
} node;

void initRoot(node **root)
{
    int i = 0;
    for (; i < 256; i++)
    {
        root[i] = calloc(1, sizeof(node));
    }
}

void sumList(uint32_t **list1, uint32_t *list2)
{
    int i = 0;
    for (; i < 256; i++)
    {
        (*list1)[i] += list2[i];
    }
}

uint32_t *listFromTree(node **root, int64_t *mem, uint8_t c, int lev)
{
    uint32_t *rootList = calloc(256, sizeof(uint32_t));
    
    if (lev == 0)
    {
        sumList(&rootList, listFromTree(root, mem, c, lev + 1));
    }
    else if ((lev < 4) && (mem[3] != -1))
    {
        
        
        sumList(&rootList, listFromTree(root, mem, c, lev + 1));
        return rootList;
    }
    else
    {
        return rootList;
    }
}

void touchCharListPpm(node **root, uint32_t ***charList, int *prevTableID, CharListTemplate type, uint8_t c)
{
    if (!(*charList)) printf("Memory allocation error!\n");
    
    uint32_t agression[] = {1, 100000};
    
    static int64_t mem[4] = {-1, -1, -1, -1};
    
    uint32_t *treeList = listFromTree(root, mem, c, 0);
    
    int tableID = 0;
    for (; tableID < 2; tableID++)
    {
        switch (type)
        {
            case BASIC_AGRESSION :
            {
                int i;
                //(*charList)[tableID][c + 1] += agression[tableID];
                
                for (i = c + 2; i < 257; i++) //for all chars [ )
                {
                    //(*charList)[tableID][i] += agression[tableID];
                }
            }
            
            if ((*charList)[tableID][256] < 5000000)
            {
                break;
            }
            else
            {
                int i;
                for (i = 1; i < 257; i++)
                {
                    //(*charList)[tableID][i] /= 2;
                }
                for (i = 1; i < 257; i++)
                {
                    //(*charList)[tableID][i] += i;
                }
                break;
            }
            
            default :
            {
                int i;
                for (i = 1; i < 257; i++)
                {
                    (*charList)[tableID][i] = (*charList)[tableID][i - 1] + 1;
                }
            }
        }
    }
    
    uint64_t w1 = (*charList)[0][c + 1] - (*charList)[0][c];
    uint64_t w2 = (*charList)[1][c + 1] - (*charList)[1][c];
    uint64_t avg = (*charList)[0][256] / 255;
    
    if (1 && (w1 % 5 == 0))
    {
        printf("\t%02X (1) -> %u\t(10) -> %u\n", c, w1, w2);
        printf("avg: %u\n", avg);
    }
    
    
    const int trashHold = 20;
    static int counter = 0;
    
    int mountGrow = w1 > (avg + 1000);
    if (mountGrow) ++counter;
    if (!mountGrow) counter = 0;
    
    if (counter > trashHold) *prevTableID = 1;
    if (!counter) *prevTableID = 0;
    
    {
        int i = 3;
        for (; i > 0; i--)
        {
            mem[i] = mem[i - 1];
        }
        mem[0] = c;
    }
    
#ifdef DRAW_HIST
    //generateCSV(*charList);
#endif
}

void compressPpm(char *inputFile, char *outputFile)
{
    FILE *inputFilePtr = (FILE *)fopen(inputFile, "rb");
    FILE *outputFilePtr = (FILE *)fopen(outputFile, "wb");
    
    node *root = calloc(255, sizeof(node));
    initRoot(&root);
    
    uint32_t **charList = calloc(5, sizeof(uint32_t*));
    int tableID = 0;
    {
        for (; tableID < 5; tableID++)
        {
            charList[tableID] = calloc(258, sizeof(uint32_t));
        }
        tableID = 0;
    }
    
    ariInt left = 0;
    ariInt right = 0xFFFFFFFF;
    const ariInt firstQuater = (right + 1) / 4;
    const ariInt half = firstQuater * 2;
    const ariInt thirdQuater = firstQuater * 3;
    
    ariInt bitsToFollow = 0;
    ariInt mask = 1 << 31;
    
    touchCharListPpm(&root, &charList, &tableID, DEFAULT, 0);
    
    while(1)
    {
        ariInt div = charList[tableID][256];
        uint8_t c = fgetc(inputFilePtr);
        if( feof(inputFilePtr) ) break;
        
        ++charList[0][257]; //recording length of file in lenOfFile cell
        
        {
            ariInt oldLeft = left;
            left  = oldLeft + ((double)charList[tableID][c]     * (right - oldLeft + 1)) / div;
            right = oldLeft + ((double)charList[tableID][c + 1] * (right - oldLeft + 1)) / div - 1;
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
        
        touchCharListPpm(&root, &charList, &tableID, BASIC_AGRESSION, c);
    }
    
    
    printf("bitsToFollow = %d\n", bitsToFollow);
    bitsPlusFollow(1, &bitsToFollow, &outputFilePtr);
    writeBit(-1, &outputFilePtr);
    //fwrite(&bitsToFollow, sizeof(ariInt), 1, outputFilePtr);
    fwrite(&(charList[0][257]), sizeof(uint32_t), 1, outputFilePtr);
    
    free(charList);
    fclose(inputFilePtr);
    fclose(outputFilePtr);
}

void decompressPpm(char *compressedFile, char *dataFile)
{
    FILE *compressedFilePtr = (FILE *)fopen(compressedFile, "rb");
    FILE *dataFilePtr = (FILE *)fopen(dataFile, "wb");
    
    node *root = calloc(255, sizeof(node));
    initRoot(&root);
    
    uint32_t **charList = calloc(5, sizeof(uint32_t*));
    int tableID = 0;
    {
        for (; tableID < 5; tableID++)
        {
            charList[tableID] = calloc(258, sizeof(uint32_t));
        }
        tableID = 0;
    }
    
    ariInt left = 0;
    ariInt right = 0xFFFFFFFF;
    const ariInt firstQuater = (right + 1) / 4;
    const ariInt half = firstQuater * 2;
    const ariInt thirdQuater = firstQuater * 3;
    
    //read length of file
    fseek(compressedFilePtr, -sizeof(uint32_t), SEEK_END);
    fread(&(charList[0][257]), sizeof(uint32_t), 1, compressedFilePtr);
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
    
    touchCharListPpm(&root, &charList, &tableID, DEFAULT, 0);
    
    int j = 0;
    while (charList[0][257]--)
    {
        ariInt div = charList[tableID][256];
        j++;
        ariInt oldLeft = left;
        ariInt oldRight = right;
        uint8_t c;
        for (c = 0; c < 256; c++) //for all possible 255 chars
        {
            left = oldLeft + ((double)charList[tableID][c] * (oldRight - oldLeft + 1)) / div;
            right = oldLeft + ((double)charList[tableID][c + 1] * (oldRight - oldLeft + 1)) / div - 1;
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
        
        touchCharListPpm(&root, &charList, &tableID, BASIC_AGRESSION, c);
        fwrite(&c, 1, 1, dataFilePtr);
    }
    
    free(charList);
    fclose(compressedFilePtr);
    fclose(dataFilePtr);
}
