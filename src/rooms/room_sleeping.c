#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_sleeping.h"
#include "player.h"          // for playerHandleInput, playerUpdateSprite
#include "game_state.h"      // for GameState
#include "room_arcade1.h"    // for STATE_ARCADE1

// ---------------------------------------------------------
// 1. Externs from other modules
// ---------------------------------------------------------
extern const u8* currentColMap;
extern u8 tileContent;

extern void drawRoomBackground(u8 room);
extern void drawDebugInfo(void);

// ---------------------------------------------------------
// 2. Room logic
// ---------------------------------------------------------
GameState runSleepingQuarters(void)
{
    drawRoomBackground(ROOM_SLEEPINGQUARTERS);
    playMusic(tune_ship);   // gameplay rooms

    // Reset sprite engine so we start clean
    SPR_reset();
    playerSprite = SPR_addSprite(&playerSpriteDef, playerX, playerY, TILE_ATTR(PAL2, FALSE, FALSE, FALSE));

    while (1)
    {
        // Player movement + collision + SFX
        playerHandleInput();

        // Switch rooms with START
        if (JOY_readJoypad(JOY_1) & BUTTON_START)
            return STATE_ARCADE1;

        // Debug + sprite update
        drawDebugInfo();
        playerUpdateSprite();

        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_QUIT;
}
