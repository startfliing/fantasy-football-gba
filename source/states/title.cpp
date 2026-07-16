//choose between continue and new game

//exit states: intro (timeout), pregame(continue), init(new game)
#include "title.hpp"
#include "terminal.hpp"
#include "pregame.hpp"
#include "players.h"
#include "team_lut.hpp"

void logTeam(int team, int player){
    Terminal::reset();
    Terminal::log("View Teams");
    Terminal::log(team_lut[team]);
    int tempPlayer = 0;
    for(int i = 0; i < players_count; i++){
        if(players_data[i].team_id != team){
            continue;
        }

        if(player != tempPlayer){
            tempPlayer++;
            continue;
        }else{
            Terminal::log("#%% %% %%, %%", 
                players_data[i].jersey_number, 
                players_data[i].first_name, 
                players_data[i].last_name,
                players_data[i].position_short_label
            );
            break;
        }
    }

}

GameState titleState(){
    REG_DISPCNT = Terminal::initTerminal() | DCNT_MODE0;

    

    int teamNum = 1;
    int currPlayerInd = 0;
    logTeam(teamNum, currPlayerInd);
    while(!key_hit(KEY_START)){
        if(key_hit(KEY_SHOULDER)){
            teamNum = wrap(teamNum + key_tri_shoulder(), 1, 33);
            currPlayerInd = 0;
            logTeam(teamNum, currPlayerInd);
        }

        if(key_hit(KEY_FIRE)){
            currPlayerInd = wrap(currPlayerInd + key_tri_fire(), 0, MAX_PLAYERS_PER_TEAM);
            logTeam(teamNum, currPlayerInd);
        }
        
        key_poll();
        VBlankIntrWait();
    }
    key_poll();
    
    // For now, loop back to itself
    // Later you can transition to other states like: mainMenuState, playState, etc.
    return (GameState)&pregameState;
}