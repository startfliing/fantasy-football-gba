#include "playerView.hpp"
#include "terminal.hpp"
#include "players.h"
#include "pos_lut.hpp"
#include "team_lut.hpp"
#include "season.hpp"

#include "pregame.hpp"
#include "ingame.hpp"

#include "team_graphics.hpp"
#include "playerViewer.h"

void drawPlayerView(){
    LZ77UnCompVram(playerViewerTiles, &tile_mem[0][124]);
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
    VBlankIntrDelay(10);
    
    Terminal::log("%%", team_lut[currPlayerData.team_id]);
    Terminal::log("#%% %% %%", currPlayerData.jersey_number, currPlayerData.first_name, currPlayerData.last_name);
    Terminal::log("%% ", fullPositions[currPlayerData.pos_id]);
    Terminal::log(" ");
    Terminal::log(" ");
    Terminal::log("Overall Rating: %%", currPlayerData.overall_rating);
    Terminal::log(" ");
    Terminal::log("Stats:");
    switch(currPlayerData.pos_id){
        
        case 2: //POS_CB
        case 5: //POS_S
            Terminal::log("Speed: %%", currPlayerData.speed);
            Terminal::log("Tackle: %%", currPlayerData.tackle);
            Terminal::log("Catching: %%", currPlayerData.catching);
            Terminal::log("Kick Return: %%", currPlayerData.kickReturn);
            Terminal::log("Coverage: %%", currPlayerData.coverage);
            break;

        case 19: //POS_TE
        case 20: //POS_WR
        case 4: //POS_RB
            Terminal::log("Speed: %%", currPlayerData.speed);
            Terminal::log("Carrying: %%", currPlayerData.carrying);
            Terminal::log("Catching: %%", currPlayerData.catching);
            Terminal::log("Kick Return: %%", currPlayerData.kickReturn);
            Terminal::log("Juke Move: %%", currPlayerData.jukeMove);
            break;
        case 6: //POS_K
        case 13: //POS_P
            Terminal::log("Speed: %%", currPlayerData.speed);
            Terminal::log("Strength: %%", currPlayerData.strength);
            Terminal::log("Toughness: %%", currPlayerData.toughness);
            Terminal::log("Kick Accuracy: %%", currPlayerData.kickAccuracy);
            Terminal::log("Throw Accuracy: %%", currPlayerData.throwAccuracy);
            break;
        case 1: //POS_C
        case 10: //POS_LS
        case 8: //POS_LG
        case 11: //POS_LT
        case 16: //POS_RG
        case 18: //POS_RT
        case 7: //POS_LE
        case 15: //POS_RE
        case 3: //POS_DT
            Terminal::log("Speed: %%", currPlayerData.speed);
            Terminal::log("Strength: %%", currPlayerData.strength);
            Terminal::log("Toughness: %%", currPlayerData.toughness);
            Terminal::log("Block: %%", currPlayerData.block);
            Terminal::log("Tackle: %%", currPlayerData.tackle);
            break;

        
        case 9: //POS_LOLB
        case 17: //POS_ROLB
        case 12: //POS_MLB
            Terminal::log("Speed: %%", currPlayerData.speed);
            Terminal::log("Strength: %%", currPlayerData.strength);
            Terminal::log("Toughness: %%", currPlayerData.toughness);
            Terminal::log("Tackle: %%", currPlayerData.tackle);
            Terminal::log("Coverage: %%", currPlayerData.coverage);
            break;
        
        case 14: //POS_QB
            Terminal::log("Speed: %%", currPlayerData.speed);
            Terminal::log("Strength: %%", currPlayerData.strength);
            Terminal::log("Toughness: %%", currPlayerData.toughness);
            Terminal::log("Throw Accuracy: %%", currPlayerData.throwAccuracy);
            Terminal::log("Carrying: %%", currPlayerData.carrying);
            break;
        
        case 0: //POS_NA
        default:
            Terminal::log("Speed: %%", currPlayerData.speed);
            Terminal::log("Strength: %%", currPlayerData.strength);
            Terminal::log("Toughness: %%", currPlayerData.toughness);
            Terminal::log("undefined");
            Terminal::log("undefined");
            break;
    }
}

void updateTeamGraphic(){
    teamGraphicSE icons = {1, 12, 22};
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

        if(key_hit(KEY_SELECT)){
            qran();
            generateSeasonSchedule(&currSeason);
        }


        key_poll();
        VBlankIntrWait();
    }

    key_poll();

    return (GameState)&ingameState;
}