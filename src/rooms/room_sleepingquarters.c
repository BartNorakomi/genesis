#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_sleepingquarters.h"
#include "player.h"
#include "game_state.h"
#include "room_arcade1.h"

// ---------------------------------------------------------
// 1. Externs from other modules
// ---------------------------------------------------------
extern const u8* currentColMap;
extern u8 tileContent;

extern void drawRoomBackground(u8 room);
extern void drawDebugInfo(void);
extern void playMusic(const u8* track);

// ---------------------------------------------------------
// 2. Local sprite pointers
// ---------------------------------------------------------
static Sprite* sleepingQuartersLightsSprite;

// ---------------------------------------------------------
// 3. Lights position (feet position for depth sorting)
// ---------------------------------------------------------
static int sleepingQuartersLightsX = 82;
static int sleepingQuartersLightsY = 76;

// ---------------------------------------------------------
// 4. MSX-style animation table (64 steps)
// ---------------------------------------------------------
// 0..12 forward
// hold 12 for 16 steps
// 12..0 backward
// hold 0 for 16 steps
static const u8 sleepingLightsAnim[64] =
{
    // 0..12
    0,1,2,3,4,5,6,7,
    8,9,10,11,12,

    // hold 12 (16 frames)
    12,12,12,12,12,12,12,12,
    12,12,12,12,12,12,12,12,

    // 12..0
    11,10,9,8,7,6,5,4,
    3,2,1,0,

    // hold 0 (16 frames)
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0
};

// ---------------------------------------------------------
// 5. Animation state
// ---------------------------------------------------------
static u8 lightStep = 0;   // 0..63
static u8 lightSlow = 0;   // like framecounter

// ---------------------------------------------------------
// 6. Room logic
// ---------------------------------------------------------
GameState runSleepingQuarters(void)
{
    drawRoomBackground(ROOM_SLEEPINGQUARTERS);
    playMusic(tune_ship);

    SPR_reset();
    playerSprite = SPR_addSprite(
        &playerSpriteDef,
        playerX,
        playerY,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );

    sleepingQuartersLightsSprite = SPR_addSprite(
        &sleepingQuartersLightsSpriteDef,
        sleepingQuartersLightsX,
        sleepingQuartersLightsY,
        TILE_ATTR(PAL0, FALSE, FALSE, FALSE)
    );

    // Reset animation state
    lightStep = 0;
    lightSlow = 0;

    while (1)
    {
        playerHandleInput();

        // ---- Room transition logic ----

        if (playerX < EdgeRoomLeft + 1)
        {
            playerX = EnterRoomRight;
            playerY = 0x5A;
            return STATE_TRAININGDECK;
        }

        if (playerX >= EdgeRoomRight)
        {
            playerX = EnterRoomLeft;
            playerY = 0x5A;
            return STATE_MEDICALBAY;
        }

        drawDebugInfo();
        playerUpdateSprite();

        // -------------------------------------------------
        // MSX-style table-driven animation
        // -------------------------------------------------
        lightSlow++;
        if ((lightSlow & 7) == 0)   // update every 8 frames
        {
            lightStep = (lightStep + 1) & 63;   // 0..63 loop
            u8 frame = sleepingLightsAnim[lightStep];
            SPR_setFrame(sleepingQuartersLightsSprite, frame);
        }

        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_QUIT;
}
