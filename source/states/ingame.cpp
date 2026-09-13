// in game state, we gamin

//exit states: postgame (win), title (lose)
#include "ingame.hpp"
#include "terminal.hpp"
#include "postgame.hpp"
#include "save.hpp"
#include "play.hpp"
#include "team_lut.hpp"
#include "season.hpp"

#include "num_text.hpp"
#include "team_graphics.hpp"

#include "ingameBG.h"
#include "football.h"
#include "yardText.h"
#include "blackWeekNum.h"
#include "ingameHeader.h"

//how many frames to hold each round's scoreboard on screen before advancing,
//so the player can watch every game of the week tick along together
static const int ROUND_PACE_FRAMES = 20;

void drawPossession(GameSituation* situation, int gameNum){
    int y = (gameNum * 4) + 1;
    int x0 = 8;
    int x1 = 18;

    int startInd = 23;
    int team0;
    int team1;
    for(int i = 0; i < 2; i++){
        team0 = situation->possession ? 22 : startInd | SE_PALBANK(11);
        team1 = situation->possession ? startInd | SE_PALBANK(11) : 22;
        se_mem[18][(y*32)+x0] = team0;
        se_mem[18][(y*32)+x1] = team1;
        situation->possession ? team1++ : team0++;
        se_mem[18][(y*32)+x0+1] = team0;
        se_mem[18][(y*32)+x1+1] = team1;
        situation->possession ? team1++ : team0++;
        startInd += 2;
        y++;
    }
}

void drawClock(GameSituation *situation, int gameNum){
    numTextSE times[2] = {
        {24, (gameNum*4), 18, 0},
        {27, (gameNum*4), 18, 0}
    };
    drawNumTextSE(&times[0], (situation->clock)/60);
    drawNumTextSE(&times[1], (situation->clock)%60, true);
}

void drawQuarter(GameSituation *situation, int gameNum){
    numTextSE quarter = {21, (gameNum*4), 18, 0};
    drawNumTextSE(&quarter, situation->quarter);
}


//starts at 149 (blank tile)
int subtextTiles[5][2] = {
    {0,0},
    {158 | SE_PALBANK(12), 154 | SE_PALBANK(12)}, //st
    {160 | SE_PALBANK(12), 156 | SE_PALBANK(12)}, //nd
    {150 | SE_PALBANK(12), 156 | SE_PALBANK(12)}, //rd
    {154 | SE_PALBANK(12), 152 | SE_PALBANK(12)}, //th
};

void drawDownSubtext(int down, int gameNum){
    int* textInd = subtextTiles[down];
    int x = 22;
    int y = (gameNum*4)+2;
    for(int i = 0 ; i < 2; i++){
        se_mem[18][(y*32)+x+i] = textInd[i];
        se_mem[18][(y*32)+x+32+i] = textInd[i]+1;
    }
}

//tileInd = 162
void drawAmpersand(int gameNum){
    int y = (gameNum*4)+2;
    int x = 25;
    se_mem[18][(y*32)+x] = 162 | SE_PALBANK(12);
    se_mem[18][(y*32)+x+32] = 163 | SE_PALBANK(12);
    se_mem[18][(y*32)+x+1] = 164 | SE_PALBANK(12);
    se_mem[18][(y*32)+x+33] = 165 | SE_PALBANK(12);
}

void drawFieldPosition(GameSituation *situation, int gameNum){
    // down + subtext
    numTextSE down = {19, (gameNum*4)+2, 18, 0};
    drawNumTextSE(&down, situation->down);
    drawDownSubtext(situation->down, gameNum);
    drawAmpersand(gameNum);

    numTextSE yardsLeft = {27, (gameNum*4)+2, 18, 0};
    drawNumTextSE(&yardsLeft, situation->distance);
    // distance
}

void drawGameScore(Game* game, int gameNum){
    numTextSE scores[2] = {
        {5, (gameNum*4)+1, 18, 0},
        {15, (gameNum*4)+1, 18, 0}
    };
    drawNumTextSE(&scores[0], game->team1.score);
    drawNumTextSE(&scores[1], game->team2.score);
}

//reduce redrawing everything every frame
void updateGameSituation(GameSituation *prevSituation, GameSituation *currSituation, int gameNum){
    if(prevSituation->distance != currSituation->distance ||
        prevSituation->down != currSituation->down){
        drawFieldPosition(currSituation, gameNum);
    }

    if(prevSituation->possession != currSituation->possession) drawPossession(currSituation, gameNum);

    if(prevSituation->clock != currSituation->clock) drawClock(currSituation, gameNum);

    if(prevSituation->quarter != currSituation->quarter) drawQuarter(currSituation, gameNum);

};

static void scroll(int* scrolly, bool byesThisWeek){
    *scrolly = clamp(*scrolly + (key_tri_vert()*2), 512-24, 512+352 - (byesThisWeek ? 32 : 0));
    REG_BG1VOFS = *scrolly;
    REG_BG2VOFS = *scrolly;
}

void loadIngameGraphics(){
    LZ77UnCompVram(footballTiles, &tile_mem[0][22]);
    LZ77UnCompVram(footballPal, pal_bg_bank[11]);

    LZ77UnCompVram(ingameHeaderTiles, &tile_mem[0][181]);
    LZ77UnCompVram(ingameHeaderMap, &se_mem[16]);

    LZ77UnCompVram(ingameBGTiles, &tile_mem[0][27]);
    LZ77UnCompVram(ingameBGPal, pal_bg_bank[12]);
    LZ77UnCompVram(ingameBGMap, se_mem[20]);
    for(int i = 0 ; i < 32*32; i++){
        se_mem[20][i] |= SE_PALBANK(12);
        se_mem[16][i] |= SE_PALBANK(12);
    }

    LZ77UnCompVram(yardTextTiles, &tile_mem[0][149]);
    LZ77UnCompVram(blackWeekNumTiles, &tile_mem[0][166]);
    LZ77UnCompVram(blackWeekNumMap, &se_mem[21]);

    memcpy16(tile_mem_obj, tp_fontTiles, tp_fontTilesLen/2);
    LZ77UnCompVram(tp_fontPal, pal_obj_mem);

    int upcomingWeek = currSeason.currentWeek + 1;

    if(upcomingWeek >= 10){
        obj_set_attr(&obj_mem[2],
            ATTR0_BUILD(72, 2, 0, 0, 0, 1, 0),
            ATTR1_BUILDR(136, 0, 0, 0),
            ATTR2_BUILD(((upcomingWeek/10) * 2)+1, 0, 0)
        );
    }
    obj_set_attr(&obj_mem[3],
        ATTR0_BUILD(72, 2, 0, 0, 0, 0, 0),
        ATTR1_BUILDR(144, 0, 0, 0),
        ATTR2_BUILD(((upcomingWeek%10) * 2)+1, 0, 0)
    );
}

int framePaceTiers[5] = {
    ROUND_PACE_FRAMES,
    15,
    10,
    5,
    1
};

GameState ingameState(){


    //fade to black
    REG_BLDCNT = BLD_TOP(BLD_BG0 | BLD_BG1 | BLD_BG2 | BLD_BG3 | BLD_OBJ) | BLD_BLACK | BLD_BOT(BLD_BACKDROP);
    for(int y = 0; y < 33; y++){
        REG_BLDY = y>>1;
        VBlankIntrWait();
    }

    SBB_CLEAR(16);
    SBB_CLEAR(17);
    SBB_CLEAR(18);
    SBB_CLEAR(19);
    REG_BG0CNT = BG_BUILD(0, 16, 0, 0, 1, 0, 0); // Header
    REG_BG1CNT = BG_BUILD(0, 18, 2, 0, 2, 0, 0); // graphics and scores and football
    REG_BG2CNT = BG_BUILD(0, 20, 0, 0, 3, 0, 0); // background
    REG_BG3CNT = BG_BUILD(0, 21, 0, 0, 0, 0, 0);

    REG_BG1HOFS = 4;
    REG_BG2HOFS = 4;

    REG_DISPCNT = DCNT_MODE0 | 
        DCNT_BG0 | 
        DCNT_BG1 | 
        DCNT_BG2 |
        DCNT_BG3 |
        DCNT_OBJ |
        DCNT_OBJ_1D;
    
    oam_init(obj_mem, 4);
    //fade in WEEK #

    initNumTextSE();
    initTeamGraphics(1);
    loadIngameGraphics();

    teamGraphicSE icons[16][2];
    int y, x;
    for(int r = 0; r < 16; r++){
        y = r*4;
        for(int c = 0; c < 2; c++){
            x = c * 10;
            icons[r][c] = {x + 1, y, 18, 3};
        }
    }

    int scrollY;

    WeekSchedule& week = currSeason.weeks[currSeason.currentWeek];

    Game games[SEASON_MAX_GAMES_PER_WEEK];
    GameSituation situations[SEASON_MAX_GAMES_PER_WEEK];
    bool active[SEASON_MAX_GAMES_PER_WEEK];

    //display Week # while building other SBB
    SBB_CLEAR(18);
    SBB_CLEAR(19);

    REG_BG0VOFS = 4;
    REG_BG1VOFS = 512-24;
    REG_BG2VOFS = 512-24;
    scrollY = 0;

    REG_BLDCNT = BLD_TOP(BLD_BG3 | BLD_OBJ) | BLD_BLACK | BLD_BOT(BLD_BACKDROP);

    for(int y = 0; y < 33; y++){
        REG_BLDY = 16-(y>>1);
        VBlankIntrWait();
    }

    //build teams while game is blacked out
    for(int g = 0; g < week.gameCount; g++){
        buildTeams(&games[g], week.games[g].team1, week.games[g].team2);
        initGameSituation(&situations[g]);
        active[g] = true;
    }

    //draw all teams
    GameSituation dummyStartSituation = {-1,-1,-1,-1,-1, -1};
    for(int g = 0; g < week.gameCount; g++){
        drawGameScore(&games[g], g);
        updateGameSituation(&dummyStartSituation, &situations[g], g);
        drawTeamGraphic(&icons[g][0], games[g].team1.teamInd);
        drawTeamGraphic(&icons[g][1], games[g].team2.teamInd);
    }

    REG_BLDCNT = BLD_TOP(BLD_BG3 | BLD_OBJ) | BLD_BLACK | BLD_BOT(BLD_BACKDROP);
    //fade in
    for(int y = 0; y < 32; y++){
        //REG_BLDALPHA = BLDA_BUILD(15-(y>>1), y>>1);
        REG_BLDY = y>>1;
        VBlankIntrWait();
    }

    REG_DISPCNT = DCNT_MODE0 | 
        DCNT_BG0 | 
        DCNT_BG1 | 
        DCNT_BG2;

    REG_BLDCNT = BLD_TOP(BLD_BG0 | BLD_BG1 | BLD_BG2) | BLD_BLACK | BLD_BOT(BLD_BACKDROP);

    for(int y = 0; y < 32; y++){
        REG_BLDY = 16-(y>>1);
        VBlankIntrWait();
    }

    for(int y = 10; y >0; y--){
        VBlankIntrWait();
    }


    bool anyActive = true;
    int framePaceTier = 0;
    while(anyActive){
        anyActive = false;
        for(int g = 0; g < week.gameCount; g++){
            if(!active[g]) continue;
            GameSituation tempSituation = situations[g];
            if(stepPlay(&games[g], &situations[g])){
                drawGameScore(&games[g], g);
                updateGameSituation(&tempSituation, &situations[g], g);
                anyActive = true;
            }else{
                updateGameSituation(&tempSituation, &situations[g], g);
                active[g] = false;
            }
        }

        int waitFrames = framePaceTiers[framePaceTier];
        for(int f = 0; f < waitFrames; f++){
            VBlankIntrWait();

            if(key_hit(KEY_LEFT) || key_hit(KEY_RIGHT)){
                framePaceTier = clamp(framePaceTier + key_tri_horz(), 0, 5);
            }
            key_poll();
            scroll(&scrollY, week.byeTeam1 != -1);
        }
    }

    //Terminal::log("Press Start to continue");
    while(!key_hit(KEY_FIRE) && !key_hit(KEY_SPECIAL)){
        key_poll();
        VBlankIntrWait();
        scroll(&scrollY, week.byeTeam1 != -1);
    }
    key_poll();

    for(int g = 0; g < week.gameCount; g++){
        recordWeekResult(&currSeason, currSeason.currentWeek, g, games[g].team1.score, games[g].team2.score);
    }
    currSeason.currentWeek++;

    return (GameState)&postgameState;
}