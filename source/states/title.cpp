//choose between continue and new game

//exit states: intro (timeout), pregame(continue), init(new game)
#include "title.hpp"
#include "terminal.hpp"
#include "pregame.hpp"
#include "players.h"
#include "team_lut.hpp"
#include "pos_lut.hpp"

#include "sky.h"
#include "stadium.h"
#include "pitch.h"
#include "title-text.h"

#include "save.hpp"
#include "loadSave.hpp"

void updateTitleVOFS(int vofs){
    REG_BG0VOFS = vofs;
    REG_BG1VOFS = vofs;
    REG_BG2VOFS = vofs > 256 ? vofs - 256 : 0;
    REG_BG3VOFS = vofs;
}

GameState titleState(){
    REG_DISPCNT = DCNT_MODE0 | 
        DCNT_BG0 |  //sky
        DCNT_BG1 |  //stadium
        DCNT_BG2 |  //pitch
        DCNT_BG3;   //text

    REG_BG0CNT = BG_BUILD(0, 16, 2, 0, 3, 0, 0); SBB_CLEAR(16); SBB_CLEAR(17);
    REG_BG1CNT = BG_BUILD(0, 18, 2, 0, 2, 0, 0); SBB_CLEAR(18); SBB_CLEAR(19);
    REG_BG2CNT = BG_BUILD(0, 20, 0, 0, 1, 0, 0); SBB_CLEAR(20);
    REG_BG3CNT = BG_BUILD(0, 21, 2, 0, 0, 0, 0); SBB_CLEAR(21); SBB_CLEAR(22);

    LZ77UnCompVram(skyPal, pal_bg_bank[0]);
    pal_bg_bank[0][0] = RGB15(0,0,0);
    LZ77UnCompVram(skyTiles, tile_mem[0]);
    LZ77UnCompVram(skyMap, se_mem[16]);
    REG_BG0VOFS = 0;
    REG_BG0HOFS = 0;

    LZ77UnCompVram(stadiumTiles, &tile_mem[0][340]);
    LZ77UnCompVram(stadiumMap, se_mem[19]);
    REG_BG1VOFS = 0;
    REG_BG1HOFS = 0;

    LZ77UnCompVram(pitchTiles, &tile_mem[0][563]);
    LZ77UnCompVram(pitchMap, se_mem[20]);
    REG_BG2VOFS = 0;
    REG_BG2HOFS = 0;

    LZ77UnCompVram(title_textTiles, &tile_mem[0][602]);
    LZ77UnCompVram(title_textMap, se_mem[22]);
    REG_BG3VOFS = 0;
    REG_BG3HOFS = 0;

    REG_BLDCNT = BLD_BUILD(BLD_BG3, BLD_BG0 | BLD_BG1 | BLD_BG2, 1);
    REG_BLDALPHA = BLDA_BUILD(0, 31);
    
    int vofs = 0;
    int shrunkVOFS = 0;
    while(!key_hit(KEY_START)){
        //update backgrounds
        shrunkVOFS = vofs>>1;
        vofs++;
        if(shrunkVOFS == 352) break;
        updateTitleVOFS(shrunkVOFS);
        key_poll();
        VBlankIntrWait();
    }

    key_poll();

    REG_BG0VOFS = 352;
    REG_BG1VOFS = 352;
    REG_BG2VOFS = 96;
    REG_BG3VOFS = 352;

    int blend = 0;
    while(!key_hit(KEY_START)){
        blend++;
        REG_BLDALPHA = BLDA_BUILD(blend>>1, 31-(blend>>1));
        if(blend>>1 == 31)break;
        key_poll();
        VBlankIntrWait();
    }

    REG_BLDALPHA = BLDA_BUILD(31, 0);

    // make sure that if player skips
    // Title is set to look correctly
    key_poll();

    while(!key_hit(KEY_START)){
        //wait for player to start 
        
        key_poll();
        VBlankIntrWait();
    }
    
    // For now, loop back to itself
    // Later you can transition to other states like: mainMenuState, playState, etc.
    return (GameState)&loadSaveState;
}