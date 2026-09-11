#ifndef __SAVE__
#define __SAVE__

#include "tonc.h"

struct saveData{
    int currSeed;
} __attribute__((packed));

extern saveData g_saveData;
extern bool g_saveDataLoaded;

void save();

void load();

saveData* getSaveData();

#endif