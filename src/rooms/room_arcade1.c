#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_arcade1.h"
#include "player.h"          // for playerHandleInput, playerUpdateSprite
#include "game_state.h"      // for GameState
#include "room_sleeping.h"   // for STATE_SLEEPING
#include "player.h"          // for player globals
#include "player.h"          // (safe to include multiple times due to guards)

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
GameState runArcade1(void)
{
    drawRoomBackground(ROOM_ARCADE1);
    playMusic(tune_arcadehall);   // gameplay rooms

    // Reset sprite engine so we start clean
    SPR_reset();
    playerSprite = SPR_addSprite(&playerSpriteDef, playerX, playerY, TILE_ATTR(PAL2, FALSE, FALSE, FALSE));

    while (1)
    {
        // Player movement + collision + SFX
        playerHandleInput();

        // Switch rooms with START
        if (JOY_readJoypad(JOY_1) & BUTTON_START)
            return STATE_SLEEPING;

        // Debug + sprite update
        drawDebugInfo();
        playerUpdateSprite();

        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_QUIT;
}
