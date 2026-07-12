#ifndef __SAVE__
#define __SAVE__

#include "tonc.h"

struct CharacterData {
    bool isActive;
    char name[10];
    u16 spriteInd; //from sprite sheet
    PALBANK palette; //color palette, adds uniqueness, could potentially use premade indices

    // in game stats
    u16 speed; //how fast they finish an item
    u16 quality; //how much damage it does
    u16 accuracy; //chance to miss
    u16 temper; //how angry the character gets
};

struct saveData{
    u16 currWave;
    CharacterData chars[9]; //order matters for positioning
} __attribute__((packed));

void save();

void load();

saveData* getSaveData();

#endif