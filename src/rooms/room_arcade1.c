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
// 2. Local sprite pointers (player + NPCs + props)
// ---------------------------------------------------------
static Sprite* girlSprite;
static Sprite* capgirlSprite;
static Sprite* redheadboySprite;

static Sprite* arcade1WallSprite;      // NEW
static Sprite* arcade1OpenDoorSprite;  // NEW

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
// 4. Prop positions (NEW)
// ---------------------------------------------------------
static int wallX       = 156;     // adjust as needed
static int wallY       = 76;

static int openDoorX   = 108;   // adjust as needed
static int openDoorY   = 57;

// ---------------------------------------------------------
// 5. Unified depth sorting (correct top‑down behavior)
// ---------------------------------------------------------
static void updateDepth(void)
{
    // Props behind everything
    SPR_setDepth(arcade1WallSprite,     900);
    SPR_setDepth(arcade1OpenDoorSprite, 850);

    // Player + NPCs
    SPR_setDepth(playerSprite,      -playerY);
    SPR_setDepth(girlSprite,        -girlY);
    SPR_setDepth(capgirlSprite,     -capgirlY);
    SPR_setDepth(redheadboySprite,  -redheadboyY);
}

// ---------------------------------------------------------
// 6. Room logic
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
    // Props (NEW)
    // -----------------------------------------------------
    arcade1WallSprite = SPR_addSprite(
        &arcade1WallSpriteDef,
        wallX,
        wallY,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );

    arcade1OpenDoorSprite = SPR_addSprite(
        &arcade1OpenDoorSpriteDef,
        openDoorX,
        openDoorY,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );

    // -----------------------------------------------------
    // NPCs
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
