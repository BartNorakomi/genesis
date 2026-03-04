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

// ---------------------------------------------------------
// 2. Local sprite pointers
// ---------------------------------------------------------
static Sprite* holodeckDoorSprite;
static Sprite* holodeckFloorSprite;   // NEW SPRITE

// ---------------------------------------------------------
// 3. Door animation state
// ---------------------------------------------------------
static int holodeckDoorX = 0;
static int holodeckDoorY = 20;

static u8 doorAction = 0;   // 0 = idle, 1 = opening, 2 = closing
static u8 doorFrame  = 0;   // 0–10

static u32 frameCounter = 0;   // SGDK does not provide this

// ---------------------------------------------------------
// 4. Door animation tables (11 frames)
// ---------------------------------------------------------
static const u8 holodeckOpenFrames[11]  = { 0,1,2,3,4,5,6,7,8,9,10 };
static const u8 holodeckCloseFrames[11] = { 10,9,8,7,6,5,4,3,2,1,0 };

// ---------------------------------------------------------
// 5. Apply animation frame
// ---------------------------------------------------------
static void holodeckDoorSetFrame(void)
{
    if (doorAction == 1)
        SPR_setFrame(holodeckDoorSprite, holodeckOpenFrames[doorFrame]);
    else if (doorAction == 2)
        SPR_setFrame(holodeckDoorSprite, holodeckCloseFrames[doorFrame]);
}

// ---------------------------------------------------------
// 6. Animate door (every 3 frames)
// ---------------------------------------------------------
static void holodeckDoorAnimate(void)
{
    if (doorAction == 0) return;

    if ((frameCounter % 3) != 0) return;

    holodeckDoorSetFrame();
    doorFrame++;

    if (doorFrame >= 11)
    {
        doorFrame = 0;
        doorAction = 0;   // finished
    }
}

// ---------------------------------------------------------
// 7. Decide whether to open or close
// ---------------------------------------------------------
static void holodeckDoorCheck(void)
{
    if (doorAction != 0) return;

    // Player left side → open
    if (playerX < 110)
    {
        if (holodeckDoorSprite->frameInd != 10)
        {
            doorAction = 1;
            doorFrame = 0;
        }
        return;
    }

    // Player right side → close
    if (holodeckDoorSprite->frameInd != 0)
    {
        doorAction = 2;
        doorFrame = 0;
    }
}

// ---------------------------------------------------------
// 8. Depth sorting
// ---------------------------------------------------------
static void updateDepth(void)
{
    SPR_setDepth(playerSprite, -playerY);

    // Floor should always be behind everything
//    SPR_setDepth(holodeckFloorSprite, 1000);   // very far back

    // Door sits above floor but behind player
    SPR_setDepth(holodeckDoorSprite, -holodeckDoorY - 80);
}

// ---------------------------------------------------------
// 9. Room logic
// ---------------------------------------------------------
GameState runHoloDeck(void)
{
    drawRoomBackground(ROOM_HOLODECK);
    playMusic(tune_ship);

    SPR_reset();

    // Player
    playerSprite = SPR_addSprite(
        &playerSpriteDef,
        playerX,
        playerY,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );

    // NEW: Floor sprite
    holodeckFloorSprite = SPR_addSprite(
        &holodeckFloorSpriteDef,
        0,      // X
        137,    // Y (adjust as needed)
        TILE_ATTR(PAL3, FALSE, FALSE, FALSE)
    );

    // Door sprite (11 frames)
    holodeckDoorSprite = SPR_addSprite(
        &holodeckDoorSpriteDef,
        holodeckDoorX,
        holodeckDoorY,
        TILE_ATTR(PAL3, FALSE, FALSE, FALSE)
    );

    while (1)
    {
        frameCounter++;

        playerHandleInput();
        updateDepth();

        // Door logic
        holodeckDoorCheck();
        holodeckDoorAnimate();

        // ---- Room transitions ----

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
