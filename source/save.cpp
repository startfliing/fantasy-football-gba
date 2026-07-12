#include "save.hpp"
#include "tonc.h"

saveData mockSaveData = {
    0,
    {
        {
            true,
            "Jacob",
            0,
            {0},
            1,
            1,
            1,
            1
        }
    }
};

static saveData g_saveData;
static bool g_saveDataLoaded = false;

void sramCpy(u8* src, u8* dst){
    for(size_t i = 0; i < sizeof(saveData); i++){
        dst[i] = src[i];
    }
}

void save(){
    u8* cpySrc = (u8*)&g_saveData;
    u8* cpyDst = sram_mem;

    sramCpy(cpySrc, cpyDst);
}

void load(){
    //TODO: remove for real game save data
    u8* cpySrc = (u8*)&mockSaveData;
    u8* cpyDst = (u8*)&g_saveData;

    sramCpy(cpySrc, cpyDst);
    g_saveDataLoaded = true;
}

saveData* getSaveData(){
    return &g_saveData;
}