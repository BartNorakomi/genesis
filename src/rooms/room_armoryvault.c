#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_armoryvault.h"
#include "player.h"
#include "game_state.h"
#include "room_sleepingquarters.h"
#include "room_medicalbay.h"
#include "npc_dialogue.h"

// ---------------------------------------------------------
// 1. Externs from other modules
// ---------------------------------------------------------
extern const u8* currentColMap;
extern u8 tileContent;

extern void drawRoomBackground(u8 room);
extern void drawDebugInfo(void);

// ---------------------------------------------------------
// 2. Local sprite pointers
// ---------------------------------------------------------
static Sprite* armoryVaultPanelSprite;

// ---------------------------------------------------------
// 3. Panel position
// ---------------------------------------------------------
static int panelX = 170;
static int panelY = 97;

// ---------------------------------------------------------
// 4. Room logic
// ---------------------------------------------------------
GameState runArmoryVault(void)
{
    drawRoomBackground(ROOM_ARMORYVAULT);
    playMusic(tune_ship);

    SPR_reset();

    // Player sprite
    playerSprite = SPR_addSprite(
        &playerSpriteDef,
        playerX,
        playerY,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );

    // Armory vault panel sprite
    armoryVaultPanelSprite = SPR_addSprite(
        &armoryVaultPanelSpriteDef,
        panelX,
        panelY,
        TILE_ATTR(PAL0, FALSE, FALSE, FALSE)
    );

    while (1)
    {
        u16 joy = JOY_readJoypad(JOY_1);

        // -------------------------------------------------
        // Press A → open NPC dialogue window
        // -------------------------------------------------
        if (joy & BUTTON_A)
        {
 //           runDialogue(&portrait_ai, 2);
//            runDialogue(&portrait_soldier, 3);
        }

        // -------------------------------------------------
        // Normal gameplay
        // -------------------------------------------------
        playerHandleInput();

        // ---- Room transition logic ----

        // Left exit → Sleeping Quarters
        if (playerX < EdgeRoomLeft + 1)
        {
            playerX = EnterRoomRight;
            playerY = 0x5A;
            return STATE_HYDROPONICSBAY;
        }

        // Right exit → BioPod
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
