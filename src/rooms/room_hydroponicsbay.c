#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_hydroponicsbay.h"
#include "player.h"
#include "game_state.h"
#include "room_sleepingquarters.h"

// ---------------------------------------------------------
// 1. Externs from other modules
// ---------------------------------------------------------
extern const u8* currentColMap;
extern u8 tileContent;

extern void drawRoomBackground(u8 room);
extern void drawDebugInfo(void);

// Shared global tile index from main.c
extern u16 globalTileIndex;

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
// 4. Unified depth sorting
// ---------------------------------------------------------
static void updateDepth(void)
{
    SPR_setDepth(playerSprite, -playerY);

    // Adjust offsets as needed for correct layering
    SPR_setDepth(hydroponicsBayFoodLeftSprite,  -foodLeftY  - 2);
    SPR_setDepth(hydroponicsBayFoodRightSprite, -foodRightY + 30);
}

// ---------------------------------------------------------
// 5. Room logic
// ---------------------------------------------------------
GameState runHydroponicsBay(void)
{
    // Background is drawn first and sets globalTileIndex
    drawRoomBackground(ROOM_HYDROPONICSBAY);
    playMusic(tune_ship);

    // // Load FG tileset using the shared global index
    // VDP_loadTileSet(hydroponicsbay_fg.tileset, globalTileIndex, DMA);

    // // Draw FG image on BG_A using the shared tile index
    // VDP_drawImageEx(
    //     BG_A,
    //     &hydroponicsbay_fg,
    //     TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, globalTileIndex),
    //     0, 0,
    //     FALSE,
    //     TRUE
    // );

    // // Advance global tile index for next room assets
    // globalTileIndex += hydroponicsbay_fg.tileset->numTile;

    // -----------------------------------------------------
    // Sprite setup
    // -----------------------------------------------------
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

    // -----------------------------------------------------
    // Main room loop
    // -----------------------------------------------------
    while (1)
    {
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

        // Debug + sprite update
        drawDebugInfo();
        playerUpdateSprite();

        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_QUIT;
}
