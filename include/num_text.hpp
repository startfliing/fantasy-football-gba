#ifndef __NUM_TEXT__
#define __NUM_TEXT__

#include "tonc.h"
#include "tp_font.h"

struct numTextSE{
    int x; //tile x
    int y; //tile y
    int sbb; //sbb
    int tempInd;
};

void initNumTextSE();

void drawDigit(numTextSE* num, int digit, bool first);

void drawNumTextSE(numTextSE* num, int val, bool time = false);

#endif