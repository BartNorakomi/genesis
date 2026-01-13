#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_hangarbay.h"
#include "player.h"
#include "game_state.h"
#include "room_sciencelab.h"
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
static Sprite* drillingMachineSprite;

// ---------------------------------------------------------
// 3. Drilling machine position (feet position for depth sorting)
// ---------------------------------------------------------
static int drillingMachineX = 118;
static int drillingMachineY = 85;

// ---------------------------------------------------------
// 4. Unified depth sorting
// ---------------------------------------------------------
static void updateDepth(void)
{
    SPR_setDepth(playerSprite,  -playerY);
    SPR_setDepth(drillingMachineSprite, - drillingMachineY - 00);
}

// ---------------------------------------------------------
// 5. Room logic
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

    while (1)
    {
        playerHandleInput();
        updateDepth();

        // ---- Room transition logic ----

        // Left exit → Science Lab
        if (playerX < EdgeRoomLeft + 1)
        {
            playerX = EnterRoomRight;
            playerY = 0x5A + 20;
            return STATE_SCIENCELAB;
        }

        // Right exit → Next room (placeholder)
        if (playerX >= EdgeRoomRight)
        {
            playerX = EnterRoomLeft;
            playerY = 0x5A - 20;
            return STATE_TRAININGDECK;
        }

        // Debug + sprite update
        drawDebugInfo();
        playerUpdateSprite();

        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_QUIT;
}
