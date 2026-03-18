#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_hydroponicsbay.h"
#include "player.h"
#include "game_state.h"
#include "room_sleepingquarters.h"
#include "save_data.h"

// ---------------------------------------------------------
// 1. Externs from other modules
// ---------------------------------------------------------
extern const u8* currentColMap;
extern u8 tileContent;

extern void drawRoomBackground(u8 room);
extern void drawDebugInfo(void);

// Shared global tile index from main.c
extern u16 globalTileIndex;

// Player globals
extern Sprite* playerSprite;
extern int playerX;
extern int playerY;

// ---------------------------------------------------------
// 2. Local sprite pointers
// ---------------------------------------------------------
static Sprite* hydroponicsBayFoodLeftSprite;
static Sprite* hydroponicsBayFoodRightSprite;

// ---------------------------------------------------------
// 3. Feet positions for depth sorting
// ---------------------------------------------------------
static int foodLeftX  =  0;
static int foodLeftY  = 96;

static int foodRightX = 160;
static int foodRightY = 136;

// ---------------------------------------------------------
// 4. Hydroponics Explainer (bit 0, dialogue 15)
// ---------------------------------------------------------
static void handleHydroponicsExplainer(void)
{
    // Bit 0 = Hydroponics Bay explainer
    if (gSave.convEntityShipExplanations & 0b00000001)
        return;

    // Wait 1 second (300 frames)
    if (!playerHasBeenInRoomFor(300))
        return;

    // Player must be centered
    if (!playerIsCenterScreen())
        return;

    // Mark bit 0 as shown
    gSave.convEntityShipExplanations |= 0b00000001;

    // Start Hydroponics explainer dialogue (NPCConv015)
    runDialogue(15);
}

// ---------------------------------------------------------
// 5. Unified depth sorting
// ---------------------------------------------------------
static void updateDepth(void)
{
    SPR_setDepth(playerSprite, -playerY);

    SPR_setDepth(hydroponicsBayFoodLeftSprite,  -foodLeftY  - 2);
    SPR_setDepth(hydroponicsBayFoodRightSprite, -foodRightY + 30);
}

// ---------------------------------------------------------
// 6. Room logic
// ---------------------------------------------------------
GameState runHydroponicsBay(void)
{
    drawRoomBackground(ROOM_HYDROPONICSBAY);
    playMusic(tune_ship);

    SPR_reset();

    // Player
    playerSprite = SPR_addSprite(
        &playerSpriteDef,
        playerX,
        playerY,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );

    // Food Left
    hydroponicsBayFoodLeftSprite = SPR_addSprite(
        &hydroponicsBayFoodLeftSpriteDef,
        foodLeftX,
        foodLeftY,
        TILE_ATTR(PAL3, FALSE, FALSE, FALSE)
    );

    // Food Right
    hydroponicsBayFoodRightSprite = SPR_addSprite(
        &hydroponicsBayFoodRightSpriteDef,
        foodRightX,
        foodRightY,
        TILE_ATTR(PAL3, FALSE, FALSE, FALSE)
    );

    // NEW: mark room entry time
    playerMarkRoomEntry();

    // -----------------------------------------------------
    // Main room loop
    // -----------------------------------------------------
    while (1)
    {
        // ---- Explainer ----
        handleHydroponicsExplainer();

        playerHandleInput();
        updateDepth();

        // ---- Room transition logic ----

        // Left exit → Medical Bay
        if (playerX < EdgeRoomLeft + 1)
        {
            playerX = EnterRoomRight;
            playerY = 0x5A;
            return STATE_MEDICALBAY;
        }

        // Right exit → Armory Vault
        if (playerX >= EdgeRoomRight)
        {
            playerX = EnterRoomLeft;
            playerY = 0x5A;
            return STATE_ARMORYVAULT;
        }

        drawDebugInfo();
        playerUpdateSprite();

        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_QUIT;
}
