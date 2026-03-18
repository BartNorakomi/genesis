#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_hangarbay.h"
#include "player.h"
#include "game_state.h"
#include "room_sciencelab.h"
#include "room_arcade1.h"
#include "save_data.h"

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
static Sprite* drillingMachineSprite;
static Sprite* fingerSprite;
static Sprite* triggerASprite;

// ---------------------------------------------------------
// 3. Object positions
// ---------------------------------------------------------
static int drillingMachineX = 118;
static int drillingMachineY = 85;

static const int hangarDrillX = 140;
static const int hangarDrillY = 88;

// ---------------------------------------------------------
// 4. Trigger A icon state
// ---------------------------------------------------------
static int showPressAIcon = 0;
static int triggerAX = 0;
static int triggerAY = 0;

static u32 frameCounter = 0;

// ---------------------------------------------------------
// 5. Hangar Bay Explainer (bit 1, dialogue 16)
// ---------------------------------------------------------
static void handleHangarExplainer(void)
{
    // Bit 1 = Hangar Bay explainer
    if (gSave.convEntityShipExplanations & 0b00000010)
        return;

    // Wait 1 second (300 frames)
    if (!playerHasBeenInRoomFor(300))
        return;

    // Player must be centered
    if (!playerIsCenterScreen())
        return;

    // Mark bit 1 as shown
    gSave.convEntityShipExplanations |= 0b00000010;

    // Start Hangar Bay explainer dialogue (NPCConv016)
    runDialogue(16);
}

// ---------------------------------------------------------
// 6. Depth sorting
// ---------------------------------------------------------
static void updateDepth(void)
{
    SPR_setDepth(playerSprite, -playerY);
    SPR_setDepth(drillingMachineSprite, -drillingMachineY);

    SPR_setDepth(fingerSprite, 780);
    SPR_setDepth(triggerASprite, 760);
}

// ---------------------------------------------------------
// 7. Trigger helpers
// ---------------------------------------------------------
static bool isPlayerNear(int tx, int ty)
{
    int dx = abs(playerX - tx);
    int dy = abs(playerY - ty);
    return (dx < 32 && dy < 16);
}

static void checkShowPressAIconHangar(void)
{
    if (isPlayerNear(hangarDrillX, hangarDrillY))
    {
        showPressAIcon = 1;
        triggerAX = hangarDrillX;
        triggerAY = hangarDrillY;
        return;
    }

    showPressAIcon = 0;
}

static void updatePressAIconSpritesHangar(void)
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
// 8. Room logic
// ---------------------------------------------------------
GameState runHangarBay(void)
{
    drawRoomBackground(ROOM_HANGARBAY);
    playMusic(tune_ship);

    SPR_reset();

    // Player sprite
    playerSprite = SPR_addSprite(
        &playerSpriteDef,
        playerX, playerY,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );

    // Drilling machine sprite
    drillingMachineSprite = SPR_addSprite(
        &hangarBayDrillingMachineSpriteDef,
        drillingMachineX,
        drillingMachineY,
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

    // NEW: mark room entry time
    playerMarkRoomEntry();

    while (1)
    {
        frameCounter++;

        // ---- Explainer ----
        handleHangarExplainer();

        // ---- Trigger A logic ----
        checkShowPressAIconHangar();

        // ---- Player movement ----
        playerHandleInput();

        updateDepth();
        updatePressAIconSpritesHangar();

        // ---- Room transitions ----

        // Left exit → Science Lab
        if (playerX < EdgeRoomLeft + 1)
        {
            playerX = EnterRoomRight;
            playerY = 0x5A + 20;
            return STATE_SCIENCELAB;
        }

        // Right exit → Training Deck
        if (playerX >= EdgeRoomRight)
        {
            playerX = EnterRoomLeft;
            playerY = 0x5A - 20;
            return STATE_TRAININGDECK;
        }

        drawDebugInfo();
        playerUpdateSprite();
        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_QUIT;
}
