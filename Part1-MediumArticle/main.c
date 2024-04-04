//Compile: gcc main.c -lm -o main.o && ./main.o
#include <stdint.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <math.h>
#define FIXEDPT_BITS 32
#include "fixedptc.h"
#include "ArithmeticCoder.h"
#include <sys/time.h>
/*Step 1: Create a random matrix*/
fixedpt **CreateRandomMatrix(int rowCount, int columnCount, float lowerBound, float upperBound)
{
 fixedpt **result = malloc(rowCount*sizeof(fixedpt*));
 for(int i = 0; i < rowCount; i++)
 {  
  result[i] = calloc(columnCount,sizeof(fixedpt));
 }
 float randomFloat = 0.0f;
 for(int i = 0; i < rowCount; i++)
 {
  for(int j = 0; j < columnCount; j++)
  {
   randomFloat = ((float)rand() / RAND_MAX) * (upperBound - lowerBound) + lowerBound;
   result[i][j] = fixedpt_rconst(randomFloat);
  }
 }
 return result;
}

/*Step 2: Create a matrix filled with zeros*/
fixedpt **CreateZeroMatrix(int rowCount, int columnCount)
{
 fixedpt **result = malloc(rowCount*sizeof(fixedpt*));
 for(int i = 0; i < rowCount; i++)
 {
  result[i] = calloc(columnCount,sizeof(fixedpt));
 }
 return result;
}

/*Step 3 : Free our matrix*/
void FreeMatrix(int rowCount, fixedpt **matrix)
{
 for(int i = 0; i < rowCount; i++)
 {
  free(matrix[i]);
 }
 free(matrix);
}

/*Step 4 : Print our matrix as a float*/
//Helper function to print fixedpoint number as floating point
void PrintFixedPoint(fixedpt A)
{
 char num[20];
 fixedpt_str(A, num, -2);
 printf("%s",num);
}

void PrintMatrix(int rowCount, int columnCount, fixedpt **matrix) 
{
 for(int i = 0; i < rowCount; i++)
 {
  for(int j = 0; j < columnCount; j++)
  {
   PrintFixedPoint(matrix[i][j]);printf(",");   
  }
 printf("\n");
 }
 printf("\n");
}
/*Step 5 : Simple Matrix multiplication*/
void NaiveFixedMatmul(int row0, int column0, int row1, int column1, fixedpt **mat0, fixedpt **mat1, fixedpt **result) 
{
 assert(column0 == row1);
 fixedpt temp = 0;
 for(int i = 0; i < row0; i++)
 {
  for(int j = 0; j < column1; j++)
  {
   result[i][j] = 0;
   for(int k = 0; k < column0; k++)
   {
    temp = fixedpt_mul(mat0[i][k], mat1[k][j]);
    result[i][j] = fixedpt_add(result[i][j],temp);
   } 
  }
 }
}

/*Step 6: Extract individual bits and count number of 1's*/
int IntToBinary(int32_t number, int oneCount, int totalCount)
{
	int bit  = 0;
	int internalOneCount = 0;
	float oneProbability = 0.5;
	for(int i = 31; i >= 0; i--)
	{
		bit = (number >> i) & 1;
		Encode(bit, oneProbability);
		internalOneCount += bit;
		totalCount += 1;
		oneProbability = (float) (oneCount+internalOneCount)/ (float) totalCount;
	}
	return internalOneCount;
}

/*Step 7: Data structure to hold our lookup table*/
/*IntWithIndex struct*/
typedef struct integer_index_struct IntWithIndex;struct integer_index_struct{int integer;int index;int *values; int valuesLength; int byteLength0;int byteLength;unsigned char *byteHolder;};
int IntWithIndexCompareIndex(const void *a, const void *b) {return ((*(IntWithIndex *)a).index - (*(IntWithIndex *)b).index);}
int IntWithIndexCompareInteger(const void *a, const void *b) {return ((*(IntWithIndex *)a).integer - (*(IntWithIndex *)b).integer);}
int IntWithIndexCompareIntegerOpposite(const void *a, const void *b) {return ((*(IntWithIndex *)b).integer - (*(IntWithIndex *)a).integer);}
int IntWithIndexCompareByteHolder(const void *a, const void *b)
{
    const IntWithIndex *struct_a = (const IntWithIndex *)a;
    const IntWithIndex *struct_b = (const IntWithIndex *)b;
 //if(struct_a->byteLength != struct_b->byteLength){return struct_a->byteLength - struct_b->byteLength;}

    for (int i = 0; i < struct_a->byteLength && i < struct_b->byteLength; ++i) {
        if (struct_a->byteHolder[i] != struct_b->byteHolder[i]) {
            return struct_a->byteHolder[i] - struct_b->byteHolder[i];
        }
    }

    // If byteHolder elements are equal, compare the lengths
    return 0;
}

int IntWithIndexCompareValues(const void *a, const void *b)
{
    const IntWithIndex *struct_a = (const IntWithIndex *)a;
    const IntWithIndex *struct_b = (const IntWithIndex *)b;
 //if(struct_a->byteLength != struct_b->byteLength){return struct_a->byteLength - struct_b->byteLength;}

    for(int i = 0; i < struct_a->valuesLength && i < struct_b->valuesLength; ++i){if(struct_a->values[i] != struct_b->values[i]){return struct_a->values[i] - struct_b->values[i];}}

    // If byteHolder elements are equal, compare the lengths
    return 0;
}
void PrintIntWithIndex(int length, IntWithIndex *array)
{
 for(int i = 0; i < length; i++)
 {
  //printf("%5d : ",array[i].integer);
  for(int j = 0; j < array[i].valuesLength; j++)
  {
   //printf("%4d,", array[i].values[j]);
  }
  printf("    16   | %d ", array[i].byteLength);
  for(int j = 0; j < array[i].byteLength; j++)
  {
   //printf("%3u,", array[i].byteHolder[j]);
  }
  printf("\n");
 }
 printf("\n");
}
IntWithIndex *CreateIntWithIndex(int length,int valuesLength)
{
 IntWithIndex *array = malloc(length * sizeof(*array));
 for(int i = 0; i < length; i++){array[i].integer = 0;array[i].index = 0;array[i].valuesLength = valuesLength;array[i].values = calloc(valuesLength,sizeof(int));array[i].byteLength0 = 0;array[i].byteLength = 0;array[i].byteHolder = calloc(byteHolderLength,sizeof(unsigned char));}
 return array;
}

void DestroyIntWithIndex(int length, IntWithIndex *array){for(int i = 0; i < length; i++){free(array[i].values);free(array[i].byteHolder);}free(array);}

/*Step 8 : Generate Lookup Table for FMA's complement*/
void GenerateTable(int row0, int column0, int row1, int column1, fixedpt **mat0, fixedpt **mat1, fixedpt **result) 
{
 IntWithIndex *tempIndex = CreateIntWithIndex(row0 * column1, column0+row1);
 assert(column0 == row1);
 assert(byteHolder != NULL);
 fixedpt temp = 0;
 int currentIndex = 0;
 int valueIndex = 0;
 int oneCount = 1;
 int totalCount = 2;
 for(int i = 0; i < row0; i++)
 {
  for(int j = 0; j < column1; j++)
  {
   result[i][j] = 0;
   valueIndex = 0;
   //printf("%4d : ", currentIndex);
   tempIndex[currentIndex].index = currentIndex;
   for(int k = 0; k < column0; k++)
   {
    //printf("%4d %4d ",mat0[i][k], mat1[k][j]);
    temp = fixedpt_mul(mat0[i][k], mat1[k][j]);
    result[i][j] = fixedpt_add(result[i][j],temp);
    tempIndex[currentIndex].values[valueIndex] = mat0[i][k];
    tempIndex[currentIndex].values[valueIndex+1] = mat1[k][j];
    valueIndex += 2;
   } 
   tempIndex[currentIndex].integer = result[i][j];
   ResetEncoder();
   oneCount = 1;
   totalCount = 2;
   for(int k = 0; k < valueIndex; k++)
   {
    //printf("%5d ", tempIndex[currentIndex].values[k]);
    oneCount += IntToBinary(tempIndex[currentIndex].values[k], oneCount, totalCount);
    totalCount += 32;
   }
   tempIndex[currentIndex].byteLength = byteHolderIndex;
   for(int k = 0; k < byteHolderIndex; k++)
   {
    tempIndex[currentIndex].byteHolder[k] = byteHolder[k];
   }
   //printf(" | %4d\n", result[i][j]);
   currentIndex += 1;
  }
 }
 qsort(tempIndex,row0 * column1, sizeof(IntWithIndex), IntWithIndexCompareByteHolder);
 PrintIntWithIndex(row0 * column1,tempIndex);
 DestroyIntWithIndex(row0 * column1,tempIndex); 
}

/*Step 9: Test our function*/

int main()
{
 

 /*Structs to hold current time*/
 struct timeval startTime;struct timeval endTime;
 double timeSpent = 0.0f;
 
 /*Initialize our random number generator with a seed*/
 srand(97656);
 /*Set lower and upper bounds for the floats we generate*/
 float lowerBound = -1.3;float upperBound = 1.3;
 /*Set number of rows and number of columns for our matrices*/
 int row0 = 3;
 int column0 = 2;
 int row1 = column0;
 int column1 = 4;
 /*allocate memory for byte holder*/
 byteHolderLength = (column0 * 2) * 50;
 byteHolder = calloc(byteHolderLength, sizeof(unsigned char));
 /*Generate our matrices*/
 fixedpt **matrix0 = CreateRandomMatrix(row0, column0,lowerBound, upperBound);
 fixedpt **matrix1 = CreateRandomMatrix(row1, column1,lowerBound, upperBound);
 fixedpt **result  = CreateZeroMatrix(row0, column1);
 printf("   Size in Bytes\nOriginal | Encoded\n");
 /*Get Start time*/
 gettimeofday(&startTime, NULL);
//NaiveFixedMatmul(row0, column0, row1, column1, matrix0, matrix1, result); 
 GenerateTable(row0, column0, row1, column1, matrix0, matrix1, result); 
 /*Get End time*/
 gettimeofday(&endTime, NULL);
 /*Print our matrices*/
//PrintMatrix(row0, column0, matrix0);
//PrintMatrix(row1, column1, matrix1);
//PrintMatrix(row0, column1, result);
 
 // Calculate the elapsed time in milliseconds
 timeSpent = (endTime.tv_sec - startTime.tv_sec) * 1000.0; // Seconds to milliseconds
 timeSpent += (endTime.tv_usec - startTime.tv_usec) / 1000.0; // Microseconds to milliseconds

 printf("Time spent %.3f\n", timeSpent);
 
 /*Free memory*/
 free(byteHolder);
}
