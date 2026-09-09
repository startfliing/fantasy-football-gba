#include "team_graphics.hpp"
#include "team_lut.hpp"

#include "bears.h"
#include "bengals.h"
#include "bills.h"
#include "broncos.h"
#include "browns.h"
#include "buccs.h"
#include "cards.h"
#include "chargers.h"
#include "chiefs.h"
#include "colts.h"
#include "cowboys.h"
#include "dolphins.h"
#include "eagles.h"
#include "falcons.h"
#include "49ers.h"
#include "giants.h"
#include "jags.h"
#include "jets.h"
#include "lions.h"
#include "packers.h"
#include "panthers.h"
#include "pats.h"
#include "raiders.h"
#include "rams.h"
#include "ravens.h"
#include "commanders.h"
#include "saints.h"
#include "seahawks.h"
#include "steelers.h"
#include "titans.h"
#include "vikings.h"
#include "texans.h"

graphicPal graphicPalTeams[33] = {
    {nullptr, 0},
    {bearsTiles, 1},
    {bengalsTiles, 1},
    {billsTiles, 10},
    {broncosTiles, 1},
    {brownsTiles, 1},
    {buccsTiles, 8},
    {cardsTiles, 3},
    {chargersTiles, 4},
    {chiefsTiles, 3},
    {coltsTiles, 2},
    {cowboysTiles, 2},
    {dolphinsTiles, 7},
    {eaglesTiles, 7},
    {falconsTiles, 3},
    {_9ersTiles, 3},
    {giantsTiles, 9},
    {jagsTiles, 7},
    {jetsTiles, 6},
    {lionsTiles, 4},
    {packersTiles, 6},
    {panthersTiles, 4},
    {patsTiles, 2},
    {raidersTiles, 9},
    {ramsTiles, 4},
    {ravensTiles, 5},
    {commandersTiles, 8},
    {saintsTiles, 3},
    {seahawksTiles, 6},
    {steelersTiles, 9},
    {titansTiles, 10},
    {vikingsTiles, 5},
    {texansTiles, 2},   
};

const void* teamPalettes[11] = {
    nullptr,
    bearsPal,
    patsPal,
    cardsPal,
    ramsPal,
    ravensPal,
    packersPal,
    dolphinsPal,
    commandersPal,
    giantsPal,
    titansPal
};

void initTeamGraphics(int cbb){
    int i;
    //load palettes
    for(i = 1; i < 11; i++){
        LZ77UnCompVram(teamPalettes[i], &pal_bg_mem[i*16]);
    }

    //load image tiles
    for(i = 0; i < 32; i++){
        LZ77UnCompVram(graphicPalTeams[i+1].graphics, &tile_mem[cbb][i*16]);
    }
}

void drawTeamGraphic(teamGraphicSE* tgse, int teamInd){
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++){
            se_mem[tgse->sbb][((tgse->y + i)*32)+(tgse->x+j)] = (496 + ((teamInd*16) + j + i*4)) | SE_PALBANK(graphicPalTeams[teamInd].pal);
        }
    }
};