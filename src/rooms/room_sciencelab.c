#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_sciencelab.h"
#include "player.h"
#include "game_state.h"
#include "room_holodeck.h"
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
static Sprite* scienceLabHelixSprite;
static Sprite* fingerSprite;
static Sprite* triggerASprite;

// ---------------------------------------------------------
// 3. Object positions
// ---------------------------------------------------------
static int scienceLabHelixX = 212;
static int scienceLabHelixY = 49;

// Science computer trigger point (converted from MSX)
// sciencelabcomputery = $54 + 4 + 4 + 4 = 96
// sciencelabcomputerx = 128 + 16 - 40 = 104
static const int scienceLabComputerX = 104;
static const int scienceLabComputerY = 96;

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
    SPR_setDepth(playerSprite, -playerY);
    SPR_setDepth(scienceLabHelixSprite, -scienceLabHelixY);

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

static void checkShowPressAIconScience(void)
{
    if (isPlayerNear(scienceLabComputerX, scienceLabComputerY))
    {
        showPressAIcon = 1;
        triggerAX = scienceLabComputerX;
        triggerAY = scienceLabComputerY;
        return;
    }

    showPressAIcon = 0;
}

static void updatePressAIconSpritesScience(void)
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
GameState runScienceLab(void)
{
    drawRoomBackground(ROOM_SCIENCELAB);
    playMusic(tune_ship);

    SPR_reset();

    // Player sprite
    playerSprite = SPR_addSprite(
        &playerSpriteDef,
        playerX, playerY,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );

    // Helix sprite
    scienceLabHelixSprite = SPR_addSprite(
        &scienceLabHelixSpriteDef,
        scienceLabHelixX,
        scienceLabHelixY,
        TILE_ATTR(PAL3, FALSE, FALSE, FALSE)
    );

    // Trigger-A sprites
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

        // 1. Room logic FIRST (so player doesn't walk out of range before check)
        checkShowPressAIconScience();

        // 2. THEN player movement
        playerHandleInput();

        // 3. Continue as normal
        updateDepth();
        updatePressAIconSpritesScience();

        // ---- Room transitions ----

        // Left exit → Holodeck
        if (playerX < EdgeRoomLeft + 1)
        {
            playerX = EnterRoomRight;
            playerY = 0x5A;
            return STATE_HOLODECK;
        }

        // Right exit → Hangar Bay
        if (playerX >= EdgeRoomRight)
        {
            playerX = EnterRoomLeft;
            playerY = 0x5A;
            return STATE_HANGARBAY;
        }

        drawDebugInfo();
        playerUpdateSprite();
        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_QUIT;
}
