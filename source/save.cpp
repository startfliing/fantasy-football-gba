#include "save.hpp"
#include "tonc.h"

saveData g_saveData;
bool g_saveDataLoaded = false;

void sramCpy(u8* src, u8* dst){
    for(size_t i = 0; i < sizeof(saveData); i++){
        dst[i] = src[i];
    }
}

void save(){
    g_saveData.magic = SAVE_MAGIC;
    u8* cpySrc = (u8*)&g_saveData;
    u8* cpyDst = sram_mem;

    sramCpy(cpySrc, cpyDst);
}

void load(){
    u8* cpySrc = sram_mem;
    u8* cpyDst = (u8*)&g_saveData;

    sramCpy(cpySrc, cpyDst);
    if(g_saveData.magic != SAVE_MAGIC){
        g_saveData.magic = SAVE_MAGIC;
        g_saveData.currSeed = 0;
        g_saveData.hasSavedSeason = false;
    }
    g_saveDataLoaded = true;
}

saveData* getSaveData(){
    return &g_saveData;
}