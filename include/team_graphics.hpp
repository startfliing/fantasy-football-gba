#ifndef __TEAM_GRAPHICS__
#define __TEAM_GRAPHICS__

#include "tonc.h"

struct graphicPal {
    const void* graphics;
    int pal;
};

struct teamGraphicSE{
    int x; //tile x
    int y; //tile y
    int sbb; //sbb
    int cbb;
};

extern graphicPal graphicPalTeams[33];

void initTeamGraphics(int cbb);

void drawTeamGraphic(teamGraphicSE* tgse, int teamind);

#endif