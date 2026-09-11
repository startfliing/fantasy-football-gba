// win state, pick new team member. randomly generated choice of 3

//exit states: pregame, title??
#include "postgame.hpp"
#include "ingame.hpp"
#include "seasonEnd.hpp"

#include "terminal.hpp"
#include "save.hpp"

#include "play.hpp"
#include "season.hpp"

GameState postgameState(){

    Terminal::log("End of Week %%!", currSeason.currentWeek);

    while(!key_hit(KEY_START)){
        key_poll();
        VBlankIntrWait();
    }
    key_poll();
    
    if(currSeason.currentWeek >= SEASON_WEEKS){
        return (GameState)&seasonEndState;
    }

    return (GameState)&ingameState;
}