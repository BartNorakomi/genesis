#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_blockcannon.h"
#include "game_state.h"
#include "room_arcade1.h"
#include "player.h"

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
// 2. Internal Block Cannon states
// ---------------------------------------------------------
typedef enum
{
    BC_STATE_TITLE,
    BC_STATE_GAME
} BlockCannonState;

// ---------------------------------------------------------
// 3. Room logic (Block Cannon minigame)
// ---------------------------------------------------------
GameState runBlockCannon(void)
{
    drawRoomBackground(ROOM_BLOCKCANNON);
    playMusic(tune_ship);

    // Foreground arcade cabinet
    VDP_loadTileSet(arcademachine.tileset, globalTileIndex, DMA);
    VDP_drawImageEx(
        BG_A,
        &arcademachine,
        TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, globalTileIndex),
        0, 0,
        FALSE,
        TRUE
    );
    globalTileIndex += arcademachine.tileset->numTile;

    SPR_reset();
    SPR_update();
    waitMs(120); // ~2 seconds wait until screen has faded in completely

    BlockCannonState bcState = BC_STATE_TITLE;

    while (1)
    {
        // Centralized input system
        playerHandleInput();
        u16 joyNew = playerGetJoyNew();

        switch (bcState)
        {
            // -------------------------------------------------
            // TITLE SCREEN
            // -------------------------------------------------
            case BC_STATE_TITLE:

                // Press B → exit back to arcade hall
                if (joyNew & BUTTON_B)
                {
                    return STATE_ARCADE1;
                }

                // Press A → start game
                if (joyNew & BUTTON_A)
                {
                    // Fade out only PAL0 (monitor)
                    PAL_fadeOut(0, 15, 8, FALSE);

                    // Draw game screen
                    VDP_drawImageEx(
                        BG_B,
                        &blockcannoningameexample,
                        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX),
                        0, 0,
                        FALSE,
                        TRUE
                    );

                    // Fade in using game palette
                    PAL_fadeIn(0, 15, blockcannoningameexample.palette->data, 8, FALSE);

                    bcState = BC_STATE_GAME;
                }
                break;

            // -------------------------------------------------
            // GAME SCREEN
            // -------------------------------------------------
            case BC_STATE_GAME:

                // Press B → return to title screen
                if (joyNew & BUTTON_B)
                {
                    // Fade out only PAL0
                    PAL_fadeOut(0, 15, 8, FALSE);

                    // Draw title screen
                    VDP_drawImageEx(
                        BG_B,
                        &blockcannontitlescreen,
                        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX),
                        0, 0,
                        FALSE,
                        TRUE
                    );

                    // Fade in using title palette
                    PAL_fadeIn(0, 15, blockcannontitlescreen.palette->data, 8, FALSE);

                    bcState = BC_STATE_TITLE;
                }

                break;
        }

        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_ARCADE1;
}
