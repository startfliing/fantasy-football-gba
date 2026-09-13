// win state, pick new team member. randomly generated choice of 3

#include "loadSave.hpp"
#include "ingame.hpp"
#include "pregame.hpp"

#include "terminal.hpp"
#include "save.hpp"
#include "num_text.hpp"
#include "team_graphics.hpp"

#include "play.hpp"
#include "season.hpp"

#include "postGameBG.h"
#include "loadSaveHeader.h"
#include "tp_font.h"

void loadSaveGraphics(){
    LZ77UnCompVram(postGameBGTiles, &tile_mem[0][124]);
    LZ77UnCompVram(postGameBGPal, pal_bg_bank[12]);
    pal_bg_bank[12][0] = RGB15(0,0,0);
    LZ77UnCompVram(postGameBGMap, &se_mem[16]);
    for(int i = 0; i < 32 * 64; i++){
        se_mem[16][i] |= SE_PALBANK(12);
    }
    LZ77UnCompVram(loadSaveHeaderTiles, &tile_mem[0][31]);
    LZ77UnCompVram(loadSaveHeaderMap, &se_mem[20]);
    for(int i = 0; i < 32 * 32; i++){
        se_mem[20][i] |= SE_PALBANK(12);
    }

    memcpy16(tile_mem_obj, tp_fontTiles, tp_fontTilesLen/2);
    LZ77UnCompVram(tp_fontPal, pal_obj_mem);

    if(currSeason.currentWeek >= 10){
        obj_set_attr(&obj_mem[0],
            ATTR0_BUILD(4, 2, 0, 0, 0, 1, 0),
            ATTR1_BUILDR(208, 0, 0, 0),
            ATTR2_BUILD(((currSeason.currentWeek/10) * 2)+1, 0, 0)
        );
    }
    obj_set_attr(&obj_mem[1],
        ATTR0_BUILD(4, 2, 0, 0, 0, 0, 0),
        ATTR1_BUILDR(216, 0, 0, 0),
        ATTR2_BUILD(((currSeason.currentWeek%10) * 2)+1, 0, 0)
    );
}

GameState loadSaveState(){
    key_poll();
    saveData* sd = getSaveData();
    if(!sd->hasSavedSeason){
        return (GameState)&pregameState;
    } 

    REG_DISPCNT = DCNT_MODE0 | 
        DCNT_BG0 |
        DCNT_BG1 |
        DCNT_BG2 |
        DCNT_OBJ_1D |
        DCNT_OBJ;

    SBB_CLEAR(16); SBB_CLEAR(17);
    SBB_CLEAR(18); SBB_CLEAR(19);
    SBB_CLEAR(20);
    REG_BG0CNT = BG_BUILD(0, 16, 2, 0, 2, 0, 0); // postGameBG
    REG_BG1CNT = BG_BUILD(0, 18, 2, 0, 1, 0, 0); // graphics and records
    REG_BG2CNT = BG_BUILD(0, 20, 0, 0, 0, 0, 0); // postGameHeader

    REG_BG0HOFS = 0;
    REG_BG1HOFS = 252;
    REG_BG2HOFS = 0;

    REG_BG0VOFS = 512-40;
    REG_BG1VOFS = 512-40;
    REG_BG2VOFS = 4;

    oam_init(oam_mem, 128);
    currSeason = g_saveData.season;
    sqran(g_saveData.currSeed);
    initNumTextSE();
    initTeamGraphics(1);
    loadSaveGraphics();

    teamGraphicSE icons[16][2];
    int y, x;
    for(int r = 0; r < 16; r++){
        y = r*4;
        for(int c = 0; c < 2; c++){
            x = c * 15;
            icons[r][c] = {x, y, 18, 3};
        }
    }
    

    int standings[SEASON_TEAMS];
    getSortedStandings(&currSeason, standings);
    teamGraphicSE* iconPtr = &icons[0][0];
    for(int i = 0; i < SEASON_TEAMS; i++){
        drawTeamGraphic(&iconPtr[i], standings[i]+1);
        numTextSE record[3] = {
            {iconPtr[i].x+3,iconPtr[i].y+1,18,0},
            {iconPtr[i].x+7,iconPtr[i].y+1,18,0},
            {iconPtr[i].x+11,iconPtr[i].y+1,18,0}
        };
        drawNumTextSE(&record[0], currSeason.teamRecords[standings[i]].wins);
        drawNumTextSE(&record[1], currSeason.teamRecords[standings[i]].losses);
        drawNumTextSE(&record[2], currSeason.teamRecords[standings[i]].ties);
    }

    u16 vofs = 0;
    while(!key_hit(KEY_START)){

        vofs = clamp(vofs + (key_tri_vert()*2), 512-40, 512+352);
        REG_BG0VOFS = vofs;
        REG_BG1VOFS = vofs;

        if(key_hit(KEY_SELECT)){
            g_saveData.hasSavedSeason = false;
            g_saveData.currSeed = qran();
            save();
            return (GameState)&pregameState;
        }

        key_poll();
        VBlankIntrWait();
    }
    key_poll();

    return (GameState)&ingameState;
}