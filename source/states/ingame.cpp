// in game state, we gamin

//exit states: postgame (win), title (lose)
#include "ingame.hpp"
#include "terminal.hpp"
#include "postgame.hpp"
#include "save.hpp"
#include "play.hpp"
#include "team_lut.hpp"
#include "players.h"

static void logPlay(const PlayResult& result){
    switch(result.type){
        case PLAY_RUN:
            if(result.turnover) Terminal::log("Run, FUMBLE!");
            else Terminal::log("#%% %% run for %% yds", players_data[result.offenseCredit].jersey_number, players_data[result.offenseCredit].last_name, result.yards);
            break;
        case PLAY_PASS:
            if(result.yards < 0) Terminal::log("Sacked by #%% %% for %% yds",players_data[result.defenseCredit].jersey_number, players_data[result.defenseCredit].last_name, result.yards);
            else if(result.turnover) Terminal::log("INTERCEPTED!");
            else if(result.incomplete) Terminal::log("Incomplete pass");
            else Terminal::log("Pass to #%% %% for %% yds", players_data[result.offenseCredit].jersey_number, players_data[result.offenseCredit].last_name, result.yards);
            break;
        case PLAY_PUNT:
            Terminal::log("Punt by #%% %%, %% yds", players_data[result.offenseCredit].jersey_number, players_data[result.offenseCredit].last_name, result.yards);
            break;
        case PLAY_FIELD_GOAL:
            Terminal::log(result.scoringKick ? "Field goal GOOD" : "Field goal MISSED");
            break;
    }
}

GameState ingameState(){
    Terminal::log("%% at %%", team_lut[currGame.team2.teamInd], team_lut[currGame.team1.teamInd]);

    bool playing = true;
    while(playing){
        int quarterBefore = currSituation.quarter;

        PlayResult result;
        playing = stepPlay(&currGame, &currSituation, &result);
        logPlay(result);

        if(playing && currSituation.quarter != quarterBefore){
            Terminal::log("End of quarter %%: %% - %%", quarterBefore, currGame.team1.score, currGame.team2.score);
        }

        //pace the sim so plays are readable
        for(int f = 0; f < 20; f++){
            key_poll();
            VBlankIntrWait();
        }
    }

    Terminal::log("FINAL %% - %%", currGame.team1.score, currGame.team2.score);
    Terminal::log("Press Start to continue");
    while(!key_hit(KEY_START)){
        key_poll();
        VBlankIntrWait();
    }
    key_poll();

    return (GameState)&postgameState;
}