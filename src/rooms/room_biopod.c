#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_biopod.h"
#include "player.h"
#include "game_state.h"
#include "room_sleepingquarters.h"       // if Biopod connects to Sleeping
#include "room_medicalbay.h"     // or whatever rooms you want

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
GameState runBioPod(void)
{
    drawRoomBackground(ROOM_BIOPOD);
    playMusic(tune_ship);

    SPR_reset();
    playerSprite = SPR_addSprite(&playerSpriteDef, playerX, playerY,
                                 TILE_ATTR(PAL2, FALSE, FALSE, FALSE));

    while (1)
    {
        playerHandleInput();

        // ---- Room transition logic ----

        // Left exit → Armory Vault
        if (playerX < EdgeRoomLeft + 1)
        {
            playerX = EnterRoomRight;
            playerY = 0x5A;
            return STATE_ARMORYVAULT;
        }

        // Right exit → Reactor Chamber (or next room)
        if (playerX >= EdgeRoomRight)
        {
            playerX = EnterRoomLeft;
            playerY = 0x5A;
            return STATE_REACTORCHAMBER;
        }

        // Debug + sprite update
        drawDebugInfo();
        playerUpdateSprite();

        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_QUIT;
}
