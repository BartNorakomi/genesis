#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_trainingdeck.h"
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
static Sprite* trainingDeckTreadmillSprite;
static Sprite* trainingDeckWallLeftSprite;
static Sprite* trainingDeckWallRightSprite;

static Sprite* fingerSprite;
static Sprite* triggerASprite;

// ---------------------------------------------------------
// 3. Object positions
// ---------------------------------------------------------
static int trainingDeckTreadmillX = 11;
static int trainingDeckTreadmillY = 132;

static int trainingDeckWallLeftX = 0;
static int trainingDeckWallLeftY = 90;

static int trainingDeckWallRightX = 250;
static int trainingDeckWallRightY = 103;

// MSX → SGDK conversion:
// trainingdecktreadmilly = 104
// trainingdecktreadmillx = 60 - 8 = 52
static const int treadmillTriggerX = 52;
static const int treadmillTriggerY = 104;

// ---------------------------------------------------------
// 4. Trigger A icon state
// ---------------------------------------------------------
static int showPressAIcon = 0;
static int triggerAX = 0;
static int triggerAY = 0;

static u32 frameCounter = 0;

// ---------------------------------------------------------
// 5. Depth sorting
// ---------------------------------------------------------
static void updateDepth(void)
{
    SPR_setDepth(trainingDeckWallLeftSprite,  -trainingDeckWallLeftY + 7);
    SPR_setDepth(trainingDeckWallRightSprite, -trainingDeckWallRightY + 4);

    SPR_setDepth(playerSprite, -playerY);
    SPR_setDepth(trainingDeckTreadmillSprite, -trainingDeckTreadmillY + 34);

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

static void checkShowPressAIconTraining(void)
{
    if (isPlayerNear(treadmillTriggerX, treadmillTriggerY))
    {
        showPressAIcon = 1;
        triggerAX = treadmillTriggerX;
        triggerAY = treadmillTriggerY;
        return;
    }

    showPressAIcon = 0;
}

static void updatePressAIconSpritesTraining(void)
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
// 7. Room logic
// ---------------------------------------------------------
GameState runTrainingDeck(void)
{
    drawRoomBackground(ROOM_TRAININGDECK);
    playMusic(tune_ship);

    SPR_reset();

    // --- Player sprite ---
    playerSprite = SPR_addSprite(
        &playerSpriteDef,
        playerX,
        playerY,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );

    // --- Walls ---
    trainingDeckWallLeftSprite = SPR_addSprite(
        &trainingDeckWallLeftSpriteDef,
        trainingDeckWallLeftX,
        trainingDeckWallLeftY,
        TILE_ATTR(PAL3, FALSE, FALSE, FALSE)
    );

    trainingDeckWallRightSprite = SPR_addSprite(
        &trainingDeckWallRightSpriteDef,
        trainingDeckWallRightX,
        trainingDeckWallRightY,
        TILE_ATTR(PAL3, FALSE, FALSE, FALSE)
    );

    // --- Treadmill ---
    trainingDeckTreadmillSprite = SPR_addSprite(
        &trainingDeckTreadmillSpriteDef,
        trainingDeckTreadmillX,
        trainingDeckTreadmillY,
        TILE_ATTR(PAL3, FALSE, FALSE, FALSE)
    );

    // --- Trigger-A sprites ---
    fingerSprite = SPR_addSprite(&fingerSpriteDef, 0, 0,
                                 TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    triggerASprite = SPR_addSprite(&triggerASpriteDef, 0, 0,
                                   TILE_ATTR(PAL2, FALSE, FALSE, FALSE));

    SPR_setVisibility(fingerSprite, HIDDEN);
    SPR_setVisibility(triggerASprite, HIDDEN);

    showPressAIcon = 0;

    while (1)
    {
        frameCounter++;

        // 1. Room logic FIRST
        checkShowPressAIconTraining();

        // 2. THEN player movement
        playerHandleInput();

        // 3. Continue as normal
        updateDepth();
        updatePressAIconSpritesTraining();

        // ---- Room transitions ----

        // Left exit → Hangar Bay
        if (playerX < EdgeRoomLeft + 1)
        {
            playerX = EnterRoomRight;
            playerY = 0x5A;
            return STATE_HANGARBAY;
        }

        // Right exit → Sleeping Quarters
        if (playerX >= EdgeRoomRight)
        {
            playerX = EnterRoomLeft;
            playerY = 0x5A + 10;
            return STATE_SLEEPINGQUARTERS;
        }

        drawDebugInfo();
        playerUpdateSprite();
        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_QUIT;
}
