#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_hydroponicsbay.h"
#include "player.h"
#include "game_state.h"
#include "room_sleepingquarters.h"    // for STATE_SLEEPING

// ---------------------------------------------------------
// 1. Externs from other modules
// ---------------------------------------------------------
extern const u8* currentColMap;
extern u8 tileContent;

extern void drawRoomBackground(u8 room);
extern void drawDebugInfo(void);

// Shared global tile index from main.c
extern u16 globalTileIndex;

// ---------------------------------------------------------
// 2. Room logic
// ---------------------------------------------------------
GameState runHydroponicsBay(void)
{
    // Background is drawn first and sets globalTileIndex
    drawRoomBackground(ROOM_HYDROPONICSBAY);
    playMusic(tune_ship);

    // Load FG tileset using the shared global index
    VDP_loadTileSet(hydroponicsbay_fg.tileset, globalTileIndex, DMA);

    // Draw FG image on BG_A using the shared tile index
    VDP_drawImageEx(
        BG_A,
        &hydroponicsbay_fg,
        TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, globalTileIndex),
        0, 0,
        FALSE,
        TRUE
    );

    // Advance global tile index for next room assets
    globalTileIndex += hydroponicsbay_fg.tileset->numTile;

    // -----------------------------------------------------
    // Sprite setup
    // -----------------------------------------------------
    SPR_reset();

    playerSprite = SPR_addSprite(
        &playerSpriteDef,
        playerX,
        playerY,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );

    // -----------------------------------------------------
    // Main room loop
    // -----------------------------------------------------
    while (1)
    {
        playerHandleInput();

        // ---- Room transition logic ----

        // Left exit → Medical Bay
        if (playerX < EdgeRoomLeft + 1)
        {
            playerX = EnterRoomRight;
            playerY = 0x5A;
            return STATE_MEDICALBAY;
        }

        // Right exit → Armory Vault
        if (playerX >= EdgeRoomRight)
        {
            playerX = EnterRoomLeft;
            playerY = 0x5A;
            return STATE_ARMORYVAULT;
        }

        // Debug + sprite update
        drawDebugInfo();
        playerUpdateSprite();

        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_QUIT;
}
