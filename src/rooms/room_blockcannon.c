#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_blockcannon.h"
#include "game_state.h"
#include "room_arcade1.h"
#include "player.h"

// ---------------------------------------------------------
// 1. Externs from other modules
// ---------------------------------------------------------
extern const u8* currentColMap;
extern u8 tileContent;

extern void drawRoomBackground(u8 room);
extern void drawDebugInfo(void);

// Shared global tile index from main.c
extern u16 globalTileIndex;

static u16 framecounter2 = 0;

// ---------------------------------------------------------
// 2. Internal Block Cannon states
// ---------------------------------------------------------
typedef enum
{
    BC_STATE_TITLE,
    BC_STATE_GAME
} BlockCannonState;

// ---------------------------------------------------------
// 3. Cannon + projectile + block placeholders
// ---------------------------------------------------------
static Sprite* cannonSprite;
static u8 CannonRow = 2;   // middle row

// MSX dy offsets: 16, 38, 60, 82, 104 (+ one extra row)
static const s16 CannonRowY[6] = {
    16,   // row 0
    38,   // row 1
    60,   // row 2
    82,   // row 3
    104,  // row 4
    126   // row 5
};

typedef struct {
    bool active;
    s16 x, y;
    s16 dx, dy;
    Sprite* spr;
} Projectile;

typedef struct {
    bool active;
    bool exploding;
    u8 explosionFrame;
    s16 x, y;
    Sprite* spr;
} Block;

static Projectile projectile;
static Block blocks[32];

// ---------------------------------------------------------
// 4. EMPTY ROUTINES (stubs)
// ---------------------------------------------------------

void AnimateBlockExplosion(void) {}
void CheckInitiateExplosionEntireColumn(void) {}
void CheckGameOverBlockHitGame(void) {}
void MoveProjectile(void) {}
void CheckProjectileHitsBlock(void) {}
void CheckShootNewProjectile(void) {}
void SetScoreBlockHitGame(void) {}
void PutNewBlocks(void) {}
void MoveBlocks(void) {}

// ---------------------------------------------------------
// 5. MSX timing logic port
// ---------------------------------------------------------
void HandleBlockTiming(void)
{
    u16 joy = playerGetJoy();   // held buttons
    u8 interval = 3;

    // If RIGHT is held → faster block movement
    if (joy & BUTTON_RIGHT)
        interval = 1;

    // Only update blocks every interval frames
    if ((framecounter2 % interval) != 0)
        return;

    PutNewBlocks();
    MoveBlocks();
}

// ---------------------------------------------------------
// 6. MoveCannon (SGDK version of your MSX routine)
// ---------------------------------------------------------
void MoveCannon(void)
{
    u16 joyNew = playerGetJoyNew();

    // UP pressed?
    if (joyNew & BUTTON_UP)
    {
        if (CannonRow > 0)
            CannonRow--;
    }

    // DOWN pressed?
    if (joyNew & BUTTON_DOWN)
    {
        if (CannonRow < 5)
            CannonRow++;
    }

    // Update sprite position
    SPR_setPosition(cannonSprite, 120, CannonRowY[CannonRow]);
}

// ---------------------------------------------------------
// 7. Room logic (Block Cannon minigame)
// ---------------------------------------------------------
GameState runBlockCannon(void)
{
    drawRoomBackground(ROOM_BLOCKCANNON);
    playMusic(tune_ship);

    // Foreground arcade cabinet
    VDP_loadTileSet(arcademachine.tileset, globalTileIndex, DMA);
    VDP_drawImageEx(
        BG_A,
        &arcademachine,
        TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, globalTileIndex),
        0, 0,
        FALSE,
        TRUE
    );
    globalTileIndex += arcademachine.tileset->numTile;

    SPR_reset();
    SPR_update();
    waitMs(120);

    BlockCannonState bcState = BC_STATE_TITLE;

    // Draw game screen
    VDP_drawImageEx(
        BG_B,
        &blockcannoningameexample,
        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX),
        0, 0,
        FALSE,
        TRUE
    );

    PAL_fadeIn(0, 15, blockcannoningameexample.palette->data, 8, FALSE);

    // Create cannon sprite
    cannonSprite = SPR_addSprite(
        &blockCannonCannonSpriteDef,
        120,
        CannonRowY[CannonRow],
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );

    bcState = BC_STATE_GAME;

    while (1)
    {
        playerHandleInput();
        u16 joyNew = playerGetJoyNew();

        switch (bcState)
        {
            case BC_STATE_TITLE:

                if (joyNew & BUTTON_B)
                    return STATE_ARCADE1;

                if (joyNew & BUTTON_A)
                {
                    PAL_fadeOut(0, 15, 8, FALSE);

                    VDP_drawImageEx(
                        BG_B,
                        &blockcannoningameexample,
                        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX),
                        0, 0,
                        FALSE,
                        TRUE
                    );

                    PAL_fadeIn(0, 15, blockcannoningameexample.palette->data, 8, FALSE);

                    bcState = BC_STATE_GAME;
                }
                break;

            case BC_STATE_GAME:
                framecounter2++;

                MoveCannon();
                AnimateBlockExplosion();
                CheckInitiateExplosionEntireColumn();
                CheckGameOverBlockHitGame();
                MoveProjectile();
                CheckProjectileHitsBlock();
                CheckShootNewProjectile();
                SetScoreBlockHitGame();

                HandleBlockTiming();

                if (joyNew & BUTTON_B)
                {
                    PAL_fadeOut(0, 15, 8, FALSE);

                    VDP_drawImageEx(
                        BG_B,
                        &blockcannontitlescreen,
                        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX),
                        0, 0,
                        FALSE,
                        TRUE
                    );

                    PAL_fadeIn(0, 15, blockcannontitlescreen.palette->data, 8, FALSE);

                    bcState = BC_STATE_TITLE;
                }
                break;
        }

        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_ARCADE1;
}
