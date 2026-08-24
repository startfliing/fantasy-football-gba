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

//redraws the scoreboard with each game's current (possibly in-progress) score
static void drawWeekScores(WeekSchedule& week, Game* games, numTextSE* scores){
    Terminal::reset();
    Terminal::log("Week %% Results", currSeason.currentWeek + 1);

    for(int g = 0; g < week.gameCount; g++){
        int team1 = week.games[g].team1;
        int team2 = week.games[g].team2;
        
        //Terminal::log("%% %% - %% %%", team_lut[team1+1], games[g].team1.score, games[g].team2.score, team_lut[team2+1]);
    }

    if(week.byeTeam1 >= 0){
        Terminal::log("BYE: %% & %%", team_lut[week.byeTeam1+1], team_lut[week.byeTeam2+1]);
    }
}

static void drawGameScore(Game* game, numTextSE* scores){
    drawNumTextSE(&scores[0], game->team1.score);
    drawNumTextSE(&scores[1], game->team2.score);
}

GameState ingameState(){

    SBB_CLEAR(16);
    SBB_CLEAR(17);
    SBB_CLEAR(18);
    SBB_CLEAR(19);
    REG_BG0CNT = BG_BUILD(0, 16, 2, 0, 1, 0, 0); //BG0 text
    REG_BG1CNT = BG_BUILD(0, 18, 2, 0, 0, 0, 0); 

    REG_DISPCNT = DCNT_MODE0 | DCNT_BG0 | DCNT_BG1 | DCNT_OBJ_1D | DCNT_OBJ;

    initNumTextSE();
    initTeamGraphics();

    numTextSE scores[16][2];
    teamGraphicSE icons[16][2];
    int y, x;
    for(int r = 0; r < 16; r++){
        y = r*4;
        for(int c = 0; c < 2; c++){
            x = c * 4;
            scores[r][c] = {x+5, y+1, 16, 0};
            icons[r][c] = {x + (c*10), y, 18};
        }
    }


    int scrollY = 0;

    while(currSeason.currentWeek < SEASON_WEEKS){
        WeekSchedule& week = currSeason.weeks[currSeason.currentWeek];

        Game games[SEASON_MAX_GAMES_PER_WEEK];
        GameSituation situations[SEASON_MAX_GAMES_PER_WEEK];
        bool active[SEASON_MAX_GAMES_PER_WEEK];


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
                scrollY = clamp(scrollY + key_tri_vert(), 0, 352);
                REG_BG0VOFS = scrollY;
                REG_BG1VOFS = scrollY;
                if(!skipPacing && key_hit(KEY_START)){
                    skipPacing = true;
                    break;
                }
            }
        }

        Terminal::log("Press Start to continue");
        while(!key_hit(KEY_START)){
            key_poll();
            VBlankIntrWait();
        }
        key_poll();

        currSeason.currentWeek++;
    }

    Terminal::log("Season complete!");
    Terminal::log("Press Start to continue");
    while(!key_hit(KEY_START)){
        key_poll();
        VBlankIntrWait();
    }
    key_poll();

    return (GameState)&postgameState;
}