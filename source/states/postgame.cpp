// win state, pick new team member. randomly generated choice of 3

//exit states: pregame, title??
#include "postgame.hpp"
#include "terminal.hpp"
#include "pregame.hpp"
#include "save.hpp"

GameState postgameState(){

    saveData* sd = getSaveData();
    Terminal::log("You pressed A %% times!", sd->currWave);

    while(!key_hit(KEY_B)){
        key_poll();
        VBlankIntrWait();
    }
    key_poll();
    
    // For now, loop back to itself
    // Later you can transition to other states like: mainMenuState, playState, etc.
    return (GameState)&pregameState;
}