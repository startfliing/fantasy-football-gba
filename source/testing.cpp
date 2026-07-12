#include "tonc.h"

#include "terminal.hpp"

#include "image.h"


// TREE TESTING


struct node{
    node* parent; //if nullptr, root

    int val;
};

node tree[8];

node* parentNodes[8] = {
    nullptr,
    &tree[0], &tree[0],
    &tree[1], &tree[1], &tree[3], &tree[5], &tree[6] 
};

void createTree(){
    for(int i = 0; i < 8; i++){
        tree[i] = { parentNodes[i], i};
    }
}


// FNPTR TESTING


struct person{
    int hp;
};

void attack(person* attacker, person* attacked){
    attacked->hp -= attacker->hp;
}

void heal(person* attacker, person* attacked){
    attacked->hp += attacker->hp;
}

void (*temp[])(person*, person*) = {
    &attack,
    &heal
};


// 



int randomNonsense(){
    //enable Border BG
    u8 cbb = 0;
    u8 sbb = 16;
    REG_BG0CNT = BG_BUILD(cbb, sbb, 0, 0, 1, 0, 0);

    //load palette
    memcpy16(pal_bg_mem, imagePal, imagePalLen/2);

    //load tiles
    LZ77UnCompVram(imageTiles, tile_mem[cbb]);
    
    //load image
    memcpy16(&se_mem[sbb], imageMap, imageMapLen/2);

    REG_BG2CNT = BG_BUILD(cbb, 18, 0, 0, 0, 0, 0);
    for(int i = 0; i< 20; i++){
        se_mem[18][(32*i)] = 3;
        se_mem[18][(32*i)+1] = 3;
        se_mem[18][(32*i)+2] = 3;
        se_mem[18][(32*i)+3] = 3;
    }

    //enable Text BG
    REG_BG1CNT = Terminal::setCNT(1, cbb+1, sbb+1);
    REG_DISPCNT = DCNT_BG0 | DCNT_BG1 | DCNT_MODE0 | DCNT_BG2;

    

    // Initialize Interrupts
    irq_init(nullptr);
	irq_enable(II_VBLANK);

    //Setup is done. Lets put it into action!
    //createTree();

    //for(int i = 0; i < 8; i++){
    //    node* currNode = &tree[i];
    //    node* findDepth = currNode;
    //    int depth = 0;
    //    //traverse graph to root to find depth
    //    while(findDepth->parent != nullptr){
    //        findDepth = findDepth->parent;
    //        depth++;
    //    }
    //    Terminal::log("Node %% : Depth %%", currNode->val, depth);
    //}

    //person a = {9};
    //person b = {7};
    //Terminal::log("sizeof(temp) = %%", sizeof(temp));
    //temp[0](&b, &a);
    //Terminal::log("Person A : %%", a.hp);
    //temp[1](&a, &a);
    //Terminal::log("Person A : %%", a.hp);

    while(1){

        if(key_hit(KEY_START)){
            Terminal::reset();
        }

        //update random nunmber
        qran();

        //poll what keys are down
        key_poll();

        //helps with visual tearing
        vid_vsync();
    }
}