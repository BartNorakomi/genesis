#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_reactorchamber.h"
#include "player.h"
#include "game_state.h"
#include "room_biopod.h"
#include "room_arcade1.h"

// ---------------------------------------------------------
// 1. Externs
// ---------------------------------------------------------
extern const u8* currentColMap;
extern u8 tileContent;

extern void drawRoomBackground(u8 room);
extern void drawDebugInfo(void);

// ---------------------------------------------------------
// 2. Local sprite pointers
// ---------------------------------------------------------
static Sprite* reactorSprite;

// ---------------------------------------------------------
// 3. Reactor position (feet position for depth sorting)
// ---------------------------------------------------------
static int reactorX = 66;
static int reactorY = 32;

// ---------------------------------------------------------
// 4. Unified depth sorting
// ---------------------------------------------------------
static void updateDepth(void)
{
    SPR_setDepth(playerSprite,  -playerY);
    SPR_setDepth(reactorSprite, -reactorY - 50);
}

// ---------------------------------------------------------
// 5. Room logic
// ---------------------------------------------------------
GameState runReactorChamber(void)
{
    drawRoomBackground(ROOM_REACTORCHAMBER);
    playMusic(tune_ship);

    SPR_reset();

    // Player sprite
    playerSprite = SPR_addSprite(
        &playerSpriteDef,
        playerX,
        playerY,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );

    // Load reactor palette into PAL3
    PAL_setPalette(PAL3, reactorSpriteDef.palette->data, CPU);

    // Reactor sprite
    reactorSprite = SPR_addSprite(
        &reactorSpriteDef,
        reactorX,
        reactorY,
        TILE_ATTR(PAL3, FALSE, FALSE, FALSE)
    );

    while (1)
    {
        playerHandleInput();
        updateDepth();

        // ---- Room transition logic ----
        if (playerX < EdgeRoomLeft + 1)
        {
            playerX = EnterRoomRight;
            playerY = 0x5A;
            return STATE_BIOPOD;
        }

        if (playerX >= EdgeRoomRight)
        {
            playerX = EnterRoomLeft;
            playerY = 0x5A - 22;
            return STATE_HOLODECK;
        }

        drawDebugInfo();
        playerUpdateSprite();

        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_QUIT;
}
