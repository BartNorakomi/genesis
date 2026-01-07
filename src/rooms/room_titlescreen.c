#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_titlescreen.h"
#include "sfx.h"

extern void drawRoomBackground(u8 room);
static Sprite* pressStartSprite;

GameState runTitleScreen(void)
{
    SPR_reset();
    drawRoomBackground(ROOM_TITLESCREEN);
    playMusic(tune_titlescreen);

    pressStartSprite = SPR_addSprite(&titlescreenpressbuttonSpriteDef, 90, 202, TILE_ATTR(PAL0, FALSE, FALSE, FALSE));
    SPR_setAnim(pressStartSprite, 0);

    while (1)
    {
        if (JOY_readJoypad(JOY_1) & BUTTON_START)
        {
            //XGM2_playPCM(hat1_13k, sizeof(hat1_13k), SOUND_PCM_CH2);

            XGM2_playPCMEx(sfx_hadoken, sizeof(sfx_hadoken), SOUND_PCM_CH1, 10, FALSE, FALSE); //priority, half rate(TRUE=6650hz, FALSE=13300hz), loop
//            return STATE_SLEEPING;
        }

        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_QUIT;
}
