// in game state, we gamin

//exit states: postgame (win), title (lose)
#include "ingame.hpp"
#include "terminal.hpp"
#include "postgame.hpp"
#include "save.hpp"
#include "play.hpp"
#include "team_lut.hpp"
#include "season.hpp"

//how many frames to hold each round's scoreboard on screen before advancing,
//so the player can watch every game of the week tick along together
static const int ROUND_PACE_FRAMES = 20;

//redraws the scoreboard with each game's current (possibly in-progress) score
static void drawWeekScores(WeekSchedule& week, Game* games){
    Terminal::reset();
    Terminal::log("Week %% Results", currSeason.currentWeek + 1);

    for(int g = 0; g < week.gameCount; g++){
        int team1 = week.games[g].team1;
        int team2 = week.games[g].team2;
        Terminal::log("%% %% - %% %%", team_lut[team1+1], games[g].team1.score, games[g].team2.score, team_lut[team2+1]);
    }

    if(week.byeTeam1 >= 0){
        Terminal::log("BYE: %% & %%", team_lut[week.byeTeam1+1], team_lut[week.byeTeam2+1]);
    }
}

GameState ingameState(){
    while(currSeason.currentWeek < SEASON_WEEKS){
        WeekSchedule& week = currSeason.weeks[currSeason.currentWeek];

        Game games[SEASON_MAX_GAMES_PER_WEEK];
        GameSituation situations[SEASON_MAX_GAMES_PER_WEEK];
        bool active[SEASON_MAX_GAMES_PER_WEEK];

        for(int g = 0; g < week.gameCount; g++){
            buildTeams(&games[g], week.games[g].team1, week.games[g].team2);
            initGameSituation(&situations[g]);
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
                    anyActive = true;
                }else{
                    active[g] = false;
                }
            }

            drawWeekScores(week, games);

            int waitFrames = skipPacing ? 1 : ROUND_PACE_FRAMES;
            for(int f = 0; f < waitFrames; f++){
                VBlankIntrWait();
                key_poll();
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