#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_holodeck.h"
#include "player.h"
#include "game_state.h"
#include "room_reactorchamber.h"
#include "room_arcade1.h"

// ---------------------------------------------------------
// 1. Externs
// ---------------------------------------------------------
extern const u8* currentColMap;
extern u8 tileContent;

extern void drawRoomBackground(u8 room);
extern void drawDebugInfo(void);

// Player globals
extern Sprite* playerSprite;
extern int playerX;
extern int playerY;

// ---------------------------------------------------------
// 2. Local sprite pointers
// ---------------------------------------------------------
static Sprite* holodeckDoorSprite;
static Sprite* holodeckFloorSprite;

// Trigger A icon sprites
static Sprite* fingerSprite;
static Sprite* triggerASprite;

// ---------------------------------------------------------
// 3. Door animation state
// ---------------------------------------------------------
static int holodeckDoorX = 0;
static int holodeckDoorY = 20;

static u8 doorAction = 0;
static u8 doorFrame  = 0;

static u32 frameCounter = 0;

// ---------------------------------------------------------
// 4. Door animation tables
// ---------------------------------------------------------
static const u8 holodeckOpenFrames[11]  = {0,1,2,3,4,5,6,7,8,9,10};
static const u8 holodeckCloseFrames[11] = {10,9,8,7,6,5,4,3,2,1,0};

// ---------------------------------------------------------
// 5. Trigger A icon state
// ---------------------------------------------------------
static const int holodeckTriggerX = 150;
static const int holodeckTriggerY = 58;

static int showPressAIcon = 0;
static int triggerAX = 0;
static int triggerAY = 0;

// NEW: custom new‑press detector
static u16 prevJoy = 0;

// ---------------------------------------------------------
// 6. Door animation helpers
// ---------------------------------------------------------
static void holodeckDoorSetFrame(void)
{
    if (doorAction == 1)
        SPR_setFrame(holodeckDoorSprite, holodeckOpenFrames[doorFrame]);
    else if (doorAction == 2)
        SPR_setFrame(holodeckDoorSprite, holodeckCloseFrames[doorFrame]);
}

static void holodeckDoorAnimate(void)
{
    if (doorAction == 0) return;
    if ((frameCounter % 3) != 0) return;

    holodeckDoorSetFrame();
    doorFrame++;

    if (doorFrame >= 11)
    {
        doorFrame = 0;
        doorAction = 0;
    }
}

static void holodeckDoorCheck(void)
{
    if (doorAction != 0) return;

    if (playerX < 110)
    {
        if (holodeckDoorSprite->frameInd != 10)
        {
            doorAction = 1;
            doorFrame = 0;
        }
        return;
    }

    if (holodeckDoorSprite->frameInd != 0)
    {
        doorAction = 2;
        doorFrame = 0;
    }
}

// ---------------------------------------------------------
// 7. Trigger A icon logic
// ---------------------------------------------------------
static bool isPlayerNearHolodeckTrigger(void)
{
    int dx = abs(playerX - holodeckTriggerX);
    int dy = abs(playerY - holodeckTriggerY);
    return (dx < 32 && dy < 16);
}

static void checkShowPressAIconHolodeck(void)
{
    if (isPlayerNearHolodeckTrigger())
    {
        showPressAIcon = 1;
        triggerAX = holodeckTriggerX;
        triggerAY = holodeckTriggerY;
    }
    else
    {
        showPressAIcon = 0;
    }
}

static void updatePressAIconSpritesHolodeck(void)
{
    if (showPressAIcon == 0)
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
// 8. Depth sorting
// ---------------------------------------------------------
static void updateDepth(void)
{
    SPR_setDepth(playerSprite, -playerY);
    SPR_setDepth(holodeckDoorSprite, -holodeckDoorY - 80);
    SPR_setDepth(fingerSprite,     780);
    SPR_setDepth(triggerASprite,   760);
}

// ---------------------------------------------------------
// 9. Main room logic
// ---------------------------------------------------------
GameState runHoloDeck(void)
{
    drawRoomBackground(ROOM_HOLODECK);
    playMusic(tune_ship);

    SPR_reset();

    playerSprite = SPR_addSprite(&playerSpriteDef, playerX, playerY,
                                 TILE_ATTR(PAL2, FALSE, FALSE, FALSE));

    holodeckFloorSprite = SPR_addSprite(&holodeckFloorSpriteDef,
                                        0, 137,
                                        TILE_ATTR(PAL3, FALSE, FALSE, FALSE));

    holodeckDoorSprite = SPR_addSprite(&holodeckDoorSpriteDef,
                                       holodeckDoorX, holodeckDoorY,
                                       TILE_ATTR(PAL3, FALSE, FALSE, FALSE));

    fingerSprite = SPR_addSprite(&fingerSpriteDef,
                                 holodeckTriggerX, holodeckTriggerY,
                                 TILE_ATTR(PAL2, FALSE, FALSE, FALSE));

    triggerASprite = SPR_addSprite(&triggerASpriteDef,
                                   holodeckTriggerX, holodeckTriggerY,
                                   TILE_ATTR(PAL2, FALSE, FALSE, FALSE));

    SPR_setVisibility(fingerSprite,   HIDDEN);
    SPR_setVisibility(triggerASprite, HIDDEN);

    showPressAIcon = 0;

    while (1)
    {
        frameCounter++;

        // NEW‑PRESS DETECTOR
        u16 joy = JOY_readJoypad(JOY_1);
        u16 joyNew = joy & ~prevJoy;
        prevJoy = joy;

        playerHandleInput();
        updateDepth();

        holodeckDoorCheck();
        holodeckDoorAnimate();

        checkShowPressAIconHolodeck();
        updatePressAIconSpritesHolodeck();

        // NEW PRESS A → return to Arcade Hall 1
        if (showPressAIcon != 0 && (joyNew & BUTTON_A))
        {
            playerX = 130;
            playerY = 102;
            return STATE_ARCADE1;
        }

        // Left exit → Reactor Chamber
        if (playerX < EdgeRoomLeft + 1)
        {
            SPR_setVisibility(holodeckFloorSprite, HIDDEN);
            SPR_update();

            playerX = EnterRoomRight;
            playerY = 0x5A - 36;
            return STATE_REACTORCHAMBER;
        }

        // Right exit → Science Lab
        if (playerX >= EdgeRoomRight)
        {
            playerX = EnterRoomLeft;
            playerY = 0x5A + 16;
            return STATE_SCIENCELAB;
        }

        drawDebugInfo();
        playerUpdateSprite();

        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_QUIT;
}
