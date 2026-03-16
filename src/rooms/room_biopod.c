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

extern Sprite* playerSprite;
extern int playerX;
extern int playerY;

// Correct signature from npc_dialogue.c
extern void runDialogue(u8 whichText);

// ---------------------------------------------------------
// 2. Local sprite pointers
// ---------------------------------------------------------
static Sprite* biopodRightPodSprite;
static Sprite* biopodLeftPodSprite;
static Sprite* biopodLightSprite;

// Trigger A icon sprites
static Sprite* fingerSprite;
static Sprite* triggerASprite;

// ---------------------------------------------------------
// 3. Feet positions for depth sorting
// ---------------------------------------------------------
static int biopodRightPodX = 156;
static int biopodRightPodY = 86;

static int biopodLeftPodX  = 68;
static int biopodLeftPodY  = 114;

static int biopodLightX    = 126;
static int biopodLightY    = 113;

// Trigger-A object (communication device) position
static const int commDeviceX = 124;
static const int commDeviceY = 54;

// Trigger-A variables
static int showPressAIcon = 0;
static int triggerAX = 0;
static int triggerAY = 0;

static u32 frameCounter = 0;

// NEW-PRESS detector
static u16 prevJoy = 0;

// ---------------------------------------------------------
// 4. Unified depth sorting
// ---------------------------------------------------------
static void updateDepth(void)
{
    SPR_setDepth(playerSprite, -playerY);
    SPR_setDepth(biopodRightPodSprite, -biopodRightPodY + 30);
    SPR_setDepth(biopodLeftPodSprite,  -biopodLeftPodY + 60);

    SPR_setDepth(fingerSprite, 780);
    SPR_setDepth(triggerASprite, 760);
}

// ---------------------------------------------------------
// 5. Trigger helpers
// ---------------------------------------------------------
static bool isPlayerNear(int tx, int ty)
{
    int dx = abs(playerX - tx);
    int dy = abs(playerY - ty);
    return (dx < 32 && dy < 16);
}

static void checkShowPressAIconBiopod(void)
{
    if (isPlayerNear(commDeviceX, commDeviceY))
    {
        showPressAIcon = 1;
        triggerAX = commDeviceX;
        triggerAY = commDeviceY;
        return;
    }

    showPressAIcon = 0;
}

static void updatePressAIconSpritesBiopod(void)
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

static void checkStartConversation(u16 joyNew)
{
    if (!showPressAIcon) return;

    if (!(joyNew & BUTTON_A)) return;

    // Start conversation 6 (decimal)
    runDialogue(14);
}

// ---------------------------------------------------------
// 6. Room logic
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

        // NEW-PRESS detector
        u16 joy = JOY_readJoypad(JOY_1);
        u16 joyNew = joy & ~prevJoy;
        prevJoy = joy;

        // Player input
        playerHandleInput();

        // Room logic
        updateDepth();
        checkShowPressAIconBiopod();
        updatePressAIconSpritesBiopod();
        checkStartConversation(joyNew);

        // ---- Room transitions ----
        if (playerX < EdgeRoomLeft + 1)
        {
            playerX = EnterRoomRight;
            playerY = 0x5A;
            return STATE_ARMORYVAULT;
        }

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