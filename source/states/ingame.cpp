// in game state, we gamin

//exit states: postgame (win), title (lose)
#include "ingame.hpp"
#include "terminal.hpp"
#include "postgame.hpp"
#include "save.hpp"


GameState ingameState(){
    Terminal::log("ingame");

    int timer = 60*10;
    u8 aCount = 0;
    while(timer > 0){
        timer--;

        if(key_hit(KEY_A)) aCount++;

        key_poll();
        VBlankIntrWait();
    }
    saveData* sd = getSaveData();
    sd->currWave = aCount;
    save();
    key_poll();
    
    // For now, loop back to itself
    // Later you can transition to other states like: mainMenuState, playState, etc.
    return (GameState)&postgameState;
}