#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_sciencelab.h"
#include "player.h"
#include "game_state.h"
#include "room_holodeck.h"     // for STATE_HOLODECK
#include "room_arcade1.h"      // placeholder for next room

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
static Sprite* scienceLabHelixSprite;

// ---------------------------------------------------------
// 3. Helix position (feet position for depth sorting)
// ---------------------------------------------------------
static int scienceLabHelixX = 212;   // adjust as needed
static int scienceLabHelixY = 49;    // adjust as needed

// ---------------------------------------------------------
// 4. Unified depth sorting
// ---------------------------------------------------------
static void updateDepth(void)
{
    SPR_setDepth(playerSprite, -playerY);
    SPR_setDepth(scienceLabHelixSprite, -scienceLabHelixY);
}

// ---------------------------------------------------------
// 5. Room logic
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

    while (1)
    {
        playerHandleInput();
        updateDepth();

        // ---- Room transition logic ----

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

        // Debug + sprite update
        drawDebugInfo();
        playerUpdateSprite();

        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_QUIT;
}
