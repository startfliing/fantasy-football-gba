// in game state, we gamin

//exit states: postgame (win), title (lose)
#include "ingame.hpp"
#include "terminal.hpp"
#include "postgame.hpp"
#include "save.hpp"
#include "play.hpp"
#include "team_lut.hpp"
#include "season.hpp"

//runs a single matchup to completion with no play-by-play pacing/logging,
//used to resolve every game of a week "at once"
static void simulateGameFast(int team1, int team2, int* score1, int* score2){
    buildTeams(team1, team2);
    initGameSituation(&currSituation);

    bool playing = true;
    while(playing){
        playing = stepPlay(&currGame, &currSituation);
    }

    *score1 = currGame.team1.score;
    *score2 = currGame.team2.score;
}

GameState ingameState(){
    while(currSeason.currentWeek < SEASON_WEEKS){
        WeekSchedule& week = currSeason.weeks[currSeason.currentWeek];

        Terminal::reset();
        Terminal::log("Week %% Results", currSeason.currentWeek + 1);

        for(int g = 0; g < week.gameCount; g++){
            int team1 = week.games[g].team1;
            int team2 = week.games[g].team2;

            int score1, score2;
            simulateGameFast(team1, team2, &score1, &score2);

            Terminal::log("%% %% - %% %%", team_lut[team1+1], score1, score2, team_lut[team2+1]);
        }

        if(week.byeTeam1 >= 0){
            Terminal::log("BYE: %% & %%", team_lut[week.byeTeam1+1], team_lut[week.byeTeam2+1]);
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