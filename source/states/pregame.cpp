// set roster

//exit states: ingame, title?
#include "pregame.hpp"
#include "terminal.hpp"
#include "ingame.hpp"
#include "save.hpp"
#include "playerView.hpp"

#include "team_lut.hpp"
#include "play.hpp"
#include "season.hpp"
#include "team_graphics.hpp"
#include "num_text.hpp"

#include "vs.h"
#include "header.h"
#include "seasonBG.h"

void loadPregameGraphics(){
    LZ77UnCompVram(vsTiles, &tile_mem[0][25]);
    LZ77UnCompVram(headerTiles, &tile_mem[0][31]);
}

void drawBG(){
    LZ77UnCompVram(seasonBGTiles, &tile_mem[0][124]);
    LZ77UnCompVram(seasonBGMap, se_mem[17]);
    for(int i = 0; i < 32*64; i++){
        se_mem[17][i] |= SE_PALBANK(11);
    }
    //update pal
    pal_bg_bank[11][1] = RGB15(28,28,28);
    pal_bg_bank[11][2] = RGB15(6,6,6);
}

void drawVersusText(teamGraphicSE* icon){
    int startInd = ((icon->y + 1)*32) + (icon->x + 5);
    int startTile = 26 | SE_PALBANK(11);
    se_mem[19][startInd] = startTile++;
    se_mem[19][startInd+1] = startTile++;
    se_mem[19][startInd+32] = startTile++;
    se_mem[19][startInd+33] = startTile++;
}

void drawWeeklyMatchups(int week, teamGraphicSE* icons, numTextSE* weekNum){
    for(int i = 0; i < currSeason.weeks[week].gameCount; i++){
        drawNumTextSE(weekNum, week+1);
        drawTeamGraphic(&icons[i*2], currSeason.weeks[week].games[i].team1 + 1);
        drawVersusText(&icons[i*2]);
        drawTeamGraphic(&icons[(i*2)+1], currSeason.weeks[week].games[i].team2 + 1);
    }
    for(int i = currSeason.weeks[week].gameCount; i < SEASON_MAX_GAMES_PER_WEEK; i++){
        int iconStart = (icons[i*2].y * 32) + icons[i*2].x;
        for(int j = 0; j < 4;j++){
            memset16(&se_mem[19][iconStart + (j*32)], 0, 12);
        }
    }
}

static int currWeek = 0;
static bool isPregameLoaded = false;
static int vofs = 0;

// Pregame state implementation
GameState pregameState(){

    REG_DISPCNT = DCNT_MODE0 |
        DCNT_BG0 |  //header
        DCNT_BG1 |  //seasonBG / playerBG
        DCNT_BG2;   //Team logos + vs + weeknumber

    REG_BG0CNT = BG_BUILD(0, 16, 0, 0, 0, 0, 0); 
    REG_BG1CNT = BG_BUILD(0, 17, 2, 0, 3, 0, 0); 
    REG_BG2CNT = BG_BUILD(0, 19, 2, 0, 2, 0, 0); 

    saveData* sd = &g_saveData;
    sqran(sd->currSeed);
    
    if(!isPregameLoaded){
        SBB_CLEAR(16);
        SBB_CLEAR(17); SBB_CLEAR(18);
        SBB_CLEAR(19); SBB_CLEAR(20);
        generateSeasonSchedule(&currSeason);
        currWeek = 0;
        vofs = 0;

        isPregameLoaded = true;
        initNumTextSE();
        initTeamGraphics(1);
        loadPregameGraphics();
        LZ77UnCompVram(seasonBGPal, &pal_bg_bank[11]);

        LZ77UnCompVram(headerMap, se_mem[16]);
        for(int i = 0; i < 32*32; i++){
            se_mem[16][i] |= SE_PALBANK(11);
        }
    }

    drawBG();

    REG_BG0HOFS = 0;
    REG_BG1HOFS = 0;
    REG_BG2HOFS = 0;
    REG_BG3HOFS = 0;

    REG_BG0VOFS = 4;
    REG_BG1VOFS = vofs;
    REG_BG2VOFS = vofs;
    REG_BG3VOFS = 0;

    teamGraphicSE icons[32];
    int y, x;
    for(int i = 0; i < SEASON_MAX_GAMES_PER_WEEK; i++){
        y = ((i & 7) * 4) + 10;
        x = i & 8 ? 16 : 2;
        icons[i*2] = {x, y, 19};
        icons[(i*2)+1] = {x + 8, y, 19};
    }

    numTextSE weekNum = {16,6,19};

    drawWeeklyMatchups(currWeek, icons, &weekNum);

    key_poll();

    while(!key_hit(KEY_START)){

        if(key_hit(KEY_LEFT) || key_hit(KEY_RIGHT)){
            currWeek = clamp(currWeek + key_tri_horz(), 0, SEASON_WEEKS);
            drawWeeklyMatchups(currWeek, icons, &weekNum);
        }

        if(key_hit(KEY_SELECT)){
            qran();
            generateSeasonSchedule(&currSeason);
            currWeek = 0;
            drawWeeklyMatchups(currWeek, icons, &weekNum);
            vofs = 0;
        }

        vofs = clamp(vofs + key_tri_vert()*2, 0, 192);
        REG_BG1VOFS = vofs;
        REG_BG2VOFS = vofs;

        if(key_hit(KEY_R)){
            return (GameState)&playerViewState;
        }


        key_poll();
        VBlankIntrWait();
    }

    key_poll();
    sd->currSeed = qran();
    save();

    isPregameLoaded = false;

    return (GameState)&ingameState;
}