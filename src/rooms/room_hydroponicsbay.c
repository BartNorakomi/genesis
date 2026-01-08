#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_hydroponicsbay.h"
#include "player.h"
#include "game_state.h"
#include "room_sleeping.h"    // for STATE_SLEEPING

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
GameState runHydroponicsBay(void)
{
    drawRoomBackground(ROOM_HYDROPONICSBAY);
    playMusic(tune_ship);

    SPR_reset();
    playerSprite = SPR_addSprite(&playerSpriteDef, playerX, playerY,
                                 TILE_ATTR(PAL2, FALSE, FALSE, FALSE));

    while (1)
    {
        playerHandleInput();

        // ---- Room transition logic ----

        // Left exit → back to Sleeping Quarters
        if (playerX < EdgeRoomLeft + 1)
        {
            playerX = EnterRoomRight;
            playerY = 0x5A;
            return STATE_MEDICALBAY;
        }

        // Right exit → (future room, placeholder)
        if (playerX >= EdgeRoomRight)
        {
            playerX = EnterRoomLeft;
            playerY = 0x5A;
            // Change this when you add the next room
            return STATE_ARCADE1;
        }

        // Debug + sprite update
        drawDebugInfo();
        playerUpdateSprite();

        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_QUIT;
}
