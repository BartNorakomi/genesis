#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_medicalbay.h"
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
GameState runMedicalBay(void)
{
    drawRoomBackground(ROOM_MEDICALBAY);
    playMusic(tune_ship);   // or a new track if you want

    // Reset sprite engine so we start clean
    SPR_reset();
    playerSprite = SPR_addSprite(&playerSpriteDef, playerX, playerY,
                                 TILE_ATTR(PAL2, FALSE, FALSE, FALSE));

    while (1)
    {
        // Player movement + collision + SFX
        playerHandleInput();

        // ---- Room transition logic ----

        // Right exit → Medical Bay
        if (playerX >= EdgeRoomRight)
        {
            playerX = EnterRoomLeft;
            playerY = 0x5A;
            return STATE_HYDROPONICSBAY;
        }

        // Left exit → Training Deck (Arcade1 for now)
        if (playerX < EdgeRoomLeft + 1)
        {
            playerX = EnterRoomRight;
            playerY = 0x5A;
            return STATE_SLEEPING;
        }

        // Debug + sprite update
        drawDebugInfo();
        playerUpdateSprite();

        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_QUIT;
}
