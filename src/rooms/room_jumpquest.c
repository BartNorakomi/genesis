#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_jumpquest.h"
#include "game_state.h"
#include "room_arcade1.h"
#include "save_data.h"

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
// 2. Internal JumpQuest states
// ---------------------------------------------------------
typedef enum
{
    JQ_STATE_TITLE,
    JQ_STATE_GAME
} JumpQuestState;

// ---------------------------------------------------------
// 3. Room logic (JumpQuest minigame)
// ---------------------------------------------------------
GameState runJumpQuest(void)
{
    drawRoomBackground(ROOM_JUMPQUEST);
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

    JumpQuestState jqState = JQ_STATE_TITLE;

    while (1)
    {
        u16 joy = JOY_readJoypad(JOY_1);

        switch (jqState)
        {
            // -------------------------------------------------
            // TITLE SCREEN
            // -------------------------------------------------
            case JQ_STATE_TITLE:

                // NEW: Press B to exit back to arcade room
                if (joy & BUTTON_B)
                {
                    gSave.gamesPlayed++;   // <-- increment here
                    saveSaveData();
                    return STATE_ARCADE1;
                }

                if (joy & BUTTON_A)
                {
                    // Fade out only PAL0 (monitor)
                    PAL_fadeOut(0, 15, 8, FALSE);

                    // Draw game screen
                    VDP_drawImageEx(
                        BG_B,
                        &jumpquestingameexample,
                        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX),
                        0, 0,
                        FALSE,
                        TRUE
                    );

                    // Fade in using game palette
                    PAL_fadeIn(0, 15, jumpquestingameexample.palette->data, 8, FALSE);

                    jqState = JQ_STATE_GAME;
                }
                break;

            // -------------------------------------------------
            // GAME SCREEN
            // -------------------------------------------------
            case JQ_STATE_GAME:

                if (joy & BUTTON_B)
                {
                    // Fade out only PAL0 (monitor)
                    PAL_fadeOut(0, 15, 8, FALSE);

                    // Draw title screen
                    VDP_drawImageEx(
                        BG_B,
                        &jumpquesttitlescreen,
                        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX),
                        0, 0,
                        FALSE,
                        TRUE
                    );

                    // Fade in using title palette
                    PAL_fadeIn(0, 15, jumpquesttitlescreen.palette->data, 8, FALSE);

                    jqState = JQ_STATE_TITLE;
                }

                if (joy & BUTTON_START)
                {
                    gSave.gamesPlayed++;   // <-- increment here
                    saveSaveData();
                    return STATE_ARCADE1;
                }

                break;
        }

        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_ARCADE1;
}
