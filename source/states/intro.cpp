// play intro images, allow for skip on key_hit(KEY_START)

//exit states: title
#include "intro.hpp"
#include "title.hpp"

#include "mng.h"
#include "gbajam1.h"
#include "gbajam2.h"

#define INTRO_SBB 16
#define INTRO_CBB 0

void loadIntro(){
    REG_BG0CNT = BG_BUILD(INTRO_CBB, INTRO_SBB, 0, 0, 1, 0, 0);
    REG_BG1CNT = BG_BUILD(INTRO_CBB+1, INTRO_SBB+1, 0, 0, 0, 0, 0);

    SBB_CLEAR(INTRO_SBB);
    SBB_CLEAR(INTRO_SBB+1);

    REG_BG2CNT = BG_BUILD(INTRO_CBB, INTRO_SBB+2, 0, 0, 2, 0, 0);
    memset16(se_mem[INTRO_SBB+2], 1, sizeof(SCREENBLOCK)/2);

    REG_BLDCNT = BLD_TOP(BLD_BG0 | BLD_BG1) | BLD_BOT(BLD_BG2) | BLD_BLACK;

    REG_DISPCNT = DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_MODE0;
}

void loadMNG(){
    //load palette
    memcpy16(pal_bg_mem, mngPal, mngPalLen/2);
    pal_bg_bank[0][0] = RGB15(0,0,0);

    //load tiles
    LZ77UnCompVram(mngTiles, tile_mem[INTRO_CBB]);
    
    //load image
    memcpy16(&se_mem[INTRO_SBB], mngMap, mngMapLen/2);
}

void loadGBAJAM1(){
    //load palette
    memcpy16(pal_bg_mem, gbajam1Pal, gbajam1PalLen/2);
    pal_bg_bank[0][0] = RGB15(0,0,0);
    //load tiles
    LZ77UnCompVram(gbajam1Tiles, tile_mem[INTRO_CBB]);
    
    //load image
    memcpy16(&se_mem[INTRO_SBB], gbajam1Map, gbajam1MapLen/2);
}

void loadGBAJAM2(){
    //load tiles
    LZ77UnCompVram(gbajam2Tiles, tile_mem[INTRO_CBB+1]);
    
    //load image
    memcpy16(&se_mem[INTRO_SBB+1], gbajam2Map, mngMapLen/2);
}

GameState introState(){
    loadIntro();

    //do this twice, start skips both
    int keyHit = key_hit(KEY_ANY);
    int phase = 0; //phase one MNG, phase 2, gbaJam

    //bottom fades in, sustains for a few seconds, fades out
    while(!keyHit && phase < 2){

        switch(phase){
            case 0:
                loadMNG();
                break;
            case 1:
                loadGBAJAM1();
                loadGBAJAM2();
                break;
            default:
                break;
        }

        int x = 0;
        int temp = 0;
        while(!keyHit && x < 128){
            x++;
            temp = x>>3;
            REG_BLDY = BLDY_BUILD(16-temp);
            key_poll();
            keyHit = key_hit(KEY_ANY);
            VBlankIntrWait();
        }

        int sustain = 0;
        while(!keyHit && x >= 128 && sustain < 120){
            sustain++;
            key_poll();
            keyHit = key_hit(KEY_ANY);
            VBlankIntrWait();
        }

        while(x > 0){
            x--;
            temp = x>>3;
            REG_BLDY = BLDY_BUILD(16-temp);
            VBlankIntrWait();
        }
        phase++;
    }

    key_poll();
    REG_BLDCNT = 0;
    // For now, loop back to itself
    // Later you can transition to other states like: mainMenuState, playState, etc.
    return (GameState)&titleState;
}