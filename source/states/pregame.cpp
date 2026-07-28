// set roster

//exit states: ingame, title?
#include "pregame.hpp"
#include "terminal.hpp"
#include "ingame.hpp"
#include "save.hpp"

#include "team_lut.hpp"
#include "play.hpp"

// Pregame state implementation
// This state handles pre-game setup and returns the next state to run
GameState pregameState(){

    saveData* sd = getSaveData();
    sqran(sd->currSeed);
    Terminal::reset();

    Terminal::log("Building teams...");

    //select teams
    int team1 = (qran() % 32);
    int team2 = (qran() % 32);
    while(team2 == team1){
        team2 = (qran() % 32);
    }

    buildTeams(team1, team2);
    initGameSituation(&currSituation);

    Terminal::reset();
    Terminal::log("%% vs %%", team_lut[team1+1], team_lut[team2+1]);
    Terminal::log("Press Start to being game!");
    while(!key_hit(KEY_START)){
        key_poll();
        VBlankIntrWait();
    }
    key_poll();
    sd->currSeed = qran();
    save();
    
    // For now, loop back to itself
    // Later you can transition to other states like: mainMenuState, playState, etc.
    return (GameState)&ingameState;
}