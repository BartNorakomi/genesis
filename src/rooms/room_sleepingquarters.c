#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_sleepingquarters.h"
#include "player.h"
#include "game_state.h"
#include "room_arcade1.h"

// ---------------------------------------------------------
// 1. Externs
// ---------------------------------------------------------
extern const u8* currentColMap;
extern u8 tileContent;

extern void drawRoomBackground(u8 room);
extern void drawDebugInfo(void);

extern Sprite* playerSprite;
extern int playerX;
extern int playerY;


// ---------------------------------------------------------
// 2. Local sprite pointers
// ---------------------------------------------------------
static Sprite* sleepingQuartersLightsSprite;

// Trigger A icon sprites
static Sprite* fingerSprite;
static Sprite* triggerASprite;


// ---------------------------------------------------------
// 3. Object positions
// ---------------------------------------------------------
static int sleepingQuartersLightsX = 82;
static int sleepingQuartersLightsY = 76;


// ---------------------------------------------------------
// 4. Trigger A system (MSX style)
// ---------------------------------------------------------

// Bed position from MSX code
static const int sleepingbedy = 104 - 16;
static const int sleepingbedx = 60 - 8;

static int showPressAIcon = 0;
static int triggerAX = 0;
static int triggerAY = 0;

static u32 frameCounter = 0;

// NEW-PRESS detector
static u16 prevJoy = 0;


// ---------------------------------------------------------
// 5. Depth sorting
// ---------------------------------------------------------
static void updateDepth(void)
{
    SPR_setDepth(playerSprite, -playerY);
    SPR_setDepth(sleepingQuartersLightsSprite, -sleepingQuartersLightsY + 10);

    SPR_setDepth(fingerSprite, 780);
    SPR_setDepth(triggerASprite, 760);
}


// ---------------------------------------------------------
// 6. Trigger helpers
// ---------------------------------------------------------
static bool isPlayerNear(int tx, int ty)
{
    int dx = abs(playerX - tx);
    int dy = abs(playerY - ty);

    return (dx < 32 && dy < 16);
}


static void checkShowPressAIconSleeping(void)
{
    if (isPlayerNear(sleepingbedx, sleepingbedy))
    {
        showPressAIcon = 1;

        triggerAX = sleepingbedx;
        triggerAY = sleepingbedy;

        return;
    }

    showPressAIcon = 0;
}


static void updatePressAIconSpritesSleeping(void)
{
    if (!showPressAIcon)
    {
        SPR_setVisibility(fingerSprite, HIDDEN);
        SPR_setVisibility(triggerASprite, HIDDEN);
        return;
    }

    SPR_setVisibility(fingerSprite, VISIBLE);
    SPR_setVisibility(triggerASprite, VISIBLE);

    int bob = (frameCounter & 31) < 16 ? 1 : -1;

    SPR_setPosition(fingerSprite,  triggerAX - 2, triggerAY - 29 + bob);
    SPR_setPosition(triggerASprite, triggerAX,     triggerAY - 14);
}


// ---------------------------------------------------------
// 7. Sleeping / Healing trigger
// ---------------------------------------------------------
static void checkStartSleepingSequence(u16 joyNew)
{
    if (!showPressAIcon) return;

    if (!(joyNew & BUTTON_A)) return;

    playerStartHealing();
}


// ---------------------------------------------------------
// 8. MSX-style lights animation table
// ---------------------------------------------------------
static const u8 sleepingLightsAnim[64] =
{
    0,1,2,3,4,5,6,7,
    8,9,10,11,12,

    12,12,12,12,12,12,12,12,
    12,12,12,12,12,12,12,12,

    11,10,9,8,7,6,5,4,
    3,2,1,0,

    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0
};

static u8 lightStep = 0;
static u8 lightSlow = 0;


// ---------------------------------------------------------
// 9. Room logic
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


    // Trigger A sprites
    fingerSprite = SPR_addSprite(
        &fingerSpriteDef,
        0,0,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );

    triggerASprite = SPR_addSprite(
        &triggerASpriteDef,
        0,0,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );


    SPR_setVisibility(fingerSprite, HIDDEN);
    SPR_setVisibility(triggerASprite, HIDDEN);

    showPressAIcon = 0;

    lightStep = 0;
    lightSlow = 0;


    while (1)
    {
        frameCounter++;


        // NEW PRESS DETECTOR
        u16 joy = JOY_readJoypad(JOY_1);
        u16 joyNew = joy & ~prevJoy;
        prevJoy = joy;


        // Player logic
        playerHandleInput();


        // Room logic
        updateDepth();

        checkShowPressAIconSleeping();
        updatePressAIconSpritesSleeping();
        checkStartSleepingSequence(joyNew);


        // -------------------------------------------------
        // Room transitions
        // -------------------------------------------------

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


        // -------------------------------------------------
        // Lights animation
        // -------------------------------------------------

        lightSlow++;

        if ((lightSlow & 7) == 0)
        {
            lightStep = (lightStep + 1) & 63;

            u8 frame = sleepingLightsAnim[lightStep];

            SPR_setFrame(sleepingQuartersLightsSprite, frame);
        }


        SPR_update();

        playerUpdateSprite();

        SYS_doVBlankProcess();
    }

    return STATE_QUIT;
}