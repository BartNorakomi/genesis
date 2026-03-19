#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_arcade2.h"
#include "player.h"
#include "game_state.h"
#include "room_arcade1.h"      // for STATE_ARCADE1
#include "room_sleepingquarters.h"

// ---------------------------------------------------------
// 1. Externs from other modules
// ---------------------------------------------------------
extern const u8* currentColMap;
extern u8 tileContent;

extern void drawRoomBackground(u8 room);
extern void drawDebugInfo(void);

// ---------------------------------------------------------
// 2. Local sprite pointers (player + props)
// ---------------------------------------------------------
static Sprite* arcade2TableSprite;
static Sprite* arcade2TableWithEntitySprite;

// ---------------------------------------------------------
// 3. Prop positions (easy to adjust)
// ---------------------------------------------------------
static int tableX  =  70;
static int tableY  = 157;

static int tableEntityX = 180;
static int tableEntityY =  96;

// ---------------------------------------------------------
// 4. Depth sorting
// ---------------------------------------------------------
static void updateDepth(void)
{
    // Props behind player
    SPR_setDepth(arcade2TableSprite, -tableY -60);
//    SPR_setDepth(arcade2TableWithEntitySprite, -tableEntityY -50);

    // Player always sorts by feet
    SPR_setDepth(playerSprite, -playerY);
}

// ---------------------------------------------------------
// 5. Room logic
// ---------------------------------------------------------
GameState runArcade2(void)
{
    drawRoomBackground(ROOM_ARCADE2);
    playMusic(tune_ship);

    SPR_reset();

    // Player sprite
    playerSprite = SPR_addSprite(
        &playerSpriteDef,
        playerX,
        playerY,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );

    // -----------------------------------------------------
    // Props (NEW)
    // -----------------------------------------------------
    arcade2TableSprite = SPR_addSprite(
        &arcade2TableSpriteDef,
        tableX,
        tableY,
        TILE_ATTR(PAL3, FALSE, FALSE, FALSE)
    );

    // arcade2TableWithEntitySprite = SPR_addSprite(
    //     &arcade2TableWithEntitySpriteDef,
    //     tableEntityX,
    //     tableEntityY,
    //     TILE_ATTR(PAL3, FALSE, FALSE, FALSE)
    // );

    // -----------------------------------------------------
    // Main room loop
    // -----------------------------------------------------
    while (1)
    {
        playerHandleInput();
        u16 joyNew = playerGetJoyNew();
        updateDepth();

        // ---- Room transition logic ----

        // Bottom exit → Arcade 1
        if (playerY == 125)
        {
            playerX = 138;   // becomes 182 when entering screen
            playerY = 55;    // becomes 54, triggers entering screen
            return STATE_ARCADE1;
        }

        drawDebugInfo();
        playerUpdateSprite();

        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_QUIT;
}
