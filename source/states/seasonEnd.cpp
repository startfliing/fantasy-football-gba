#include "seasonEnd.hpp"

#include "title.hpp"
#include "terminal.hpp"

#include "season.hpp"

GameState seasonEndState(){

    Terminal::log("season End!");

    while(!key_hit(KEY_START)){
        key_poll();
        VBlankIntrWait();
    }
    key_poll();

    return (GameState)&titleState;
}