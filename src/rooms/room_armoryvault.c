#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_armoryvault.h"
#include "player.h"
#include "game_state.h"
#include "room_sleepingquarters.h"     // for STATE_SLEEPING
#include "room_medicalbay.h"   // for STATE_MEDICALBAY (or whatever is next)

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
GameState runArmoryVault(void)
{
    drawRoomBackground(ROOM_ARMORYVAULT);
    playMusic(tune_ship);

    SPR_reset();
    playerSprite = SPR_addSprite(&playerSpriteDef, playerX, playerY,
                                 TILE_ATTR(PAL2, FALSE, FALSE, FALSE));

    while (1)
    {
        playerHandleInput();

        // ---- Room transition logic ----

        // Left exit → Sleeping Quarters
        if (playerX < EdgeRoomLeft + 1)
        {
            playerX = EnterRoomRight;
            playerY = 0x5A;
            return STATE_HYDROPONICSBAY;
        }

        // Right exit → Medical Bay (or next room)
        if (playerX >= EdgeRoomRight)
        {
            playerX = EnterRoomLeft;
            playerY = 0x5A + 16;
            return STATE_BIOPOD;
        }

        // Debug + sprite update
        drawDebugInfo();
        playerUpdateSprite();

        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_QUIT;
}
