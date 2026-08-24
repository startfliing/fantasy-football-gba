#include "num_text.hpp"

void initNumTextSE(){
    LZ77UnCompVram(tp_fontPal, pal_bg_mem);
    memcpy16(&tile_mem[0][1], tp_fontTiles, tp_fontTilesLen/2);
}

void drawDigit(numTextSE* num, int digit, bool first){
    //if digit is not first and 0, then 0
    //if digit is not first and not 0, then digit
    //if digit is first and not 0, then digit
    //if digit is first and 0, then digit
    int digitInd = !first && !digit ? 0 : (digit*2)+2;
    int computedX = num->x + num->tempInd;
    int computedY = (32*num->y);

    se_mem[num->sbb][computedY+computedX] = digitInd;
    se_mem[num->sbb][computedY+32+computedX] = digitInd+1;
    num->tempInd++;
}

void drawNumTextSE(numTextSE* num, int val){
    num->tempInd = 0;
    drawDigit(num, val/100, false);
    drawDigit(num, (val/10)%10, false);
    drawDigit(num, val%10, true);
}