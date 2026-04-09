#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_jumpquest.h"
#include "game_state.h"
#include "room_arcade1.h"
#include "save_data.h"
#include "player.h"

extern const MapDefinition jump_quest_map;

// ---------------------------------------------------------
// Tile constants (MSX tile numbers)
// ---------------------------------------------------------
#define JumpDownSpikeTile        0x0B
#define JumpDownTrampolineTile   0x06

// ---------------------------------------------------------
// Forward declarations for tile object scanners
// ---------------------------------------------------------
static void CheckForSpikeObject(u8 tile, s16 x, s16 y);
static void CheckForTrampolineObject(u8 tile, s16 x, s16 y);

// ---------------------------------------------------------
// 1. Externs from other modules
// ---------------------------------------------------------
extern const u8* currentColMap;
extern u8 tileContent;

extern void drawRoomBackground(u8 room);
extern void drawDebugInfo(void);

extern u16 globalTileIndex;

// Shared frame counter
u16 framecounter2 = 0;

typedef struct
{
    s16 x;
    s16 y;
    u8 On;
} JumpObject;

static JumpObject FreeToUseObject0;
static JumpObject FreeToUseObject1;
static JumpObject FreeToUseObject2;
static JumpObject FreeToUseObject3;

// Map pointer
static Map* level_1_map;

// ---------------------------------------------------------
// Bunny sprite
// ---------------------------------------------------------
static Sprite* bunnySpr = NULL;

// Bunny world position
static s16 bunnyX;
static s16 bunnyY;

static Sprite* spikeSpr1 = NULL;
static Sprite* spikeSpr2 = NULL;

static const u8 spikeAnimFrames[32] =
{
    0,0,0,0,0,0,0,0,   // 8 frames of idle
    1,2,3,4,5,6,7,8,   // rising
    9,10,10,10,10,10,10,9, // peak hold
    8,7,6,5,4,3,2,1    // falling
};

// ---------------------------------------------------------
// JumpQuest states
// ---------------------------------------------------------
typedef enum
{
    JQ_STATE_TITLE,
    JQ_STATE_GAME
} JumpQuestState;

JumpQuestState jqState = JQ_STATE_TITLE;

static u8 GetSpikeAnimFrame(void)
{
    u8 idx = (framecounter2 >> 1) & 31;   // /2 then mod 32
    return spikeAnimFrames[idx];
}

// ---------------------------------------------------------
// JumpDownGame variables
// ---------------------------------------------------------
static u8 ObjectPhase = 0;

static u16 r23onLineIntJumpDownGame = 57;
static u8 JumpDownGameTilesX = 20;
static u8 JumpDownGameTilesY = 216;

static u8 Row6Wide = 1;
static u8 BuildUpNewRowJumpDownGame = 1;

extern const u8 tilemapjumpdowngame[];
extern const u32 tilemapjumpdowngame_size;

static const u8* TileRowTablePointer = tilemapjumpdowngame;


static u8 PutRemainderTile = 0;
static u16 Scroll27LinesDown = 0;

static u8 JumpBunnyLeft = 0;
static u8 JumpBunnyRight = 0;

static u8 BunnyDied = 0;
static u8 BunnySlidingOnIce = 0;
static u8 SpiderWeb3TimesPressed = 0;
static u8 AnimateSpiderWebJump = 0;

static u8 BunnyJumpedOnTrampoline2 = 0;
static u8 BunnyJumpedOnTrampoline3 = 0;

static u8 Object0On = 0;
static u8 Object1On = 0;
static u8 Object2On = 0;
static u8 Object3On = 0;

static u8 JumpBunnyLeftOnTrampoline = 0;
static u8 JumpBunnyRightOnTrampoline = 0;

static u8 JumpDownGameAmountOfFramesSittingStill = 0;

static u16 ScoreJumpDownGame = 0;
static u8 JumpDownGameSkipFirstScore = 1;

static u8 BunnyFacingRight = 1;

static u8 Scroll4RowsAtStartOfGame = 18;

// ---------------------------------------------------------
// Input latch
// ---------------------------------------------------------
static u16 Controls = 0;

// ---------------------------------------------------------
// Stubs for tile lookup
// ---------------------------------------------------------
static u8 GetTileBunnyStandsOn(void) { return 0; }
static u8 GetTileBunnyStandsOnOffset(s16 offset) { (void)offset; return 0; }

static void UpdateSpikePosition(Sprite* spr, s16 x, s16 y)
{
    SPR_setPosition(spr, x, y);
}

static void HandleSpikeObject(void)
{
    // Spike 1
    if (Object0On)
    {
        u8 frame = GetSpikeAnimFrame();
        SPR_setFrame(spikeSpr1, frame);
        UpdateSpikePosition(spikeSpr1, FreeToUseObject0.x, FreeToUseObject0.y);
    }
    else
    {
        SPR_setPosition(spikeSpr1, -32, -32);   // hide
    }

    // Spike 2
    if (Object1On)
    {
        u8 frame = GetSpikeAnimFrame();
        SPR_setFrame(spikeSpr2, frame);
        UpdateSpikePosition(spikeSpr2, FreeToUseObject1.x, FreeToUseObject1.y);
    }
    else
    {
        SPR_setPosition(spikeSpr2, -32, -32);   // hide
    }
}

// ---------------------------------------------------------
// Empty routines
// ---------------------------------------------------------
static void swap_spat_col_and_char_table(void) {}
static void CheckBunnyInLavaNothingIceOrSpike(void) {}
static void CheckBunnyOffScreen(void) {}
static void CheckShouldBunnyJumpOnTrampoline(void) {}
static void HandleBunnyDied(void) {}

static void HandleTrampolineObject(void) {}
static void PutEdgesOfArcadeMachineFrameBottom(void) {}

static void BuildUpBackgroundJumpDownGame(void)
{
    if (!BuildUpNewRowJumpDownGame)
        return;

    // MSX increments this state machine
    BuildUpNewRowJumpDownGame++;

    // Skip erase steps (2 and 3) – we don’t draw tiles
    if (BuildUpNewRowJumpDownGame == 2 || BuildUpNewRowJumpDownGame == 3)
        return;

    // --- SCAN ONE ROW OF TILES ---

    s16 x = JumpDownGameTilesX;
    s16 y = JumpDownGameTilesY;

    const u8* p = TileRowTablePointer;

    while (x < 215)
    {
        u8 tile = *p;

        // Spawn spike or trampoline if needed
        CheckForSpikeObject(tile, x, y);
        CheckForTrampolineObject(tile, x, y);

        p++;        // next tile
        x += 36;    // next screen X position
    }

    // Save updated pointer
    TileRowTablePointer = p;

    // --- END OF ROW ---

    // Toggle row width (MSX alternates 6-wide / 5-wide)
    Row6Wide ^= 1;

    // Reset X based on row width
    JumpDownGameTilesX = Row6Wide ? (22 - 2) : (40 - 2);

    // Move down one row
    JumpDownGameTilesY += 27;

    // Reset state machine
    BuildUpNewRowJumpDownGame = 0;
}

static void CheckForSpikeObject(u8 tile, s16 x, s16 y)
{
    if (tile != JumpDownSpikeTile)
        return;

    if (!FreeToUseObject0.On)
    {
        FreeToUseObject0.On = 1;
        FreeToUseObject0.x  = x + 9;
        FreeToUseObject0.y  = y + 2;
        return;
    }

    if (!FreeToUseObject1.On)
    {
        FreeToUseObject1.On = 1;
        FreeToUseObject1.x  = x + 9;
        FreeToUseObject1.y  = y + 2;
        return;
    }
}

static void CheckForTrampolineObject(u8 tile, s16 x, s16 y)
{
    if (tile != JumpDownTrampolineTile)
        return;

    if (!FreeToUseObject2.On)
    {
        FreeToUseObject2.On = 1;
        FreeToUseObject2.x  = x + 2;
        FreeToUseObject2.y  = y + 6;
        return;
    }

    if (!FreeToUseObject3.On)
    {
        FreeToUseObject3.On = 1;
        FreeToUseObject3.x  = x + 2;
        FreeToUseObject3.y  = y + 6;
        return;
    }
}


static void HandleScore(void) {}
static void SetBunnySpatCoordinates(void) {}
static void PutEdgesOfArcadeMachineFrameTop(void) {}
static void CheckGameOverJumpDownGame(void) {}
static void SlideOnIceBunnyRight(void) {}
static void SpiderWebJump(void) {}
static void JumpBunnyLeftOnTrampolineRoutine(void) {}
static void JumpBunnyRightOnTrampolineRoutine(void) {}

// ---------------------------------------------------------
// Scroll background
// ---------------------------------------------------------
static void ScrollBackgroundJumpDownGame(void)
{
    if (Scroll27LinesDown == 0)
        return;

    Scroll27LinesDown--;
    r23onLineIntJumpDownGame++;

    bunnyY--;

    MAP_scrollTo(level_1_map, 0, r23onLineIntJumpDownGame);
}

// ---------------------------------------------------------
// Scroll4RowsAtStartOfGameRoutine
// ---------------------------------------------------------
static void Scroll4RowsAtStartOfGameRoutine(void)
{
    if (Scroll27LinesDown != 0)
        return;

    if (Scroll4RowsAtStartOfGame == 0)
        return;

    Scroll4RowsAtStartOfGame--;

    if (Scroll4RowsAtStartOfGame >= 7)
        return;

    BuildUpNewRowJumpDownGame = 1;
    Scroll27LinesDown = 27;
}

// ---------------------------------------------------------
// Jump right movement table
// ---------------------------------------------------------
static const s8 JumpRightTable[] =
{
    +1,-2,  +0,-2,  +1,-1,  +1,-1,  +0, 0,  +1, 0,  +1,+1,  +0, 0,  +1,+1,  +1,+1,
    +0, 0,  +1,+1,  +1,+1,  +0,+1,  +1,+2,  +1,+1,  +0,+1,  +1,+2,  +1,+1,  +0,+2,
    +1,+2,  +1,+2,  +0,+2,  +1,+3,  +1,+3,  +0,+3,  +2,+2
};

static const s8 JumpLeftTable[] =
{
    -1,-2,   0,-2,   -1,-1,  -1,-1,   0, 0,   -1, 0,   -1,+1,   0, 0,   -1,+1,   -1,+1,
     0, 0,  -1,+1,  -1,+1,   0,+1,   -1,+2,  -1,+1,   0,+1,   -1,+2,  -1,+1,    0,+2,
    -1,+2,  -1,+2,   0,+2,  -1,+3,  -1,+3,   0,+3,   -2,+2
};

static void MoveBunnyRight(void)
{
    u8 idx = JumpBunnyRight * 2;
    if (idx >= sizeof(JumpRightTable))
        return;

    bunnyX += JumpRightTable[idx + 0];
    bunnyY += JumpRightTable[idx + 1];
}

static void MoveBunnyLeft(void)
{
    u8 idx = JumpBunnyLeft * 2;
    if (idx >= sizeof(JumpLeftTable))
        return;

    bunnyX += JumpLeftTable[idx + 0];
    bunnyY += JumpLeftTable[idx + 1];
}

// ---------------------------------------------------------
// Show idle sprite
// ---------------------------------------------------------
static void ShowBunnySittingStillSprite(void)
{
    SPR_setFrame(bunnySpr, BunnyFacingRight ? 0 : 1);
}

// ---------------------------------------------------------
// JumpBunnyRightRoutine
// ---------------------------------------------------------
static void JumpBunnyRightRoutine(void)
{
    BunnyFacingRight = 1;

    if (BunnySlidingOnIce)
    {
        SlideOnIceBunnyRight();
        return;
    }

    MoveBunnyRight();
    JumpBunnyRight++;

    if (JumpBunnyRight < 13)
    {
        SPR_setFrame(bunnySpr, 1);
        SPR_setHFlip(bunnySpr, FALSE);
        return;
    }

    if (JumpBunnyRight < 28)
    {
        SPR_setFrame(bunnySpr, 2);
        SPR_setHFlip(bunnySpr, FALSE);
        return;
    }

    JumpBunnyRight = 0;
    SPR_setFrame(bunnySpr, 0);
    SPR_setHFlip(bunnySpr, FALSE);

}

static void JumpBunnyLeftRoutine(void)
{
    BunnyFacingRight = 0;

    if (BunnySlidingOnIce)
    {
        // You can implement this later
        return;
    }

    MoveBunnyLeft();
    JumpBunnyLeft++;

    // Early jump phase
    if (JumpBunnyLeft < 13)
    {
        SPR_setFrame(bunnySpr, 1);
        SPR_setHFlip(bunnySpr, TRUE);
        return;
    }

    // Mid-air phase
    if (JumpBunnyLeft < 28)
    {
        SPR_setFrame(bunnySpr, 2);
        SPR_setHFlip(bunnySpr, TRUE);
        return;
    }

    // End of jump
    JumpBunnyLeft = 0;
    SPR_setFrame(bunnySpr, 0);      // idle-left frame
    SPR_setHFlip(bunnySpr, TRUE);  // ensure clean state
}


// ---------------------------------------------------------
// HandleBunnyJumpAndSetSpriteCharacterAndColor
// ---------------------------------------------------------
static void HandleBunnyJumpAndSetSpriteCharacterAndColor(void)
{
    if (BunnyDied)
        return;

    if (AnimateSpiderWebJump)
    {
        SpiderWebJump();
        return;
    }

    if (JumpBunnyLeftOnTrampoline)
    {
        JumpBunnyLeftOnTrampolineRoutine();
        return;
    }

    if (JumpBunnyRightOnTrampoline)
    {
        JumpBunnyRightOnTrampolineRoutine();
        return;
    }

    if (JumpBunnyLeft)
    {
        JumpBunnyLeftRoutine();
        return;
    }

    if (JumpBunnyRight)
    {
        JumpBunnyRightRoutine();
        return;
    }
}


// ---------------------------------------------------------
// CheckShouldBunnyJump – SGDK-style input
// ---------------------------------------------------------
static void CheckShouldBunnyJump(void)
{
    if (Scroll4RowsAtStartOfGame > 1)
        return;

    if (Scroll27LinesDown != 0)
        return;

    if (BunnyDied != 0)
        return;

    if (Controls & BUTTON_LEFT)
    {
        JumpBunnyLeft = 1;
        BuildUpNewRowJumpDownGame = 1;
        Scroll27LinesDown = 27 + 1; // we increase by one because on the genesis the background rows are 1 line longer
        return;
    }

    if (Controls & BUTTON_RIGHT)
    {
        JumpBunnyRight = 1;
        BuildUpNewRowJumpDownGame = 1;
        Scroll27LinesDown = 27 + 1; // we increase by one because on the genesis the background rows are 1 line longer
        return;
    }
}

// ---------------------------------------------------------
// HandlePhase
// ---------------------------------------------------------
static void HandlePhase(void)
{
    if (ObjectPhase & 1)
        return;

    ObjectPhase = 1;

    bunnyX = 112;
    bunnyY = 232;

    r23onLineIntJumpDownGame = 57;
    JumpDownGameTilesX = 20;
    JumpDownGameTilesY = 216;

    Row6Wide = 1;
    BuildUpNewRowJumpDownGame = 1;

    TileRowTablePointer = tilemapjumpdowngame;

    PutRemainderTile = 0;
    Scroll27LinesDown = 0;
    JumpBunnyLeft = 0;
    JumpBunnyRight = 0;
    BunnyDied = 0;
    BunnySlidingOnIce = 0;
    SpiderWeb3TimesPressed = 0;
    AnimateSpiderWebJump = 0;

    BunnyJumpedOnTrampoline2 = 0;
    BunnyJumpedOnTrampoline3 = 0;

    Object0On = 0;
    Object1On = 0;
    Object2On = 0;
    Object3On = 0;

    JumpBunnyLeftOnTrampoline = 0;
    JumpBunnyRightOnTrampoline = 0;

    JumpDownGameAmountOfFramesSittingStill = 0;

    ScoreJumpDownGame = 0;
    JumpDownGameSkipFirstScore = 1;

    BunnyFacingRight = 1;

    Scroll4RowsAtStartOfGame = 18;
}

// ---------------------------------------------------------
// JumpDownGameRoutine
// ---------------------------------------------------------
void JumpDownGameRoutine(void)
{
    framecounter2++;

    swap_spat_col_and_char_table();
    CheckBunnyInLavaNothingIceOrSpike();
    CheckBunnyOffScreen();

    CheckShouldBunnyJumpOnTrampoline();
    CheckShouldBunnyJump();

    HandleBunnyJumpAndSetSpriteCharacterAndColor();
    HandleBunnyDied();

    HandleSpikeObject();
    HandleTrampolineObject();

    PutEdgesOfArcadeMachineFrameBottom();

    ScrollBackgroundJumpDownGame();
    BuildUpBackgroundJumpDownGame();
    Scroll4RowsAtStartOfGameRoutine();
    HandleScore();
    SetBunnySpatCoordinates();
    PutEdgesOfArcadeMachineFrameTop();
    CheckGameOverJumpDownGame();

    HandlePhase();
}



// ---------------------------------------------------------
// runJumpQuest
// ---------------------------------------------------------
GameState runJumpQuest(void)
{
    drawRoomBackground(ROOM_JUMPQUEST);
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

    jqState = JQ_STATE_GAME;

    VDP_loadTileSet(&jump_quest_tileset, globalTileIndex, DMA);

    level_1_map = MAP_create(
        &jump_quest_map,
        BG_B,
        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, globalTileIndex)
    );

    PAL_fadeIn(0, 15, jumpquestmap.palette->data, 8, FALSE);

    bunnySpr = SPR_addSprite(&jumpQuestRabbitSpriteDef, bunnyX, bunnyY, TILE_ATTR(PAL2, FALSE, FALSE, FALSE));

    spikeSpr1 = SPR_addSprite(&jumpQuestSpikeSpriteDef, 0, 0, TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    spikeSpr2 = SPR_addSprite(&jumpQuestSpikeSpriteDef, 0, 0, TILE_ATTR(PAL2, FALSE, FALSE, FALSE));


    while (1)
    {
        populateControls();
        Controls = playerGetJoy();
        u16 joyNew = playerGetJoyNew();

        switch (jqState)
        {
            case JQ_STATE_TITLE:
                if (joyNew & BUTTON_B)
                {
                    gSave.gamesPlayed++;
                    saveSaveData();
                    return STATE_ARCADE1;
                }
                break;

            case JQ_STATE_GAME:
                JumpDownGameRoutine();
                SPR_setPosition(bunnySpr, bunnyX, bunnyY - 21);
                break;
        }

        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_ARCADE1;
}
