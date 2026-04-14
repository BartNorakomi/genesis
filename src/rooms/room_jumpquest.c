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
// Tile constants (MSX tile numbers / map values)
// ---------------------------------------------------------
#define JumpDownNothingTile          0
#define JumpDownTreeTile             2
#define JumpDownLavaTile             5
#define JumpDownSpikeTile            6
#define JumpDownSpiderWebTile        7
#define JumpDownIceTile              8
#define JumpDownReverseControlsTile  9
#define JumpDownSandTile             10
#define JumpDownTrampolineTile       11
#define JumpDownCloudsTile           12


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
    u8 JumpedOn;   // trampoline animation counter
} JumpObject;

static void CheckCollisionTrampoline(JumpObject* obj);
static void AnimateTrampolineWhenBunnyJumpsOnIt(JumpObject* obj);

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
static Sprite* spikeSpr1 = NULL;
static Sprite* spikeSpr2 = NULL;
static Sprite* trampolineSpr1 = NULL;
static Sprite* trampolineSpr2 = NULL;

// Bunny world position
static s16 bunnyX;
static s16 bunnyY;

// ---------------------------------------------------------
static const u8 spikeAnimFrames[32] =
{
    0,0,0,0,0,0,0,0,
    1,2,3,4,5,6,7,8,
    9,10,10,10,10,10,10,9,
    8,7,6,5,4,3,2,1
};

// ---------------------------------------------------------
// JumpQuest states
// ---------------------------------------------------------
typedef enum
{
    JQ_STATE_TITLE,
    JQ_STATE_GAME,
    JQ_STATE_GAMEOVER
} JumpQuestState;

JumpQuestState jqState = JQ_STATE_TITLE;

// ---------------------------------------------------------
static u8 GetSpikeAnimFrame(void)
{
    return spikeAnimFrames[(framecounter2 >> 1) & 31];
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
// Tile lookup
// ---------------------------------------------------------
static u8 GetTileBunnyStandsOn(void)
{
    // Only valid when Scroll4RowsAtStartOfGame == 1
    // (you can re‑enable this when intro scroll is perfect)
    // if (Scroll4RowsAtStartOfGame != 1)
    //     return 1;

    if (BuildUpNewRowJumpDownGame != 0)
        return 1;

    const u8* row = TileRowTablePointer - 39 + 6;

    u8 x = (u8)bunnyX;

    // // ----- 5‑tile row (centered at 112) -----
    if (x == 112 - 72) return row[-5];
    if (x == 112 - 36) return row[-4];
    if (x == 112)      return row[-3];
    if (x == 112 + 36) return row[-2];
    if (x == 112 + 72) return row[-1];

    // // ----- 6‑tile row (centered at 94) -----
    if (x == 94 - 72)  return row[-6];
    if (x == 94 - 36)  return row[-5];
    if (x == 94)       return row[-4];
    if (x == 94 + 36)  return row[-3];
    if (x == 94 + 72)  return row[-2];
    if (x == 94 + 108) return row[-1];

    return 1;
}

static bool GetTileLeftUnderBunnyStandsOn(void)
{
    if (bunnyX == (94 - 72))
        return true;

    const u8* row = TileRowTablePointer - 39 + 6;

    u8 x = (u8)bunnyX;

    // // ----- 5‑tile row (centered at 112) -----
    if (x == 112 - 72) return row[0];
    if (x == 112 - 36) return row[1];
    if (x == 112)      return row[2];
    if (x == 112 + 36) return row[3];
    if (x == 112 + 72) return row[4];

    // // ----- 6‑tile row (centered at 94) -----
    if (x == 94 - 72)  return row[-1];
    if (x == 94 - 36)  return row[0];
    if (x == 94)       return row[1];
    if (x == 94 + 36)  return row[2];
    if (x == 94 + 72)  return row[3];
    if (x == 94 + 108) return row[4];

    return 1;
}

static bool GetTileRightUnderBunnyStandsOn(void)
{
    if (bunnyX == (94 - 72))
        return true;

    const u8* row = TileRowTablePointer - 39 + 6;

    u8 x = (u8)bunnyX;

    // // ----- 5‑tile row (centered at 112) -----
    if (x == 112 - 72) return row[1];
    if (x == 112 - 36) return row[2];
    if (x == 112)      return row[3];
    if (x == 112 + 36) return row[4];
    if (x == 112 + 72) return row[5];

    // // ----- 6‑tile row (centered at 94) -----
    if (x == 94 - 72)  return row[0];
    if (x == 94 - 36)  return row[1];
    if (x == 94)       return row[2];
    if (x == 94 + 36)  return row[3];
    if (x == 94 + 72)  return row[4];
    if (x == 94 + 108) return row[5];

    return 1;
}

// ---------------------------------------------------------
// Spike handling
// ---------------------------------------------------------
static void UpdateSpikePosition(Sprite* spr, s16 x, s16 y)
{
    SPR_setPosition(spr, x, y);
}

// ---------------------------------------------------------
// ? FIXED SPIKE HANDLER
// ---------------------------------------------------------
static void HandleSpikeObject(void)
{
    if (FreeToUseObject0.On)
    {
        SPR_setFrame(spikeSpr1, GetSpikeAnimFrame());
        UpdateSpikePosition(spikeSpr1, FreeToUseObject0.x, FreeToUseObject0.y);
    }
    else
    {
        SPR_setPosition(spikeSpr1, 100, 216);
    }

    if (FreeToUseObject1.On)
    {
        SPR_setFrame(spikeSpr2, GetSpikeAnimFrame());
        UpdateSpikePosition(spikeSpr2, FreeToUseObject1.x, FreeToUseObject1.y);
    }
    else
    {
        SPR_setPosition(spikeSpr2, 100, 216);
    }
}

static void HandleTrampolineObject(void)
{
    // Trampoline 1 (object 2)
    if (FreeToUseObject2.On)
    {
        CheckCollisionTrampoline(&FreeToUseObject2);
        AnimateTrampolineWhenBunnyJumpsOnIt(&FreeToUseObject2);

        SPR_setPosition(trampolineSpr1, FreeToUseObject2.x, FreeToUseObject2.y);
    }
    else
    {
        SPR_setPosition(trampolineSpr1, 100, 216);
    }

    // Trampoline 2 (object 3)
    if (FreeToUseObject3.On)
    {
        CheckCollisionTrampoline(&FreeToUseObject3);
        AnimateTrampolineWhenBunnyJumpsOnIt(&FreeToUseObject3);

        SPR_setPosition(trampolineSpr2, FreeToUseObject3.x, FreeToUseObject3.y);
    }
    else
    {
        SPR_setPosition(trampolineSpr2, 100, 216);
    }
}

static void CheckCollisionTrampoline(JumpObject* obj)
{
    if (!obj->On)
        return;

    // Bunny X range check
    s16 bx = bunnyX;
    s16 tx = obj->x;

    if (bx < tx - 30) return;
    if (bx > tx + 20) return;

    // Bunny Y check
    s16 by = bunnyY + 33;   // 11 + 22 from MSX
    if (by != obj->y)
        return;

    // Already bouncing?
    if (obj->JumpedOn)
        return;

    obj->JumpedOn = 1;
}

static void AnimateTrampolineWhenBunnyJumpsOnIt(JumpObject* obj)
{
    if (!obj->JumpedOn)
        return;

    obj->JumpedOn++;

    // Move down (counter < 10)
    if (obj->JumpedOn < 10)
    {
        obj->y += 2;
        return;
    }

    // Move up (10–13)
    if (obj->JumpedOn < 14)
    {
        obj->y -= 4;
        return;
    }

    // Wait until 30
    if (obj->JumpedOn < 30)
        return;

    // Reset
    obj->JumpedOn = 0;
}


// ---------------------------------------------------------
// Empty routines (to be ported later)
// ---------------------------------------------------------
static void swap_spat_col_and_char_table(void) {}

static void CheckBunnyInLavaNothingIceOrSpike(void)
{
    if (TileRowTablePointer < tilemapjumpdowngame + 50)
        return;

    // If bouncing on trampoline → ignore ground checks
    if (JumpBunnyLeftOnTrampoline)
        return;

    if (JumpBunnyRightOnTrampoline)
        return;

    u8 tile = GetTileBunnyStandsOn();

    // --- Instant death tiles ---
    if (tile == JumpDownLavaTile)
    {
        if (!BunnyDied) BunnyDied = 1;
        return;
    }

    if (tile == JumpDownNothingTile)
    {
        if (!BunnyDied) BunnyDied = 1;
        return;
    }

    // --- Ice ---
    if (tile == JumpDownIceTile)
    {
        // Sliding direction depends on facing
        if (BunnyFacingRight)
        {
            // Jump right
            JumpBunnyRight = 1;
            BunnySlidingOnIce = 1;
        }
        else
        {
            // Jump left
            JumpBunnyLeft = 1;
            BunnySlidingOnIce = 1;
        }

        BuildUpNewRowJumpDownGame = 1;
        Scroll27LinesDown = 27;

        return;
    }

    // --- Spike ---
    if (tile == JumpDownSpikeTile)
    {
        u8 a = (framecounter2 >> 1) & 31;

        // Active spike window: (17-4) to (23+3)
        if (a >= (17 - 4) && a < (23 + 3))
        {
            if (!BunnyDied)
                BunnyDied = 80;   // explode version

            return;
        }

        return;
    }

    // --- Clouds and beyond ---
    if (tile >= JumpDownCloudsTile)
    {
        if (!BunnyDied) BunnyDied = 1;
        return;
    }
}

static void CheckBunnyOffScreen(void) {}
static void CheckShouldBunnyJumpOnTrampoline(void) {}

static void HandleBunnyDied(void)
{
    // If bunny is not in "dead" state, bail
    if (!BunnyDied)
        return;

    // Advance death timer (wrap protection like the asm: inc + ret z)
    if (BunnyDied != 0xFF)
        BunnyDied++;

    u8 t = BunnyDied;
    u8 frame;

    // These thresholds mirror the sub 20 / sub 6 steps in the asm
    if (t < 20)          frame = 3;   // first phase
    else if (t < 40)     frame = 4;
    else if (t < 60)     frame = 3;
    else if (t < 80)     frame = 4;
    else if (t < 86)     frame = 5;
    else if (t < 92)     frame = 6;
    else if (t < 98)     frame = 7;
    else if (t < 104)    frame = 8;
    else                 frame = 9;   // final explosion frames

    SPR_setFrame(bunnySpr, frame);
    SPR_setHFlip(bunnySpr, BunnyFacingRight ? FALSE : TRUE);
}

static void PutEdgesOfArcadeMachineFrameBottom(void) {}

static void HandleScore(void) {}
static void SetBunnySpatCoordinates(void) {}
static void PutEdgesOfArcadeMachineFrameTop(void) {}

static void CheckGameOverJumpDownGame(void)
{
    // If bunny death timer reached 120 → game over
    if (BunnyDied == 120)
        goto GAME_OVER;

    // If player presses B → game over
    if (Controls & BUTTON_B)
        goto GAME_OVER;

    return;

GAME_OVER:
    SPR_reset();
    SPR_update();

    // bunnySpr = NULL;
    // spikeSpr1 = NULL;
    // spikeSpr2 = NULL;

    // Draw game over gfx
    VDP_drawImageEx(
        BG_B,
        &jumpquestgameover,
        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX),
        0, 0,
        FALSE,
        TRUE
    );

    PAL_setPalette(PAL0, jumpquestgameover.palette->data, DMA);

    jqState = JQ_STATE_GAMEOVER;   // or STATE_ARCADE1 or whatever you use
}

static void SlideOnIceBunnyRight(void) {}
static void SpiderWebJump(void) {}
static void JumpBunnyLeftOnTrampolineRoutine(void) {}
static void JumpBunnyRightOnTrampolineRoutine(void) {}

// ---------------------------------------------------------
// BuildUpBackgroundJumpDownGame
// ---------------------------------------------------------
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
        u8 tileb = *p;

        // Spawn spike or trampoline if needed
        CheckForSpikeObject(tileb, x, y);
        CheckForTrampolineObject(tileb, x, y);

        p++;        // next tile
        x += 36;    // next screen X position
    }

    // Save updated pointer
    TileRowTablePointer = p;

    // --- END OF ROW ---

    // Toggle row width (MSX alternates 6-wide / 5-wide visually)
    Row6Wide ^= 1;

    // Reset X based on row width
    JumpDownGameTilesX = Row6Wide ? (22 - 2) : (40 - 2);

    // Move down one row
    JumpDownGameTilesY += 27;

    // Reset state machine
    BuildUpNewRowJumpDownGame = 0;
}

// ---------------------------------------------------------
// Object scanners
// ---------------------------------------------------------
static void CheckForSpikeObject(u8 tileb, s16 x, s16 y)
{
    char buf[8];
    sprintf(buf, "%3d", tilemapjumpdowngame[8]);
    VDP_drawTextBG(BG_A, buf, 25, 27);

    char buff[8];
    sprintf(buff, "%3d", TileRowTablePointer);
    VDP_drawTextBG(BG_A, buff, 18, 27);

   char buffff[8];
   sprintf(buffff, "%3d", tilemapjumpdowngame);
   VDP_drawTextBG(BG_A, buffff, 0, 27);

    if (tileb != JumpDownSpikeTile)
        return;

    if (!FreeToUseObject0.On)
    {
        FreeToUseObject0.On = 1;
        FreeToUseObject0.x  = x + 10;
        FreeToUseObject0.y  = 228;
        return;
    }

    if (!FreeToUseObject1.On)
    {
        FreeToUseObject1.On = 1;
        FreeToUseObject1.x  = x + 10;
        FreeToUseObject1.y  = 228;
        return;
    }
}

static void CheckForTrampolineObject(u8 tileb, s16 x, s16 y)
{
    if (tileb != JumpDownTrampolineTile)
        return;

    if (!FreeToUseObject2.On)
    {
        FreeToUseObject2.On = 1;
        FreeToUseObject2.x  = x + 2;
        FreeToUseObject2.y  = 228;
        return;
    }

    if (!FreeToUseObject3.On)
    {
        FreeToUseObject3.On = 1;
        FreeToUseObject3.x  = x + 2;
        FreeToUseObject3.y  = 228;
        return;
    }
}

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
    FreeToUseObject0.y--;
    FreeToUseObject1.y--;
    FreeToUseObject2.y--;
    FreeToUseObject3.y--;

    if (FreeToUseObject0.y <= -16) FreeToUseObject0.On = 0;
    if (FreeToUseObject1.y <= -16) FreeToUseObject1.On = 0;
    if (FreeToUseObject2.y <= -16) FreeToUseObject2.On = 0;
    if (FreeToUseObject3.y <= -16) FreeToUseObject3.On = 0;

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
// Jump movement tables
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
// Jump routines
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
    SPR_setHFlip(bunnySpr, TRUE);   // ensure clean state
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
    // Scroll intro still running? -> no jump
    if (Scroll4RowsAtStartOfGame > 1)
        return;

    // Background still scrolling? -> no jump
    if (Scroll27LinesDown != 0)
        return;

    // Bunny dead? -> no jump
    if (BunnyDied != 0)
        return;

    // --- Tile under bunny decides special behavior ---
    u8 tile = GetTileBunnyStandsOn();

    char bufff[8];
    sprintf(bufff, "%3d", tile);
    VDP_drawTextBG(BG_A, bufff, 13, 27);

    // Spider web: handled elsewhere, just bail here
    if (tile == JumpDownSpiderWebTile)
    {
        // BunnyStandsInSpiderWeb equivalent is your SpiderWebJump / AnimateSpiderWebJump logic
        return;
    }

    // Reverse controls?
    bool reverseControls = (tile == JumpDownReverseControlsTile);

    // Sand tile uses "new press" instead of held input
    u16 inputSource = (tile == JumpDownSandTile) ? playerGetJoyNew() : Controls;

    // We only care about LEFT / RIGHT
    u16 lrMask = BUTTON_LEFT | BUTTON_RIGHT;
    u16 lr = inputSource & lrMask;

    if (!lr)
        return;

    // Decide which direction is "right" depending on reverse controls
    u16 wantRight = reverseControls ? BUTTON_LEFT : BUTTON_RIGHT;

    if (lr & wantRight)
    {
        u8 tileRightUnderBunny = GetTileRightUnderBunnyStandsOn();

        //        CheckTreeWhenJumpingLeft();
        // --- Jump LEFT ---
        if (tileRightUnderBunny == JumpDownTreeTile)
            return;     // standing on a tree → cannot jump left

        JumpBunnyRight = 1;
        BuildUpNewRowJumpDownGame = 1;
        Scroll27LinesDown = 27 + 1;   // +1 because of Genesis row height difference
    }
    else
    {
        // --- Jump LEFT ---
        u8 tileLeftUnderBunny = GetTileLeftUnderBunnyStandsOn();

        //        CheckTreeWhenJumpingLeft();
        // --- Jump LEFT ---
        if (tileLeftUnderBunny == JumpDownTreeTile)
            return;     // standing on a tree → cannot jump left

        JumpBunnyLeft = 1;
        BuildUpNewRowJumpDownGame = 1;
        Scroll27LinesDown = 27 + 1;
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
 
    FreeToUseObject0.On = 0;
    FreeToUseObject1.On = 0;
    FreeToUseObject2.On = 0;
    FreeToUseObject3.On = 0;

    JumpBunnyLeftOnTrampoline = 0;
    JumpBunnyRightOnTrampoline = 0;

    JumpDownGameAmountOfFramesSittingStill = 0;

    ScoreJumpDownGame = 0;
    JumpDownGameSkipFirstScore = 1;

    BunnyFacingRight = 1;

    Scroll4RowsAtStartOfGame = 18;

    bunnySpr = SPR_addSprite(&jumpQuestRabbitSpriteDef, bunnyX, bunnyY, TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    spikeSpr1 = SPR_addSprite(&jumpQuestSpikeSpriteDef, 0, 0, TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    spikeSpr2 = SPR_addSprite(&jumpQuestSpikeSpriteDef, 0, 0, TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    trampolineSpr1 = SPR_addSprite(&jumpQuestTrampolineSpriteDef, 0, 0, TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    trampolineSpr2 = SPR_addSprite(&jumpQuestTrampolineSpriteDef, 0, 0, TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
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
                    MAP_release(level_1_map);
                    return STATE_ARCADE1;
                }
                if (joyNew & BUTTON_A)
                {
                    VDP_loadTileSet(&jump_quest_tileset, globalTileIndex, DMA);
                    MAP_release(level_1_map);
                    
                    level_1_map = MAP_create(
                        &jump_quest_map,
                        BG_B,
                        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, globalTileIndex)
                    );

                    PAL_fadeIn(0, 15, jumpquestmap.palette->data, 8, FALSE);

                    ObjectPhase = 0;
                    jqState = JQ_STATE_GAME;
                }
                break;

            case JQ_STATE_GAME:
                JumpDownGameRoutine();
                SPR_setPosition(bunnySpr, bunnyX, bunnyY - 21);
                break;

            case JQ_STATE_GAMEOVER:
                {
                VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
                VDP_setVerticalScroll(BG_B, 0);

                    // Wait for button press to exit
                if (joyNew & (BUTTON_A | BUTTON_B))
                {
                    PAL_fadeOut(0, 15, 8, FALSE);

                        // Draw title screen gfx
                    VDP_drawImageEx(
                        BG_B,
                        &jumpquesttitlescreen,
                        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX),
                        0, 0,
                        FALSE,
                        TRUE
                    );

                    PAL_fadeIn(0, 15, jumpquesttitlescreen.palette->data, 8, FALSE);

                    jqState = JQ_STATE_TITLE;
                }

                break;
                }
        }

        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_ARCADE1;
}
