#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>

uint32_t RANGE_LOW = 1; /*Minimum value in our range*/
uint32_t RANGE_HIGH = 0xffffffff; /*Maximum value in our range*/
uint32_t RANGE_CURRENT = 0; /*Current value in our range*/
unsigned char *byteHolder; /*Array to to hold encoded bytes*/
uint32_t byteHolderIndex = 0; /*Current index of byteHolderArray*/
uint32_t byteHolderLength = 0;/*Maximum length of byteHolderArray*/

/*Reset global variables*/
void ResetEncoder()
{
 RANGE_LOW = 1;
 RANGE_HIGH = 0xffffffff;
 RANGE_CURRENT = 0;
 byteHolderIndex = 0;
}
/*Encode a single bit*/
void Encode(int bit, float probability)
{
 assert(probability >= 0.0f);
 assert(probability <= 1.0f);
 assert(bit == 0 || bit == 1);
 assert(RANGE_HIGH > RANGE_LOW);
 assert(RANGE_LOW >= 0);
 int intProbability = (int) (probability * 65536.0f);
 uint32_t RANGE_MID = RANGE_LOW +((RANGE_HIGH - RANGE_LOW) >> 16) * intProbability + ((((RANGE_HIGH - RANGE_LOW) & 0xffff) * intProbability) >> 16);
 assert(RANGE_HIGH >= RANGE_MID);
 assert(RANGE_MID >= RANGE_LOW);
 if(bit){RANGE_HIGH = RANGE_MID;}else{RANGE_LOW = RANGE_MID + 1;}
 while((RANGE_HIGH ^ RANGE_LOW) < 0x1000000)
 {
  assert(byteHolderIndex < byteHolderLength);
  if(byteHolderIndex < byteHolderLength){byteHolder[byteHolderIndex]=RANGE_HIGH >> 24;}
  byteHolderIndex += 1;
         RANGE_HIGH = RANGE_HIGH <<8 | 255;
         RANGE_LOW  = RANGE_LOW  << 8;
 }
}
