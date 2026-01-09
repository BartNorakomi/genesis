#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_arcade1.h"
#include "player.h"
#include "game_state.h"
#include "room_sleepingquarters.h"

// ---------------------------------------------------------
// 1. Externs from other modules
// ---------------------------------------------------------
extern const u8* currentColMap;
extern u8 tileContent;

extern void drawRoomBackground(u8 room);
extern void drawDebugInfo(void);

// ---------------------------------------------------------
// 2. Local sprite pointers (player + NPCs)
// ---------------------------------------------------------
static Sprite* girlSprite;
static Sprite* capgirlSprite;
static Sprite* redheadboySprite;

// ---------------------------------------------------------
// 3. NPC positions (easy to adjust)
// ---------------------------------------------------------
static int girlX       = 50;
static int girlY       = 98;

static int capgirlX    = 160;
static int capgirlY    = 108;

static int redheadboyX = 240;
static int redheadboyY = 78;

// ---------------------------------------------------------
// 4. Unified depth sorting (correct top‑down behavior)
// ---------------------------------------------------------
// Using negative Y gives:
//   - player BELOW NPC (higher Y) → deeper → in front
//   - player ABOVE NPC (lower Y) → shallower → behind
static void updateDepth(void)
{
    SPR_setDepth(playerSprite, -playerY);
    SPR_setDepth(girlSprite,       -girlY);
    SPR_setDepth(capgirlSprite,    -capgirlY);
    SPR_setDepth(redheadboySprite, -redheadboyY);
}

// ---------------------------------------------------------
// 5. Room logic
// ---------------------------------------------------------
GameState runArcade1(void)
{
    drawRoomBackground(ROOM_ARCADE1);
    playMusic(tune_arcadehall);

    SPR_reset();

    // Player sprite
    playerSprite = SPR_addSprite(
        &playerSpriteDef,
        playerX,
        playerY,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );

    // -----------------------------------------------------
    // NPCs (static sprites with adjustable X/Y)
    // -----------------------------------------------------
    girlSprite = SPR_addSprite(
        &girlSpriteDef,
        girlX + PLAYERANDNPC_OFFSET_X,
        girlY + PLAYERANDNPC_OFFSET_Y,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );

    capgirlSprite = SPR_addSprite(
        &capgirlSpriteDef,
        capgirlX + PLAYERANDNPC_OFFSET_X,
        capgirlY + PLAYERANDNPC_OFFSET_Y,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );

    redheadboySprite = SPR_addSprite(
        &redheadboySpriteDef,
        redheadboyX + PLAYERANDNPC_OFFSET_X,
        redheadboyY + PLAYERANDNPC_OFFSET_Y,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );

    // -----------------------------------------------------
    // Main room loop
    // -----------------------------------------------------
    while (1)
    {
        playerHandleInput();
        updateDepth();

        if (JOY_readJoypad(JOY_1) & BUTTON_START)
            return STATE_SLEEPINGQUARTERS;

        drawDebugInfo();
        playerUpdateSprite();

        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_QUIT;
}
