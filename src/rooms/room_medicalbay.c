#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_medicalbay.h"
#include "player.h"
#include "game_state.h"
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

// Correct signature from npc_dialogue.c
extern void runDialogue(u8 whichText);

// ---------------------------------------------------------
// 2. Local sprite pointers
// ---------------------------------------------------------
static Sprite* medicalBayChairSprite;
static Sprite* medicalBayLightsSprite;

// Trigger A icon sprites
static Sprite* fingerSprite;
static Sprite* triggerASprite;

// ---------------------------------------------------------
// 3. Object positions
// ---------------------------------------------------------
static int medicalBayChairX = 0;
static int medicalBayChairY = 87;

static int medicalBayLightsX = 68;
static int medicalBayLightsY = 113;

// ---------------------------------------------------------
// 4. Trigger A system (MSX‑style names)
// ---------------------------------------------------------

static const int medicalbaybed1y = 0x54 + 10;
static const int medicalbaybed1x = 128 - 60;

static const int medicalbaybed2y = 0x54 + 2;
static const int medicalbaybed2x = 128 - 60;

static const int medicalbayEmbryosy = 0x54 - 4 - 30 - 8;
static const int medicalbayEmbryosx = 0x54 - 4 + 60 - 16;

static int showPressAIcon = 0;
static int triggerAX = 0;
static int triggerAY = 0;

static u32 frameCounter = 0;

// NEW: new‑press detector
static u16 prevJoy = 0;

// ---------------------------------------------------------
// 5. Medical Bay Explainer (bit 7, dialogue 22)
// ---------------------------------------------------------
static void handleMedicalExplainer(void)
{
    // Bit 7 = Medical Bay explainer
    if (gSave.convEntityShipExplanations & 0b10000000)
        return;

    // Wait 1 second (300 frames)
    if (!playerHasBeenInRoomFor(300))
        return;

    // Player must be centered
    if (!playerIsCenterScreen())
        return;

    // Mark bit 7 as shown
    gSave.convEntityShipExplanations |= 0b10000000;

    // Start Medical Bay explainer dialogue (NPCConv022)
    runDialogue(22);
}

// ---------------------------------------------------------
// 6. Depth sorting
// ---------------------------------------------------------
static void updateDepth(void)
{
    SPR_setDepth(playerSprite, -playerY);
    SPR_setDepth(medicalBayChairSprite, -medicalBayChairY + 10);

    SPR_setDepth(fingerSprite,     780);
    SPR_setDepth(triggerASprite,   760);
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

static void checkShowPressAIconMedical(void)
{
    // Embryos
    if (isPlayerNear(medicalbayEmbryosx, medicalbayEmbryosy))
    {
        showPressAIcon = 1;
        triggerAX = medicalbayEmbryosx;
        triggerAY = medicalbayEmbryosy;
        return;
    }

    // Bed 1
    if (isPlayerNear(medicalbaybed1x, medicalbaybed1y))
    {
        showPressAIcon = 1;
        triggerAX = medicalbaybed1x;
        triggerAY = medicalbaybed1y;
        return;
    }

    // Bed 2
    if (isPlayerNear(medicalbaybed2x, medicalbaybed2y))
    {
        showPressAIcon = 1;
        triggerAX = medicalbaybed2x;
        triggerAY = medicalbaybed2y;
        return;
    }

    showPressAIcon = 0;
}

static void updatePressAIconSpritesMedical(void)
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
// 8. Embryo Conversation Trigger
// ---------------------------------------------------------
static void checkStartEmbryoConversation(u16 joyNew)
{
    if (playerY >= 0x3A) return;
    if (!showPressAIcon) return;
    if (!(joyNew & BUTTON_A)) return;

    runDialogue(41);
}

// ---------------------------------------------------------
// 9. Healing Sequence Trigger
// ---------------------------------------------------------
static void checkStartHealingSequence(u16 joyNew)
{
    if (playerY < 0x3A) return;
    if (!showPressAIcon) return;
    if (!(joyNew & BUTTON_A)) return;

    playerStartHealing();
}

// ---------------------------------------------------------
// 10. Room logic
// ---------------------------------------------------------
GameState runMedicalBay(void)
{
    drawRoomBackground(ROOM_MEDICALBAY);
    playMusic(tune_ship);

    SPR_reset();

    playerSprite = SPR_addSprite(&playerSpriteDef, playerX, playerY,
                                 TILE_ATTR(PAL2, FALSE, FALSE, FALSE));

    medicalBayChairSprite = SPR_addSprite(&medicalBayChairSpriteDef,
                                          medicalBayChairX, medicalBayChairY,
                                          TILE_ATTR(PAL3, FALSE, FALSE, FALSE));

    medicalBayLightsSprite = SPR_addSprite(&medicalBayLightsSpriteDef,
                                           medicalBayLightsX, medicalBayLightsY,
                                           TILE_ATTR(PAL3, FALSE, FALSE, FALSE));

    // Trigger A sprites
    fingerSprite = SPR_addSprite(&fingerSpriteDef, 0, 0,
                                 TILE_ATTR(PAL2, FALSE, FALSE, FALSE));

    triggerASprite = SPR_addSprite(&triggerASpriteDef, 0, 0,
                                   TILE_ATTR(PAL2, FALSE, FALSE, FALSE));

    SPR_setVisibility(fingerSprite,   HIDDEN);
    SPR_setVisibility(triggerASprite, HIDDEN);

    showPressAIcon = 0;

    // NEW: mark room entry time
    playerMarkRoomEntry();

    while (1)
    {
        frameCounter++;

        // NEW‑PRESS DETECTOR
        u16 joy = JOY_readJoypad(JOY_1);
        u16 joyNew = joy & ~prevJoy;
        prevJoy = joy;

        // ---- Explainer ----
        handleMedicalExplainer();

        // Player logic
        playerHandleInput();

        // Room logic
        updateDepth();
        checkShowPressAIconMedical();
        updatePressAIconSpritesMedical();
        checkStartEmbryoConversation(joyNew);
        checkStartHealingSequence(joyNew);

        // Room transitions
        if (playerX >= EdgeRoomRight)
        {
            playerX = EnterRoomLeft;
            playerY = 0x5A;
            return STATE_HYDROPONICSBAY;
        }

        if (playerX < EdgeRoomLeft + 1)
        {
            playerX = EnterRoomRight;
            playerY = 0x5A;
            return STATE_SLEEPINGQUARTERS;
        }

        drawDebugInfo();

        SPR_update();
        playerUpdateSprite();
        SYS_doVBlankProcess();
    }

    return STATE_QUIT;
}
