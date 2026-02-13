#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_jumpquest.h"
#include "player.h"
#include "game_state.h"
#include "room_sciencelab.h"
#include "room_arcade1.h"

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
// 5. Room logic
// ---------------------------------------------------------
GameState runJumpQuest(void)
{
    // Draw BG first (sets globalTileIndex to the next free slot)
    drawRoomBackground(ROOM_JUMPQUEST);
    playMusic(tune_ship);

    // -----------------------------------------------------
    // Foreground layer (arcade machine)
    // -----------------------------------------------------
    VDP_loadTileSet(arcademachine.tileset, globalTileIndex, DMA);

    VDP_drawImageEx(
        BG_A,
        &arcademachine,
        TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, globalTileIndex),
        0, 0,
        FALSE,
        TRUE
    );

    // Advance global tile index for next room assets
    //globalTileIndex += arcademachine.tileset->numTile;

    // -----------------------------------------------------
    // Sprite setup
    // -----------------------------------------------------
    SPR_reset();

    while (1)
    {
        playerHandleInput();

        // Debug + sprite update
        drawDebugInfo();
        playerUpdateSprite();

        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_QUIT;
}
