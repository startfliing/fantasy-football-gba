#include "playerView.hpp"
#include "terminal.hpp"
#include "players.h"
#include "pos_lut.hpp"
#include "team_lut.hpp"

#include "pregame.hpp"
#include "ingame.hpp"

#include "team_graphics.hpp"
#include "playerViewer.h"

void drawPlayerView(){
    LZ77UnCompVram(playerViewerTiles, &tile_mem[0][87]);
    LZ77UnCompVram(playerViewerMap, se_mem[17]);
    for(int i = 0; i < 32*32; i++){
        se_mem[17][i] |= SE_PALBANK(11);
    }

    //drawPlayer
    pal_bg_bank[11][1] = RGB15(6,6,6);
    pal_bg_bank[11][2] = RGB15(28,28,28);
}

struct playerViewer{
    int playerNum;
    int team;
};

static playerViewer currPlayer = {0,0};

void drawPlayer(){
    int playerInd = (74*(currPlayer.team))+currPlayer.playerNum;
    if(currPlayer.team >= 5) playerInd--;
    Players currPlayerData = players_data[playerInd];
    Terminal::reset();
    Terminal::log("%%", team_lut[currPlayerData.team_id]);
    Terminal::log("#%% %% %%", currPlayerData.jersey_number, currPlayerData.first_name, currPlayerData.last_name);
    Terminal::log(" ");
    Terminal::log("%% ", fullPositions[currPlayerData.pos_id]);
}

void updateTeamGraphic(){
    teamGraphicSE icons = {23, 6, 22};
    drawTeamGraphic(&icons, currPlayer.team+1);
}

static bool playerViewLoaded = false;

// Pregame state implementation
GameState playerViewState(){

    REG_DISPCNT = DCNT_MODE0 |
        DCNT_BG0 |  //header
        DCNT_BG1 |  //seasonBG / playerBG
        DCNT_BG2 |  //Team logos + vs + weeknumber
        DCNT_BG3;   //Terminal for player text

    REG_BG2CNT = BG_BUILD(0, 22, 0, 0, 0, 0, 0); 
    
    drawPlayerView();
    REG_BG0VOFS = 4;
    REG_BG1VOFS = 0;
    REG_BG2VOFS = 0;
    REG_BG3VOFS = 0;
    
    
    if(!playerViewLoaded){
        Terminal::initTerminal();
        SBB_CLEAR(22);
        drawPlayer();
        updateTeamGraphic();
        playerViewLoaded = true;
    }

    key_poll();


    while(!key_hit(KEY_START)){

        //update player
        if(key_hit(KEY_LEFT) || key_hit(KEY_RIGHT)){
            currPlayer.playerNum = wrap(currPlayer.playerNum + key_tri_horz(), 0, currPlayer.team == 4 ? 73 : 74);
            drawPlayer();
        }

        //update team
        if(key_hit(KEY_UP) || key_hit(KEY_DOWN)){
            currPlayer.team = wrap(currPlayer.team + key_tri_vert(), 0, 32);
            currPlayer.playerNum = 0;
            updateTeamGraphic();
            drawPlayer();
        }


        if(key_hit(KEY_L)){
            return (GameState)&pregameState;
        }


        key_poll();
        VBlankIntrWait();
    }

    key_poll();

    return (GameState)&ingameState;
}