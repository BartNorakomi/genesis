#include <genesis.h>
#include "resources.h"
#include "npc_dialogue.h"
#include "game_state.h"

// ---------------------------------------------------------
// 2. Dialogue window tile base
// ---------------------------------------------------------
#define TILE_DIALOGUE_BASE  500

// ---------------------------------------------------------
// 3. Draw the dialogue window
// ---------------------------------------------------------
void npcDialogueOpenWindow(void)
{
    // Load palette into PAL1
    PAL_setPalette(PAL1, NPCBlackRectangle.palette->data, CPU);

    // Load tiles into VRAM
    VDP_loadTileData(
        NPCBlackRectangle.tileset->tiles,
        TILE_DIALOGUE_BASE,
        NPCBlackRectangle.tileset->numTile,
        DMA
    );

    // Draw window at tile position (0, 1)
    VDP_drawImageEx(
        BG_A,
        &NPCBlackRectangle,
        TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, TILE_DIALOGUE_BASE),
        0, 1,
        FALSE,
        TRUE
    );
}

// ---------------------------------------------------------
// 4. Clear the dialogue window
// ---------------------------------------------------------
void npcDialogueCloseWindow(void)
{
    // Clear the tilemap area where the window was drawn
    VDP_clearTileMapRect(BG_A, 0, 1, 40, 28);
}

// ---------------------------------------------------------
// 5. Dialogue state logic
// ---------------------------------------------------------
GameState runDialogue(void)
{
    npcDialogueOpenWindow();

    while (1)
    {
        u16 joy = JOY_readJoypad(JOY_1);

        // Press B to close dialogue and return to previous room
        if (joy & BUTTON_B)
        {
            npcDialogueCloseWindow();
            return;
        }

        SYS_doVBlankProcess();
    }

}
