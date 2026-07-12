// set roster

//exit states: ingame, title?
#include "pregame.hpp"
#include "terminal.hpp"
#include "ingame.hpp"
#include "save.hpp"

// Pregame state implementation
// This state handles pre-game setup and returns the next state to run
GameState pregameState(){

    saveData* sd = getSaveData();
    Terminal::log("sd->currWave = %%", sd->currWave);
    Terminal::log("sd->chars[0].name = %%", sd->chars[0].name);

    while(!key_hit(KEY_ANY)){
        key_poll();
        VBlankIntrWait();
    }
    key_poll();
    
    // For now, loop back to itself
    // Later you can transition to other states like: mainMenuState, playState, etc.
    return (GameState)&ingameState;
}