#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_basketball.h"
#include "game_state.h"
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
// 2. Internal Basketball states
// ---------------------------------------------------------
typedef enum
{
    BB_STATE_TITLE,
    BB_STATE_GAME
} BasketballState;

// ---------------------------------------------------------
// 3. Room logic (Basketball minigame)
// ---------------------------------------------------------
GameState runBasketball(void)
{
    drawRoomBackground(ROOM_BASKETBALL);
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

    BasketballState bbState = BB_STATE_TITLE;

    while (1)
    {
        u16 joy = JOY_readJoypad(JOY_1);

        switch (bbState)
        {
            // -------------------------------------------------
            // TITLE SCREEN
            // -------------------------------------------------
            case BB_STATE_TITLE:

                // Press B → exit back to arcade hall
                if (joy & BUTTON_B)
                {
                    return STATE_ARCADE1;
                }

                // Press A → start game
                if (joy & BUTTON_A)
                {
                    // Fade out only PAL0 (monitor)
                    PAL_fadeOut(0, 15, 8, FALSE);

                    // Draw game screen
                    VDP_drawImageEx(
                        BG_B,
                        &basketballingameexample,
                        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX),
                        0, 0,
                        FALSE,
                        TRUE
                    );

                    // Fade in using game palette
                    PAL_fadeIn(0, 15, basketballingameexample.palette->data, 8, FALSE);

                    bbState = BB_STATE_GAME;
                }
                break;

            // -------------------------------------------------
            // GAME SCREEN
            // -------------------------------------------------
            case BB_STATE_GAME:

                // Press B → return to title screen
                if (joy & BUTTON_B)
                {
                    // Fade out only PAL0
                    PAL_fadeOut(0, 15, 8, FALSE);

                    // Draw title screen
                    VDP_drawImageEx(
                        BG_B,
                        &basketballtitlescreen,
                        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX),
                        0, 0,
                        FALSE,
                        TRUE
                    );

                    // Fade in using title palette
                    PAL_fadeIn(0, 15, basketballtitlescreen.palette->data, 8, FALSE);

                    bbState = BB_STATE_TITLE;
                }

                // Press START → exit to arcade hall
                if (joy & BUTTON_START)
                {
                    return STATE_ARCADE1;
                }

                break;
        }

        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_ARCADE1;
}
