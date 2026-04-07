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
    BC_STATE_GAME,
    BC_STATE_GAMEOVER
} BlockCannonState;

BlockCannonState bcState = BC_STATE_TITLE;

// ---------------------------------------------------------
// 3. Cannon + projectile + block placeholders
// ---------------------------------------------------------
static Sprite* cannonSprite;
static u8 CannonRow = 2;   // middle row

// MSX dy offsets: 16, 38, 60, 82, 104
static const s16 CannonRowY[5] = {
    16,   // row 0
    38,   // row 1
    60,   // row 2
    82,   // row 3
    104,  // row 4
};

static bool gameJustStarted = FALSE;

typedef enum {
    BlockColorGreen  = 1,
    BlockColorYellow = 2,
    BlockColorRed    = 3
} BlockColor;


static const u8* BlocksColumnsTablePointer;

// ---------------------------------------------------------
// BlocksColumnsTable (converted from MSX db statements)
// Each row has 5 columns
// ---------------------------------------------------------
static const u8 BlocksColumnsTable[][5] = {
    // level 1
    {0,0,0,0,0},
    {0,0,0,0,0},

    {0,0,1,1,1},
    {1,0,1,0,1},
    {0,1,1,1,0},
    {0,1,0,1,0},

    {1,1,0,0,1},
    {0,0,1,1,1},
    {1,1,1,0,0},
    {1,0,0,0,1},

    {0,1,1,1,0},
    {1,0,0,0,1},
    {1,1,0,1,1},
    {0,1,1,0,0},

    {1,0,1,0,1},
    {0,0,1,1,1},
    {1,1,0,1,0},
    {1,0,0,0,1},

    {0,1,1,1,0},
    {0,0,1,1,1},
    {1,0,0,1,1},
    {1,1,0,0,0},

    {1,1,1,0,0},
    {0,0,1,1,1},
    {1,0,0,1,0},
    {0,1,0,1,1},

    {0,0,1,0,1},
    {1,0,0,1,1},
    {0,1,0,1,1},
    {1,0,1,0,0},

    {0,1,1,1,0},
    {1,0,0,1,1},
    {0,0,1,0,1},
    {0,1,0,1,0},

    {1,1,0,0,0},
    {0,0,1,1,1},
    {1,1,0,1,0},
    {0,1,0,0,1},

    {1,0,1,0,1},
    {0,1,0,1,0},
    {1,0,1,1,0},
    {1,0,0,1,1},

    {0,1,1,0,0},
    {1,0,0,1,1},
    {0,1,1,0,1},
    {0,0,0,1,1},

    {1,1,0,0,0},
    {0,0,1,1,1},
    {0,1,1,0,0},
    {1,0,0,1,1},

    {0,1,1,0,1},
    {1,0,0,1,1},
    {1,0,1,0,1},
    {1,1,1,1,1},

// level 2
{0,0,0,0,0},
{0,0,0,0,0},
{0,0,0,0,0},
{0,0,0,0,0},

{0,1,1,1,0},
{1,0,0,1,1},
{0,1,1,0,1},
{1,0,0,0,1},

{0,1,1,1,0},
{1,0,0,1,1},
{0,1,0,1,0},
{1,0,1,0,0},

{0,1,1,0,1},
{0,0,2,1,1},
{1,1,0,1,0},
{0,0,0,1,1},

{0,1,1,1,0},
{1,0,0,1,1},
{0,1,1,0,1},
{1,0,0,0,1},

{0,1,1,1,0},
{1,0,0,1,1},
{0,1,0,1,0},
{1,0,1,0,0},

{0,1,1,0,1},
{0,0,2,1,1},
{1,1,0,1,0},
{0,0,0,1,1},

{0,1,1,1,0},
{0,0,1,1,1},
{1,0,0,1,1},
{1,1,0,0,0},

{1,1,1,0,0},
{0,0,1,1,1},
{1,0,0,1,0},
{0,1,0,1,1},

{0,0,1,0,1},
{1,0,0,1,1},
{0,1,0,1,1},
{1,0,1,0,0},

{0,1,1,1,0},
{1,0,0,1,1},
{0,0,0,0,1},
{0,1,0,1,0},

{1,1,1,0,0},
{0,0,0,1,1},
{1,1,0,1,0},
{0,1,0,0,0},

{1,0,1,0,1},
{0,1,0,1,0},
{1,0,1,1,0},
{1,0,0,1,0},

{0,1,1,0,0},
{1,0,0,1,1},
{0,1,1,0,1},
{0,0,0,1,0},

{1,1,0,0,0},
{0,0,1,1,1},
{0,1,1,0,0},
{1,0,0,1,1},

{0,1,1,0,1},
{1,0,0,1,1},
{1,0,1,0,1},
{1,1,1,1,1},

// level 3
{0,0,0,0,0},
{0,0,0,0,0},
{0,0,0,0,0},
{0,0,0,0,0},

{0,2,1,1,0},
{1,0,0,1,1},
{0,1,1,0,1},
{1,0,0,0,1},

{0,2,1,1,0},
{1,0,0,1,1},
{0,1,0,1,0},
{2,0,1,0,0},

{0,2,1,0,1},
{0,0,2,1,1},
{1,1,0,1,0},
{0,0,0,1,1},

{0,2,2,1,0},
{1,0,0,1,1},
{0,1,1,0,1},
{1,0,0,0,1},

{0,1,1,2,0},
{1,0,0,1,1},
{0,1,0,2,0},
{1,0,1,0,0},

{0,1,0,0,1},
{0,0,2,1,1},
{1,0,0,1,0},
{0,1,0,1,1},

{0,0,1,1,0},
{0,1,1,0,1},
{1,0,0,1,1},
{1,1,0,0,0},

{1,1,1,0,0},
{0,0,1,1,1},
{1,0,0,1,0},
{0,1,0,1,1},

{0,0,1,0,1},
{1,0,0,1,1},
{0,1,0,1,1},
{1,0,1,0,0},

{0,1,1,1,0},
{1,0,0,1,1},
{0,0,0,0,1},
{0,1,0,1,0},

{1,1,1,0,0},
{0,0,0,1,1},
{1,1,0,1,0},
{0,1,0,0,0},

{1,0,1,0,1},
{0,1,0,1,0},
{1,0,1,1,0},
{1,0,0,1,0},

{0,1,1,0,0},
{1,0,0,1,1},
{0,1,1,0,1},
{0,0,0,1,0},

{1,1,0,0,0},
{0,0,1,1,1},
{0,2,2,0,0},
{1,0,0,1,1},

{0,1,1,0,1},
{1,0,0,1,1},
{1,0,1,0,1},
{1,1,1,1,1},


// level 4
{0,0,0,0,0},
{0,0,0,0,0},
{0,0,0,0,0},
{0,0,0,0,0},

{1},   // this is the standalone "db 1"

{0,1,1,1,0},
{0,0,1,1,1},
{1,0,0,1,1},
{0,1,0,0,0},

{0,1,1,0,0},
{0,0,0,1,1},
{1,0,0,1,0},
{0,1,0,0,1},

{0,0,1,0,1},
{1,0,0,1,1},
{0,1,0,0,1},
{1,0,0,0,0},

{0,1,1,1,0},
{0,0,0,1,1},
{0,0,0,0,1},
{0,1,0,1,0},

{1,0,1,0,0},
{0,0,0,0,1},
{1,1,0,1,0},
{0,1,0,0,0},

{1,0,1,0,1},
{0,0,0,1,0},
{1,0,1,1,0},
{1,0,0,1,0},

{0,1,1,1,0},
{1,0,0,1,1},
{0,0,0,0,1},
{0,1,0,1,0},

{1,1,1,0,0},
{0,0,0,1,1},
{1,1,0,1,0},
{0,1,0,0,0},

{1,0,1,0,1},
{0,1,0,1,0},
{1,0,1,1,0},
{1,0,0,1,0},

{0,1,1,1,0},
{1,0,0,1,1},
{0,0,2,0,1},
{0,2,0,1,0},

{1,1,0,0,0},
{0,0,1,1,1},
{1,1,0,1,0},
{0,2,0,0,1},

{1,0,1,0,1},
{0,1,0,1,0},
{1,0,2,1,0},
{1,0,0,1,1},

{0,1,1,0,0},
{1,0,0,1,1},
{0,1,2,0,1},
{0,0,0,1,1},

{1,1,0,0,0},
{0,0,1,1,1},
{0,2,2,0,0},
{1,0,0,1,1},

{0,1,1,0,1},
{1,0,0,2,2},
{2,0,2,0,1},
{1,1,1,1},

// level 5
{0,0,0,0,0},
{0,0,0,0,0},
{0,0,0,0,0},
{0,0,0,0,0},

{1,2,1,0,0},
{0,0,1,0,1},
{1,0,0,1,0},
{0,1,0,1,0},

{0,0,1,0,2},
{1,0,0,2,2},
{0,2,0,1,1},
{2,0,1,0,0},

{0,2,1,1,0},
{2,0,0,1,1},
{0,0,0,0,1},
{0,2,0,2,0},

{1,2,1,0,0},
{0,0,0,2,1},
{1,1,0,1,0},
{0,2,0,0,0},

{1,0,1,0,1},
{0,1,0,1,0},
{1,0,0,0,0},
{1,0,0,1,0},

{0,1,1,0,0},
{1,0,0,0,1},
{0,1,0,1,1},
{0,1,1,0,0},

{1,0,0,0,1},
{0,0,1,1,0},
{1,1,0,1,0},
{1,0,0,0,1},

{0,3,1,1,0},
{0,0,1,1,1},
{2,0,0,1,1},
{1,1,0,0,0},

{1,3,1,0,0},
{0,0,1,1,1},
{1,0,0,1,0},
{0,1,0,2,1},

{0,0,1,0,1},
{1,0,0,1,1},
{0,1,0,1,1},
{1,0,3,0,0},

{0,1,1,1,0},
{1,0,0,1,1},
{0,0,2,0,1},
{0,3,0,1,0},

{1,1,0,0,0},
{0,0,1,1,1},
{1,1,0,1,0},
{0,2,0,0,1},

{1,0,1,0,1},
{0,1,0,1,0},
{1,0,2,1,0},
{1,0,0,1,1},

{0,1,1,0,0},
{1,0,0,1,1},
{0,1,2,0,1},
{0,0,0,1,1},

{1,1,0,0,0},
{0,0,1,1,1},
{0,1,2,0,0},
{1,0,0,1,1},

{0,1,1,0,1},
{1,0,0,2,1},
{1,0,1,0,1},
{1,1,1,1,1},

// level 6
{0,0,0,0,0},
{0,0,0,0,0},
{0,0,0,0,0},
{0,0,0,0,0},

{0,2,1,1,0},
{3,3,0,3,1},
{0,1,0,1,0},
{2,0,1,0,0},

{0,2,1,0,1},
{0,0,2,1,1},
{1,1,0,1,0},
{0,0,0,1,1},

{0,2,1,1,0},
{1,0,0,1,1},
{0,1,3,0,1},
{1,0,0,0,1},

{0,1,1,2,0},
{1,0,0,1,1},
{0,1,0,2,0},
{1,0,1,0,0},

{0,1,0,0,1},
{0,0,2,1,1},
{1,0,0,1,0},
{0,1,0,1,1},

{0,0,1,3,0},
{0,1,1,0,1},
{2,0,0,1,1},
{1,1,0,0,0},

{3,1,1,0,0},
{0,0,1,1,1},
{1,0,0,1,0},
{0,1,0,1,1},

{0,0,1,0,1},
{1,0,0,2,1},
{0,1,0,1,1},
{1,0,1,0,0},

{0,2,2,3,0},
{1,0,0,2,1},
{0,0,0,0,1},
{0,1,0,2,0},

{1,1,1,1,1},
{0,0,0,0,0},
{1,1,0,1,0},
{0,2,0,0,0},

{1,0,1,0,1},
{0,1,0,1,0},
{1,0,1,1,0},
{1,0,0,1,0},

{0,2,2,0,0},
{2,0,0,2,2},
{0,2,2,0,2},
{0,0,0,2,0},

{2,2,0,0,0},
{0,0,1,1,1},
{0,1,1,0,0},
{2,0,0,2,2},

{0,1,1,0,2},
{2,0,0,1,2},
{2,0,2,0,2},
{1,1,1,1,1},

// level 7
{0,0,0,0,0},
{0,0,0,0,0},
{0,0,0,0,0},
{0,0,0,0,0},

{0,0,2,2,2},
{2,0,2,0,2},
{0,2,2,2,0},
{0,2,0,2,0},

{2,2,0,0,2},
{0,0,2,2,2},
{2,2,2,0,0},
{2,0,0,0,2},

{1,1,1,1,1},
{0,0,0,0,0},
{2,2,0,2,2},
{0,2,2,0,0},

{2,0,2,0,2},
{0,0,2,2,2},
{2,2,0,2,0},
{2,0,0,0,2},

{0,2,2,2,0},
{0,0,2,2,2},
{2,0,0,2,2},
{2,2,0,0,0},

{2,2,2,0,0},
{0,0,2,2,2},
{2,0,0,2,0},
{0,2,0,2,2},

{1,1,1,1,1},
{0,0,0,0,0},
{0,0,0,0,0},
{2,0,2,0,0},

{0,2,2,2,0},
{2,0,0,2,2},
{0,0,2,0,2},
{0,2,0,2,0},

{2,2,0,0,0},
{0,0,2,2,2},
{2,2,0,2,0},
{0,2,0,0,2},

{2,0,2,0,2},
{0,2,0,2,0},
{2,0,2,2,0},
{2,0,0,2,2},

{2,0,0,0,0},
{1,1,1,1,1},
{0,0,0,0,0},
{0,0,0,0,0},

{2,2,0,0,0},
{0,0,2,2,2},
{0,2,2,0,0},
{2,0,0,2,2},

{0,2,2,0,2},
{2,0,0,2,2},
{2,0,2,0,2},
{1,1,1,1,1},

// level 8
{0,0,0,0,0},
{0,0,0,0,0},
{0,0,0,0,0},
{0,0,0,0,0},

{0,3,1,1,0},
{0,0,1,1,1},
{1,0,0,1,1},
{0,1,0,0,0},

{0,1,3,0,0},
{0,0,0,1,1},
{1,0,0,1,0},
{0,1,0,0,1},

{0,0,3,0,1},
{1,0,0,1,1},
{0,1,0,0,1},
{1,0,0,0,0},

{0,3,1,1,0},
{0,0,0,1,1},
{1,0,0,0,1},
{0,1,0,1,0},

{1,0,3,0,0},
{0,0,0,0,1},
{1,1,0,1,0},
{0,1,0,0,0},

{1,0,3,0,1},
{0,0,0,1,0},
{1,0,1,1,0},
{1,0,0,1,1},

{0,3,1,1,0},
{1,0,0,1,1},
{0,0,0,0,1},
{0,1,0,1,0},

{1,1,3,0,0},
{0,0,0,1,1},
{1,1,0,1,0},
{0,1,0,0,0},

{3,0,1,0,1},
{0,1,0,1,0},
{1,0,1,1,0},
{1,0,0,1,0},

{0,1,1,1,0},
{3,0,0,1,1},
{0,0,2,0,1},
{0,2,0,1,0},

{1,1,0,0,0},
{0,0,2,1,1},
{1,1,0,1,0},
{0,2,0,0,1},

{1,0,1,0,1},
{0,1,0,1,0},
{1,0,2,1,0},
{1,0,0,2,1},

{0,1,1,0,0},
{1,0,0,1,1},
{0,1,2,0,1},
{0,0,0,1,2},

{1,1,0,0,0},
{0,0,1,1,3},
{0,2,2,0,0},
{1,0,0,1,1},

{0,1,1,0,3},
{1,0,0,2,2},
{2,0,2,0,1},
{1,1,1,1,1},

{0,0,0,0,0},
{2,0,0,2,2},
{2,0,2,0,2},
{1,1,1,1,1},

// level 9
{0,0,0,0,0},
{0,0,0,0,0},
{0,0,0,0,0},
{0,0,0,0,0},

{0,3,1,0,1},
{0,0,2,1,1},
{0,0,0,0,0},
{1,1,1,1,1},

{0,1,1,1,0},
{0,0,1,3,1},
{0,0,0,0,0},
{1,1,1,1,1},

{1,1,3,0,0},
{0,0,1,1,1},
{0,0,0,0,0},
{1,1,1,1,1},

{0,0,1,0,1},
{1,0,0,3,1},
{0,0,0,0,0},
{1,1,1,1,1},

{0,1,1,1,0},
{1,0,0,1,3},
{0,0,0,0,0},
{1,1,1,1,1},

{1,1,1,0,0},
{0,0,0,3,1},
{1,1,0,1,0},
{0,1,1,0,0},

{0,0,0,0,1},
{0,0,0,0,0},
{1,1,3,1,1},
{1,0,1,1,0},
{1,0,0,1,0},

{0,1,2,0,0},
{1,0,0,1,1},
{0,0,0,0,0},
{1,1,1,1,1},

{1,1,0,0,0},
{0,0,3,1,1},
{0,1,1,0,0},
{1,0,0,1,3},

{0,0,0,0,0},
{1,1,1,1,1},
{0,1,1,0,1},
{1,0,0,0,1},

{0,2,1,1,0},
{1,0,0,3,1},
{0,0,0,0,0},
{1,1,1,1,1},

{0,2,1,0,1},
{0,0,2,3,1},
{1,1,0,1,0},
{0,0,0,1,1},

{0,2,2,1,0},
{1,0,0,3,1},
{0,0,0,0,0},
{1,1,1,1,1},

{0,1,1,2,0},
{1,0,0,1,1},
{0,1,0,2,0},
{1,0,2,0,0},

{0,0,0,0,0},
{2,0,0,2,2},
{2,0,3,0,2},
{1,1,1,1,1},

// level 10
{0,0,0,0,0},
{0,0,0,0,0},
{0,0,0,0,0},
{0,0,0,0,0},

{1,1,0,0,2},
{0,0,1,1,1},
{1,2,1,0,0},
{1,0,0,0,3},

{0,1,1,1,0},
{2,0,0,0,1},
{1,1,0,2,1},
{0,1,3,0,0},

{1,0,1,0,1},
{0,0,1,1,1},
{1,2,0,1,0},
{1,0,0,0,1},

{0,3,3,3,0},
{0,0,1,1,1},
{1,0,0,2,1},
{1,1,0,0,0},

{2,2,1,0,0},
{0,0,2,2,2},
{1,0,0,3,0},
{0,1,0,1,1},

{0,0,1,0,1},
{1,0,0,1,1},
{0,1,0,2,1},
{1,0,3,0,0},

{0,1,1,1,0},
{1,0,0,2,1},
{0,0,1,0,1},
{0,2,0,1,0},

{1,3,0,0,0},
{0,0,1,2,1},
{1,1,0,1,0},
{0,1,0,0,1},

{1,0,2,0,1},
{0,1,0,2,0},
{2,0,1,1,0},
{1,0,0,1,2},

{0,3,1,0,0},
{1,0,0,1,1},
{0,1,1,0,1},
{0,0,0,1,1},

{1,1,0,0,0},
{0,0,2,1,1},
{0,1,1,0,0},
{1,0,0,1,3},

{0,2,1,0,1},
{1,0,0,1,1},
{1,0,3,0,2},
{1,1,1,1,1},

// level 11
{0,0,0,0,0},
{0,0,0,0,0},
{0,0,0,0,0},
{0,0,0,0,0},

{0,2,1,1,0},
{3,3,0,3,1},
{0,1,0,1,0},
{2,0,1,0,0},

{0,3,1,0,1},
{0,0,2,1,1},
{1,1,0,1,0},
{0,0,0,1,1},

{0,3,1,1,0},
{1,0,0,1,1},
{0,1,3,0,1},
{1,0,0,0,1},

{0,1,1,2,0},
{1,0,0,1,1},
{0,1,0,2,0},
{1,0,3,0,0},

{0,1,0,0,1},
{0,0,2,1,1},
{1,0,0,3,0},
{0,1,0,1,1},

{0,0,1,3,0},
{0,1,1,0,1},
{2,0,0,1,1},
{1,1,0,0,0},

{3,1,1,0,0},
{0,0,1,1,1},
{1,0,0,1,0},
{0,1,0,3,1},

{0,0,1,0,1},
{1,0,0,2,1},
{0,1,0,1,1},
{1,0,1,0,0},

{0,2,2,3,0},
{1,0,0,2,1},
{0,0,0,0,1},
{0,2,0,2,0},

{1,1,1,1,1},
{0,0,0,0,0},
{1,3,0,1,0},
{0,2,0,0,0},

{1,1,1,1,1},

// level 12
{0,0,0,0,0},
{0,0,0,0,0},
{0,0,0,0,0},
{0,0,0,0,0},

{0,2,1,1,0},
{3,3,0,3,1},
{0,1,0,1,0},
{2,0,1,0,0},

{0,3,1,0,1},
{0,0,2,1,1},
{1,1,0,3,0},
{0,0,0,1,1},

{0,3,1,1,0},
{1,0,0,1,1},
{0,1,3,0,1},
{1,0,0,0,1},

{0,1,3,2,0},
{1,0,0,1,1},
{0,1,0,2,0},
{1,0,3,0,0},

{0,1,0,0,1},
{0,0,2,1,1},
{1,0,0,3,0},
{0,3,0,1,1},

{0,0,1,3,0},
{0,1,1,0,1},
{2,0,0,1,1},
{1,3,0,0,0},

{3,1,1,0,0},
{0,0,1,1,1},
{1,0,0,1,0},
{0,1,0,3,1},

{0,0,1,0,1},
{1,0,0,2,1},
{0,1,0,1,1},
{1,0,1,0,0},

{0,2,2,3,0},
{1,0,0,2,3},
{0,0,0,0,1},
{0,2,0,2,0},

{1,1,1,1,1},
{0,0,0,0,0},
{1,3,0,1,0},
{0,2,0,0,0},

{1,1,1,1,1},

// level 13
{0,0,0,0,0},
{0,0,0,0,0},
{0,0,0,0,0},
{0,0,0,0,0},

{0,2,2,1,0},
{3,3,0,3,1},
{0,1,0,1,0},
{2,0,1,0,0},

{0,3,1,0,1},
{0,0,2,1,1},
{1,2,0,3,0},
{0,0,0,1,1},

{0,3,1,1,0},
{1,0,0,1,1},
{0,1,3,0,2},
{1,0,0,0,1},

{0,1,3,2,0},
{1,0,0,1,1},
{0,1,0,2,0},
{2,0,3,0,0},

{0,1,0,0,1},
{0,0,2,1,1},
{1,0,0,3,0},
{0,3,0,1,1},

{0,0,1,3,0},
{0,1,1,0,1},
{2,0,0,1,1},
{1,3,0,0,0},

{3,1,1,0,0},
{0,0,1,2,1},
{1,0,0,1,0},
{0,1,0,3,1},

{0,0,1,0,1},
{1,0,0,2,1},
{0,1,0,1,1},
{1,0,1,0,0},

{0,2,2,3,0},
{1,0,0,2,3},
{0,0,0,0,1},
{0,2,0,2,0},

{1,1,1,1,1},
{0,0,0,0,0},
{1,3,0,2,0},
{0,2,0,0,0},

{1,1,1,1,1},

// level 14
{0,0,0,0,0},
{0,0,0,0,0},
{0,0,0,0,0},
{0,0,0,0,0},

{0,2,2,1,0},
{3,3,0,3,1},
{0,1,0,1,0},
{2,0,1,0,0},

{0,3,1,0,1},
{0,0,2,1,1},
{1,2,0,3,0},
{0,0,0,2,1},

{0,3,1,2,0},
{1,0,0,1,1},
{0,1,3,0,2},
{1,0,0,0,1},

{0,1,3,2,0},
{1,0,0,1,1},
{0,2,0,2,0},
{2,0,3,0,0},

{0,2,0,0,1},
{0,0,2,1,1},
{1,0,0,3,0},
{0,3,0,1,1},

{0,0,1,3,0},
{0,1,1,0,1},
{2,0,0,1,1},
{1,3,0,0,0},

{3,1,1,0,0},
{0,0,1,2,1},
{1,0,0,1,0},
{0,1,0,3,2},

{0,0,1,0,1},
{1,0,0,2,1},
{0,1,0,1,1},
{1,0,2,0,0},

{0,2,2,3,0},
{1,0,0,2,3},
{0,0,0,0,1},
{0,2,0,2,0},

{1,1,1,1,1},
{0,0,0,0,0},
{2,3,0,2,0},
{0,2,0,0,0},

{1,1,1,1,1},

// level 15
{0,0,0,0,0},
{0,0,0,0,0},
{0,0,0,0,0},
{0,0,0,0,0},

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
    u8 explosionFrame;   // 1–13
    u8 baseColor;        // 1=green, 2=yellow, 3=red
    u8 currentFrame;     // actual sprite frame index
    s16 x, y;
    Sprite* spr;
} Block;


static Projectile projectile;
static Block blocks[32];

// Shoot flags (MSX equivalents)
static bool requestShoot = FALSE;
static bool animateShoot = FALSE;
static u8 cannonAnimTimer = 0;

// MSX variables we must add
static u8 PutNewBlocksCounter = 1;
static u16 ScoreBlockHitGame = 0;

// ---------------------------------------------------------
// 4. ResetVariablesBlockHitGame (MSX logic port)
// ---------------------------------------------------------
void ResetVariablesBlockHitGame(void)
{
    SPR_reset();
    SPR_update();

    cannonSprite = NULL;
    projectile.spr = NULL;

    for (int i = 0; i < 31; i++)
        blocks[i].spr = NULL;

    CannonRow = 2;
    PutNewBlocksCounter = 1;
    requestShoot = FALSE;
    animateShoot = FALSE;
    cannonAnimTimer = 0;

    // MSX: ld hl,BlocksColumnsTable-5
    BlocksColumnsTablePointer = &BlocksColumnsTable[0][0] - 5;

    ScoreBlockHitGame = 0;

    // Reset all block data
    for (int i = 0; i < 31; i++)
    {
        blocks[i].active = FALSE;
        blocks[i].exploding = FALSE;
        blocks[i].explosionFrame = 0;
        blocks[i].baseColor = 0;
        blocks[i].currentFrame = 0;
        blocks[i].x = 0;
        blocks[i].y = 0;
        blocks[i].spr = NULL;   // after SPR_reset
    }

    cannonSprite = SPR_addSprite(
        &blockCannonCannonSpriteDef,
        8,
        CannonRowY[CannonRow] - 2,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );

    projectile.spr = SPR_addSprite(
        &blockCannonBlockSpriteDef,
        -32, -32,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );

    projectile.active = FALSE;
    projectile.x = 1;
    projectile.y = 213;
    projectile.dx = 0;
    projectile.dy = 0;

    if (projectile.spr)
        SPR_setPosition(projectile.spr, -32, -32);

}
   
void SetScoreBlockHitGame(void)
{
    // 1. Determine mask based on RIGHT being held
    u16 joy = playerGetJoy();   // held buttons
    u8 mask = 31;

    if (joy & BUTTON_RIGHT)
        mask = 7;

    // 2. Throttle scoring using framecounter2
    if ((framecounter2 & mask) != 0)
        return;

    // 3. Increment score
    ScoreBlockHitGame++;

    // 4. Draw score on screen
    char buf[6];
    sprintf(buf, "%u", ScoreBlockHitGame);

    // Clear old score area (tile coords: 19,1 width 6 tiles)
    VDP_clearTextArea(19, 1, 6, 1);

    // Draw new score
    VDP_drawText(buf, 19, 1);
}


static void ColorBlock(u8 color, Block* blk)
{
    blk->baseColor = color;

    switch (color)
    {
        case BlockColorGreen:
            blk->currentFrame = 0;
            break;

        case BlockColorYellow:
            blk->currentFrame = 14;
            break;

        case BlockColorRed:
            blk->currentFrame = 28;
            break;
    }

    SPR_setFrame(blk->spr, blk->currentFrame);
}

static s16 findFreeBlockIndex(void)
{
    for (s16 i = 0; i < 31; i++)
    {
        if (!blocks[i].active)
            return i;
    }
    return -1;
}

void PutNewBlocks(void)
{
    // 1. Timing
    PutNewBlocksCounter--;
    if (PutNewBlocksCounter != 0)
        return;

    PutNewBlocksCounter = 26;

    // 2. Advance table pointer (ix += 5)
    BlocksColumnsTablePointer += 5;

    // char buf[8];
    // sprintf(buf, "%3d", BlocksColumnsTablePointer);
    // VDP_drawTextBG(BG_A, buf, 25, 27);

    // 3. Row loop: c = 19, +22 each row, stop at 107+22
    u8 c = 19;                       // first block Y
    const u8* p = BlocksColumnsTablePointer;

    while (1)
    {
        u8 color = *p;               // 0=no block, 1/2/3 = block colors

        if (color != 0)
        {
            // Find free block entry
            s16 idx = findFreeBlockIndex();
            if (idx < 0)
                return;              // no free sprite → bail out

            blocks[idx].active = TRUE;
            blocks[idx].exploding = FALSE;
            blocks[idx].explosionFrame = 0;

            blocks[idx].x = 250;     // SetXBlockTo250
            blocks[idx].y = c;       // SetYBlock

            if (blocks[idx].spr == NULL)
            {
                blocks[idx].spr = SPR_addSprite(
                    &blockCannonBlockSpriteDef,
                    blocks[idx].x,
                    blocks[idx].y,
                    TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
                );
            }
            else
            {
                SPR_setPosition(blocks[idx].spr, blocks[idx].x, blocks[idx].y);
            }

            // SetColorBlock (1=green, 2=yellow, 3=red)
            ColorBlock(color, &blocks[idx]);
        }

        // Next row
        c += 22;
        if (c >= 129) return;

        p++;    // inc ix
    }
}


void MoveBlocks(void)
{
    for (int i = 0; i < 31; i++)
    {
        if (!blocks[i].active)
            continue;

        // Move block left by 1 pixel
        blocks[i].x--;

        // If block reaches X == 1 → deactivate (MSX behavior)
        if (blocks[i].x <= 1)
        {
            blocks[i].active = FALSE;
            SPR_setPosition(blocks[i].spr, -32, -32);
            continue;
        }

        // Update sprite position
        SPR_setPosition(blocks[i].spr, blocks[i].x, blocks[i].y);

        // MSX threshold: 249 - 12 = 237
        if (blocks[i].x < 237)
            continue;
    }
}


// ---------------------------------------------------------
// 6. MSX timing logic port
// ---------------------------------------------------------
void HandleBlockTiming(void)
{
    u16 joy = playerGetJoy();   // held buttons
    u8 interval = 3;

    if (joy & BUTTON_RIGHT)
        interval = 1;

    if ((framecounter2 % interval) != 0)
        return;

    PutNewBlocks();
    MoveBlocks();
}

// ---------------------------------------------------------
// 7. MoveCannon
// ---------------------------------------------------------
void MoveCannon(void)
{
    u16 joyNew = playerGetJoyNew();

    if (joyNew & BUTTON_UP)
    {
        if (CannonRow > 0)
            CannonRow--;
    }

    if (joyNew & BUTTON_DOWN)
    {
        if (CannonRow < 4)
            CannonRow++;
    }

    SPR_setPosition(cannonSprite, 8, CannonRowY[CannonRow] - 2);
}

// ---------------------------------------------------------
// 8. HandleAnimateShootCannon
// ---------------------------------------------------------
void HandleAnimateShootCannon(void)
{
    if (!animateShoot)
        return;

    if (cannonAnimTimer == 0)
        cannonAnimTimer = 1;
    else
        cannonAnimTimer++;

    if (cannonAnimTimer < 4)
    {
        SPR_setFrame(cannonSprite, 1);
    }
    else if (cannonAnimTimer < 6)
    {
        SPR_setFrame(cannonSprite, 2);
    }
    else if (cannonAnimTimer < 8)
    {
        SPR_setFrame(cannonSprite, 1);
    }
    else
    {
        SPR_setFrame(cannonSprite, 0);
        cannonAnimTimer = 0;
        animateShoot = FALSE;
    }
}

// ---------------------------------------------------------
// 9. CheckShootNewProjectile
// ---------------------------------------------------------
void CheckShootNewProjectile(void)
{
    u16 joyNew = playerGetJoyNew();

    if (!requestShoot)
    {
        if (joyNew & BUTTON_A)
            requestShoot = TRUE;
        else
            return;
    }

    if (projectile.active)
        return;

    animateShoot = TRUE;
    requestShoot = FALSE;

    projectile.active = TRUE;
    projectile.x = 47;
    projectile.dx = 20;
    projectile.dy = 0;

    switch (CannonRow)
    {
        case 0: projectile.y = 19; break;
        case 1: projectile.y = 19 + 22; break;
        case 2: projectile.y = 19 + 44; break;
        case 3: projectile.y = 19 + 66; break;
        case 4: projectile.y = 19 + 88; break;
    }

    SPR_setPosition(projectile.spr, projectile.x, projectile.y);
}

// ---------------------------------------------------------
// 10. MoveProjectile
// ---------------------------------------------------------
void MoveProjectile(void)
{
    if (!projectile.active)
        return;

    projectile.x += 20;

    if (projectile.x >= 240)
    {
        projectile.active = FALSE;
        projectile.x = 1;
        projectile.y = 213;

        SPR_setPosition(projectile.spr, -32, -32);
        return;
    }

    SPR_setPosition(projectile.spr, projectile.x, projectile.y);
}


void CheckProjectileHitsBlock(void)
{
    if (!projectile.active)
        return;

    s16 projX = projectile.x;
    s16 projY = projectile.y;

    // MSX: if projectile.x == 1 → not in play
    if (projX <= 1)
        return;

    // Loop through all blocks (0–30)
    for (int i = 0; i < 31; i++)
    {
        if (!blocks[i].active)
            continue;

        // Compare Y first (MSX: exact match)
        if (blocks[i].y != projY)
            continue;

        // Compare X with MSX offset: projectile.x + 26 >= block.x
        if (projX + 26 < blocks[i].x)
            continue;

        // Check if block is already exploding
        if (blocks[i].exploding)
            continue;

        // --- HIT DETECTED ---

        // Remove projectile (MSX: x=1, y=213)
        projectile.active = FALSE;
        projectile.x = 1;
        projectile.y = 213;
        SPR_setPosition(projectile.spr, -32, -32);

        // Find free block slot (MSX loop2)
        s16 freeIdx = findFreeBlockIndex();
        if (freeIdx < 0)
            return;

        // Add new block in front of the hit block
        blocks[freeIdx].active = TRUE;
        blocks[freeIdx].exploding = FALSE;
        blocks[freeIdx].explosionFrame = 0;

        // MSX: newX = hitBlockX - 26
        blocks[freeIdx].x = blocks[i].x - 26;
        blocks[freeIdx].y = projY;

        if (blocks[freeIdx].spr == NULL)
        {
            blocks[freeIdx].spr = SPR_addSprite(
                &blockCannonBlockSpriteDef,
                blocks[freeIdx].x,
                blocks[freeIdx].y,
                TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
            );
        }
        else
        {
            SPR_setPosition(blocks[freeIdx].spr, blocks[freeIdx].x, blocks[freeIdx].y);
        }

        // MSX: new block color = 1 (green)
        ColorBlock(BlockColorGreen, &blocks[freeIdx]);


        return;
    }
}

void CheckInitiateExplosionEntireColumn(void)
{
    // 1. Find lowest X among all normal blocks
    s16 lowestX = 255;

    for (int i = 0; i < 31; i++)
    {
        if (!blocks[i].active)
            continue;

        if (blocks[i].exploding)
            continue;

        // Only full blocks (frame 0 of their color)
        if (blocks[i].explosionFrame != 0)
            continue;

        if (blocks[i].x < lowestX)
            lowestX = blocks[i].x;
    }

    if (lowestX == 255)
        return;

    // 2. Count blocks at this X
    u8 count = 0;

    for (int i = 0; i < 31; i++)
    {
        if (!blocks[i].active)
            continue;

        if (blocks[i].x != lowestX)
            continue;

        if (blocks[i].exploding)
            continue;

        count++;
    }

    if (count != 5)
        return;

    // 3. Score +5
    ScoreBlockHitGame += 5;

    // 4. Process each block
    for (int i = 0; i < 31; i++)
    {
        if (!blocks[i].active)
            continue;

        if (blocks[i].x != lowestX)
            continue;

        // Reduce color
        if (blocks[i].baseColor > 1)
        {
            blocks[i].baseColor--;
            ColorBlock(blocks[i].baseColor, &blocks[i]);
        }
        else
        {
            // Start explosion
            blocks[i].exploding = TRUE;
            blocks[i].explosionFrame = 1;

            blocks[i].currentFrame =
                (blocks[i].baseColor - 1) * 14 + blocks[i].explosionFrame;

            SPR_setFrame(blocks[i].spr, blocks[i].currentFrame);
        }
    }
}

void AnimateBlockExplosion(void)
{
    for (int i = 0; i < 31; i++)
    {
        if (!blocks[i].active)
            continue;

        if (!blocks[i].exploding)
            continue;

        // Advance explosion frame
        blocks[i].explosionFrame++;

        // If explosion finished → remove block
        if (blocks[i].explosionFrame > 13)
        {
            blocks[i].active = FALSE;
            blocks[i].exploding = FALSE;
            blocks[i].explosionFrame = 0;

            // Move sprite offscreen
            SPR_setPosition(blocks[i].spr, -32, -32);
            continue;
        }

        // Compute correct animation frame
        // baseColor: 1=green, 2=yellow, 3=red
        u8 baseFrame = (blocks[i].baseColor - 1) * 14;

        blocks[i].currentFrame = baseFrame + blocks[i].explosionFrame;

        SPR_setFrame(blocks[i].spr, blocks[i].currentFrame);
    }
}

void CheckGameOverBlockHitGame(void)
{
    u16 joy = playerGetJoyNew();

    // If B is pressed → GAME OVER
    if (joy & BUTTON_B)
        goto GAME_OVER;

    // Check if any normal block reached X < 48
    for (int i = 0; i < 31; i++)
    {
        if (!blocks[i].active)
            continue;

        if (blocks[i].exploding)
            continue;

        if (blocks[i].x < 48)
            goto GAME_OVER;
    }

    // No game over condition → return normally
    return;

GAME_OVER:

//    ResetVariablesBlockHitGame();

    SPR_reset();
    SPR_update();

    cannonSprite = NULL;

    // Draw game over gfx
    VDP_drawImageEx(
        BG_B,
        &blockcannongameover,
        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX),
        0, 0,
        FALSE,
        TRUE
    );

    // Switch state
    bcState = BC_STATE_GAMEOVER;
}

// ---------------------------------------------------------
// 11. Room logic
// ---------------------------------------------------------
GameState runBlockCannon(void)
{
    drawRoomBackground(ROOM_BLOCKCANNON);
    playMusic(tune_ship);

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

    VDP_drawImageEx(
        BG_B,
        &blockcannoningameexample,
        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX),
        0, 0,
        FALSE,
        TRUE
    );

    PAL_fadeIn(0, 15, blockcannoningameexample.palette->data, 8, FALSE);

    // -----------------------------------------------------
    // CALL RESET HERE (MSX equivalent)
    // -----------------------------------------------------
//    ResetVariablesBlockHitGame();

    bcState = BC_STATE_GAME;

    while (1)
    {
        populateControls();
        u16 joyNew = playerGetJoyNew();

        switch (bcState)
        {
            case BC_STATE_TITLE:
                gameJustStarted = FALSE;

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

                    cannonSprite = SPR_addSprite(
                        &blockCannonCannonSpriteDef,
                        8,
                        CannonRowY[CannonRow] - 2,
                        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
                    );

                    bcState = BC_STATE_GAME;
                }
                break;

            case BC_STATE_GAME:
                if (!gameJustStarted)
                {
                    ResetVariablesBlockHitGame();
                    gameJustStarted = TRUE;
                }
                framecounter2++;
                MoveCannon();
                HandleAnimateShootCannon();
                AnimateBlockExplosion();
                CheckInitiateExplosionEntireColumn();
                CheckGameOverBlockHitGame();
                MoveProjectile();
                CheckProjectileHitsBlock();
                CheckShootNewProjectile();
                SetScoreBlockHitGame();
                HandleBlockTiming();
                break;

            case BC_STATE_GAMEOVER:
            {
                // Wait for button press to exit
            if (joyNew & (BUTTON_A | BUTTON_B))
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

        }

        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_ARCADE1;
}
