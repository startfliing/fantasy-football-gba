// set roster

//exit states: ingame, title?
#include "pregame.hpp"
#include "terminal.hpp"
#include "ingame.hpp"
#include "save.hpp"

#include "team_lut.hpp"
#include "play.hpp"
#include "season.hpp"
#include "team_graphics.hpp"
#include "num_text.hpp"

#include "vs.h"
#include "header.h"
#include "seasonBG.h"

TILE vsIcon[4];

enum PREGAME_PAGE{
    SEASON,
    PLAYERS
};

void buildWeek(int week){
    for(int i = 0; i < currSeason.weeks[week].gameCount; i++){
        int yStartInd = ((i & 7) * 4) + 7;
        int xStartInd = i & 8 ? 2 : 16;
        //printTeam1(xStartInd, yStartInd);
        //printTeam2(yStartInd+8, yStartInd);
    }
}

void loadPregameGraphics(){
    LZ77UnCompVram(vsTiles, &tile_mem[0][25]);
    LZ77UnCompVram(headerTiles, &tile_mem[0][31]);
    LZ77UnCompVram(seasonBGTiles, &tile_mem[0][87]);
    LZ77UnCompVram(seasonBGPal, &pal_bg_bank[11]);
}

void drawBG(bool isSeasonView){
    if(isSeasonView){
        LZ77UnCompVram(seasonBGMap, se_mem[17]);
        for(int i = 0; i < 32*64; i++){
            se_mem[17][i] |= SE_PALBANK(11);
        }
        //update pal
        pal_bg_bank[11][1] = RGB15(28,28,28);
        pal_bg_bank[11][2] = RGB15(6,6,6);
    }else{ //player view

    }
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

// Pregame state implementation
GameState pregameState(){

    REG_DISPCNT = DCNT_MODE0 |
        DCNT_BG0 |  //header
        DCNT_BG1 |  //seasonBG / playerBG
        DCNT_BG2;   //Team logos + vs + weeknumber

    REG_BG0CNT = BG_BUILD(0, 16, 0, 0, 0, 0, 0); SBB_CLEAR(16);
    REG_BG1CNT = BG_BUILD(0, 17, 2, 0, 2, 0, 0); SBB_CLEAR(17); SBB_CLEAR(18);
    REG_BG2CNT = BG_BUILD(0, 19, 2, 0, 1, 0, 0); SBB_CLEAR(19); SBB_CLEAR(20);
    
    REG_BG0VOFS = 4;
    REG_BG1VOFS = 0;
    REG_BG2VOFS = 0;

    initNumTextSE();
    initTeamGraphics(1);
    
    loadPregameGraphics();
    drawBG(true);
    LZ77UnCompVram(headerMap, se_mem[16]);
    for(int i = 0; i < 32*32; i++){
        se_mem[16][i] |= SE_PALBANK(11);
    }

    saveData* sd = getSaveData();
    int currSeed = sqran(sd->currSeed);

    generateSeasonSchedule(&currSeason);

    teamGraphicSE icons[32];
    int y, x;
    for(int i = 0; i < SEASON_MAX_GAMES_PER_WEEK; i++){
        int yStartInd = ((i & 7) * 4) + 8;
        int xStartInd = i & 8 ? 2 : 16;
        icons[i*2] = {xStartInd, yStartInd, 19};
        icons[(i*2)+1] = {xStartInd + 8, yStartInd, 19};
    }

    PREGAME_PAGE currPage = PREGAME_PAGE::PLAYERS;
    numTextSE weekNum = {16,4,19};

    drawWeeklyMatchups(0, icons, &weekNum);


    key_poll();
    int vofs = 0;
    int currWeek = 0;
    while(!key_hit(KEY_START)){

        if(key_hit(KEY_LEFT) || key_hit(KEY_RIGHT)){
            currWeek = clamp(currWeek + key_tri_horz(), 0, SEASON_WEEKS);
            drawWeeklyMatchups(currWeek, icons, &weekNum);
        }

        vofs = clamp(vofs + key_tri_vert()*2, 0, 176);
        REG_BG1VOFS = vofs;
        REG_BG2VOFS = vofs;


        key_poll();
        VBlankIntrWait();
    }

    key_poll();
    sd->currSeed = qran();
    save();

    return (GameState)&ingameState;
}