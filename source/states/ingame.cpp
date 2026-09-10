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

//how many frames to hold each round's scoreboard on screen before advancing,
//so the player can watch every game of the week tick along together
static const int ROUND_PACE_FRAMES = 20;


static void drawGameScore(Game* game, numTextSE* scores){
    drawNumTextSE(&scores[0], game->team1.score);
    drawNumTextSE(&scores[1], game->team2.score);
}

static void scroll(int* scrolly, bool byesThisWeek){
    *scrolly = clamp(*scrolly + key_tri_vert(), 0, 352 - (byesThisWeek ? 32 : 0));
    REG_BG0VOFS = *scrolly;
    REG_BG1VOFS = *scrolly;
}

GameState ingameState(){

    SBB_CLEAR(16);
    SBB_CLEAR(17);
    SBB_CLEAR(18);
    SBB_CLEAR(19);
    REG_BG0CNT = BG_BUILD(0, 16, 2, 0, 1, 0, 0); //BG0 text
    REG_BG1CNT = BG_BUILD(0, 18, 2, 0, 0, 0, 0);
    REG_BG2CNT = BG_BUILD(0, 20, 0, 0, 3, 0, 0); 

    REG_DISPCNT = DCNT_MODE0 | DCNT_BG0 | DCNT_BG1 | DCNT_OBJ_1D | DCNT_OBJ;

    initNumTextSE();
    initTeamGraphics(1);

    numTextSE scores[16][2];
    teamGraphicSE icons[16][2];
    int y, x;
    for(int r = 0; r < 16; r++){
        y = r*4;
        for(int c = 0; c < 2; c++){
            x = c * 4;
            scores[r][c] = {x+5, y+1, 16, 0};
            icons[r][c] = {x + (c*10), y, 16, 3};
        }
    }


    int scrollY;

    while(currSeason.currentWeek < SEASON_WEEKS){
        WeekSchedule& week = currSeason.weeks[currSeason.currentWeek];

        Game games[SEASON_MAX_GAMES_PER_WEEK];
        GameSituation situations[SEASON_MAX_GAMES_PER_WEEK];
        bool active[SEASON_MAX_GAMES_PER_WEEK];

        //display Week # while building other SBB
        SBB_CLEAR(16);
        SBB_CLEAR(17);
        SBB_CLEAR(18);
        SBB_CLEAR(19);

        REG_BG0VOFS = 0;
        REG_BG1VOFS = 0;
        scrollY = 0;

        for(int g = 0; g < week.gameCount; g++){
            buildTeams(&games[g], week.games[g].team1, week.games[g].team2);
            initGameSituation(&situations[g]);
            drawGameScore(&games[g], scores[g]);
            drawTeamGraphic(&icons[g][0], games[g].team1.teamInd);
            drawTeamGraphic(&icons[g][1], games[g].team2.teamInd);
            active[g] = true;
        }

        //steps every game of the week forward one play at a time in lockstep,
        //redrawing the scoreboard after each round, until all games are final
        bool skipPacing = false;
        bool anyActive = true;
        while(anyActive){
            anyActive = false;
            for(int g = 0; g < week.gameCount; g++){
                if(!active[g]) continue;
                if(stepPlay(&games[g], &situations[g])){
                    drawGameScore(&games[g], scores[g]);
                    anyActive = true;
                }else{
                    active[g] = false;
                }
            }

            int waitFrames = skipPacing ? 1 : ROUND_PACE_FRAMES;
            for(int f = 0; f < waitFrames; f++){
                VBlankIntrWait();
                key_poll();
                scroll(&scrollY, week.byeTeam1 != -1);
                if(!skipPacing && key_hit(KEY_START)){
                    skipPacing = true;
                    break;
                }
            }
        }

        //Terminal::log("Press Start to continue");
        while(!key_hit(KEY_START)){
            key_poll();
            VBlankIntrWait();
            scroll(&scrollY, week.byeTeam1 != -1);
        }
        key_poll();

        currSeason.currentWeek++;
    }

    //Terminal::log("Season complete!");
    //Terminal::log("Press Start to continue");
    while(!key_hit(KEY_START)){
        key_poll();
        VBlankIntrWait();
    }
    key_poll();

    return (GameState)&postgameState;
}