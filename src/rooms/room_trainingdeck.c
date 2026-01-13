#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_trainingdeck.h"
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

// ---------------------------------------------------------
// 2. Local sprite pointers
// ---------------------------------------------------------
static Sprite* trainingDeckTreadmillSprite;
static Sprite* trainingDeckWallLeftSprite;
static Sprite* trainingDeckWallRightSprite;

// ---------------------------------------------------------
// 3. Object positions (feet positions for depth sorting)
// ---------------------------------------------------------
static int trainingDeckTreadmillX = 11;
static int trainingDeckTreadmillY = 132;

static int trainingDeckWallLeftX = 0;    // adjust to match room art
static int trainingDeckWallLeftY = 90;

static int trainingDeckWallRightX = 250; // adjust to match room art
static int trainingDeckWallRightY = 103;

// ---------------------------------------------------------
// 4. Depth sorting
// ---------------------------------------------------------
static void updateDepth(void)
{
    // walls always behind player + treadmill
    SPR_setDepth(trainingDeckWallLeftSprite,  -trainingDeckWallLeftY + 7);
    SPR_setDepth(trainingDeckWallRightSprite, -trainingDeckWallRightY + 4);

    SPR_setDepth(playerSprite, -playerY);
    SPR_setDepth(trainingDeckTreadmillSprite, -trainingDeckTreadmillY + 34);
}

// ---------------------------------------------------------
// 5. Room logic
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

    while (1)
    {
        playerHandleInput();
        updateDepth();

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
