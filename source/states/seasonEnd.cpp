#include "seasonEnd.hpp"

#include "title.hpp"
#include "pregame.hpp"

#include "save.hpp"
#include "season.hpp"

#include "team_graphics.hpp"

#include "seasonEndBG.h"

void loadSeasonEndGraphics(){
    LZ77UnCompVram(seasonEndBGTiles, &tile_mem[0][124]);
    LZ77UnCompVram(seasonEndBGPal, &pal_bg_bank[12]);
    LZ77UnCompVram(seasonEndBGMap, &se_mem[20]);
    for(int i = 0; i < 32*32; i++){
        se_mem[20][i] |= SE_PALBANK(12);
    }
}

GameState seasonEndState(){

    REG_DISPCNT = DCNT_MODE0 |
        DCNT_BG0 |  // BG
        DCNT_BG1;   // graphics

    SBB_CLEAR(20);
    SBB_CLEAR(22);

    REG_BG0CNT = BG_BUILD(0, 20, 0, 0, 1, 0, 0); 
    REG_BG1CNT = BG_BUILD(0, 22, 0, 0, 0, 0, 0); 
    
    initTeamGraphics(1);
    loadSeasonEndGraphics();
    REG_BG0VOFS = 0;
    REG_BG1VOFS = 0;

    REG_BG0HOFS = 0;
    REG_BG1HOFS = 0;
    

    SBB_CLEAR(22);

    int standings[SEASON_TEAMS];
    getSortedStandings(&currSeason, standings);

    teamGraphicSE places[3] = {
        {13, 4, 22, 0},
        {3, 8, 22, 0},
        {23, 8, 22, 0},
    };

    for(int i = 0; i < 3; i++){
        drawTeamGraphic(&places[i], standings[i]+1);
    }

    while(!key_hit(KEY_START)){
        key_poll();
        VBlankIntrWait();
        if(key_hit(KEY_SELECT)){
            currSeason = {};
            g_saveData.season = currSeason;
            g_saveData.hasSavedSeason = false;
            g_saveData.currSeed = qran();
            save();
            return (GameState)&titleState;
        }
        
    }
    key_poll();

    currSeason = {};
    g_saveData.season = currSeason;
    g_saveData.hasSavedSeason = false;
    g_saveData.currSeed = qran();
    save();

    return (GameState)&titleState;
}