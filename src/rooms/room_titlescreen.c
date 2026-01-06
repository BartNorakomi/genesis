#include <genesis.h>
#include "resources.h"
#include "rooms.h"
#include "room_titlescreen.h"

extern void drawRoomBackground(u8 room);
static Sprite* pressStartSprite;

GameState runTitleScreen(void)
{
    drawRoomBackground(ROOM_TITLESCREEN);
    playMusic(tune_titlescreen);   // title screen
    
    SPR_reset();
    pressStartSprite = SPR_addSprite(&titlescreenpressbuttonSpriteDef, 90, 202, TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
    SPR_setAnim(pressStartSprite, 0);

    while (1)
    {
        // Press START to continue
        if (JOY_readJoypad(JOY_1) & BUTTON_START)
            return STATE_SLEEPING;
        SPR_update();

        SYS_doVBlankProcess();
    }

    return STATE_QUIT;
}
