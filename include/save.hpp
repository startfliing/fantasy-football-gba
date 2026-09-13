#ifndef __SAVE__
#define __SAVE__

#include "tonc.h"
#include "season.hpp"

#define SAVE_MAGIC 0x46464742 // "FFGB" (Fantasy Football GBA)

struct saveData{
    u32 magic;
    int currSeed;
    bool hasSavedSeason;
    Season season;
} __attribute__((packed));

extern saveData g_saveData;
extern bool g_saveDataLoaded;

void save();

void load();

saveData* getSaveData();

#endif