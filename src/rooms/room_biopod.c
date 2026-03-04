#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_biopod.h"
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
static Sprite* biopodRightPodSprite;
static Sprite* biopodLeftPodSprite;
static Sprite* biopodLightSprite;

// ---------------------------------------------------------
// 3. Feet positions for depth sorting
// ---------------------------------------------------------
static int biopodRightPodX = 156;
static int biopodRightPodY = 86;

static int biopodLeftPodX  = 68;
static int biopodLeftPodY  = 114;

static int biopodLightX    = 126;
static int biopodLightY    = 113;

// ---------------------------------------------------------
// 4. Unified depth sorting
// ---------------------------------------------------------
static void updateDepth(void)
{
    SPR_setDepth(playerSprite, -playerY);
    SPR_setDepth(biopodRightPodSprite, -biopodRightPodY + 30);
    SPR_setDepth(biopodLeftPodSprite,  -biopodLeftPodY + 60);
}

// ---------------------------------------------------------
// 5. Room logic
// ---------------------------------------------------------
GameState runBioPod(void)
{
    drawRoomBackground(ROOM_BIOPOD);
    playMusic(tune_ship);

    SPR_reset();

    // Player sprite
    playerSprite = SPR_addSprite(
        &playerSpriteDef,
        playerX, playerY,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );

    // Right pod
    biopodRightPodSprite = SPR_addSprite(
        &biopodRightPodSpriteDef,
        biopodRightPodX,
        biopodRightPodY,
        TILE_ATTR(PAL3, FALSE, FALSE, FALSE)
    );

    // Left pod
    biopodLeftPodSprite = SPR_addSprite(
        &biopodLeftPodSpriteDef,
        biopodLeftPodX,
        biopodLeftPodY,
        TILE_ATTR(PAL3, FALSE, FALSE, FALSE)
    );

    // Light (animated)
    biopodLightSprite = SPR_addSprite(
        &biopodLightSpriteDef,
        biopodLightX,
        biopodLightY,
        TILE_ATTR(PAL3, FALSE, FALSE, FALSE)
    );

    while (1)
    {
        u16 joy = JOY_readJoypad(JOY_1);

        // -------------------------------------------------
        // Press A → open NPC dialogue window (black rectangle)
        // -------------------------------------------------
        if (joy & BUTTON_A)
        {
            runDialogue();
        }

        // -------------------------------------------------
        // Normal gameplay
        // -------------------------------------------------
        playerHandleInput();
        updateDepth();

        // ---- Room transitions ----

        // Left exit → Armory Vault
        if (playerX < EdgeRoomLeft + 1)
        {
            playerX = EnterRoomRight;
            playerY = 0x5A;
            return STATE_ARMORYVAULT;
        }

        // Right exit → Reactor Chamber
        if (playerX >= EdgeRoomRight)
        {
            playerX = EnterRoomLeft;
            playerY = 0x36;
            return STATE_REACTORCHAMBER;
        }

        drawDebugInfo();
        playerUpdateSprite();

        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_QUIT;
}
