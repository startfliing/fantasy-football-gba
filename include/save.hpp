#ifndef __SAVE__
#define __SAVE__

#include "tonc.h"


struct saveData{
    int currSeed;
} __attribute__((packed));

void save();

void load();

saveData* getSaveData();

#endif