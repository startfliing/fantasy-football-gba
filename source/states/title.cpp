//choose between continue and new game

//exit states: intro (timeout), pregame(continue), init(new game)
#include "title.hpp"
#include "terminal.hpp"
#include "pregame.hpp"

GameState titleState(){
    REG_DISPCNT = Terminal::initTerminal() | DCNT_MODE0;

    Terminal::log("title state");

    while(!key_hit(KEY_ANY)){
        key_poll();
        VBlankIntrWait();
    }
    key_poll();
    
    // For now, loop back to itself
    // Later you can transition to other states like: mainMenuState, playState, etc.
    return (GameState)&pregameState;
}