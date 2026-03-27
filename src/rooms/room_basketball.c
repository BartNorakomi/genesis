#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_basketball.h"
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

// ---------------------------------------------------------
// 2. Internal Basketball states
// ---------------------------------------------------------
typedef enum
{
    BB_STATE_TITLE,
    BB_STATE_GAME,
    BB_STATE_GAMEOVER
} BasketballState;

// ---------------------------------------------------------
// 3. Local sprite pointers
// ---------------------------------------------------------
static Sprite* basketballSprite;
static Sprite* basketballPoleSprite;
static Sprite* basketballPoleSprite2;
static Sprite* basketballNetSprite;
static Sprite* basketballShadowSprite;
static Sprite* basketballCoinSprite;

// ---------------------------------------------------------
// 4. Ball physics variables
// ---------------------------------------------------------
static u8 ballX = 120;
static s16 ballY = 200;
static s16 ballHorizSpeed = 0;
static s16 ballVertSpeed = 0;
static u16 framecounter2 = 0;


static u8  basketballCombotime;
static u8  basketballCombo;
static u16 basketballScore;
static u8  basketballresettimervar;
static u8  basketballMaxtime = 150;
static u8  basketballMaxCombotime;

// ---------------------------------------------------------
// 5. Hoop physics variables
// ---------------------------------------------------------
static s16 hoopLeftX = -25;
static s16 hoopRightX = 256;
static s16 hoopPhase = 1;
static u8 hoopVar1 = 0;

// Coin position variables (merged)
static u8 coinX = 0;
static u8 coinY = 213;   // 213 = hidden, same as original Z80 logic
static u8 basketballcoins = 0;

static u8  basketballHudHandler = 0;
static u8  basketballtime       = 150;

// NEW: scoring flag
static u8 basketballFirstPointScored = 0;

static u8 freezeControls = 0;

// Basket spacing
#define XDifferenceBaskets 173
#define XDifferenceBackboards 201

// ---------------------------------------------------------
// 5. Subroutine stubs (converted from 68k structure)
// ---------------------------------------------------------

// --- Bounce helpers ---
static void CheckBounceOnRightRimLeftSide(void);
static void CheckBounceOnRightBackboard(void);
static void CheckBounceOnRightRimRightSide(void);

static void CheckBounceOnLeftRimLeftSide(void);
static void CheckBounceOnLeftBackboard(void);
static void CheckBounceOnLeftRimRightSide(void);

// --- Bounce groups ---
static void CheckBounceRightRimAndBackboard(void)
{
    CheckBounceOnRightRimLeftSide();
    CheckBounceOnRightBackboard();
    CheckBounceOnRightRimRightSide();
}

static void CheckBounceLeftRimAndBackboard(void)
{
    CheckBounceOnLeftRimLeftSide();
    CheckBounceOnLeftBackboard();
    CheckBounceOnLeftRimRightSide();
}

// ---------------------------------------------------------
// ApplyGravity
// ---------------------------------------------------------
static void ApplyGravity(void)
{
    if ((framecounter2 & 3) != 0)
        return;

    ballVertSpeed++;
}

// ---------------------------------------------------------
// MoveBallVertically
// ---------------------------------------------------------
static void MoveBallVertically(void)
{
    char buf[8];
    sprintf(buf, "%3d", ballY);
    VDP_drawTextBG(BG_A, buf, 25, 27);

    if (ballVertSpeed < 0)
    {
        ballY += ballVertSpeed;

        if (ballY > 103)
            return;

        ballY = 103;
        return;
    }

    ballY += ballVertSpeed;

    if (ballY <= 255)
        return;

    ballY = 255;

    ballVertSpeed--;

    if (ballVertSpeed < 0)
        return;

    ballVertSpeed = -ballVertSpeed;
    framecounter2 = 2;
}

// ---------------------------------------------------------
// SetXYinSpat
// ---------------------------------------------------------
static void SetXYinSpat(void)
{
    SPR_setPosition(basketballSprite, ballX, ballY - 144);
    SPR_setPosition(basketballShadowSprite, ballX, 110);
}

// ---------------------------------------------------------
// HandleTrigA
// ---------------------------------------------------------
static void HandleTrigA(void)
{
    u16 joyNew = playerGetJoyNew();
    if (!(joyNew & BUTTON_A))
        return;

    s16 b = 0;

    if (hoopPhase == 2)
        b = 0;
    else if (hoopPhase < 2)
        b = -1;
    else if (hoopPhase == 6)
        b = 0;
    else if (hoopPhase < 6)
        b = +1;
    else
        b = -1;

    ballHorizSpeed = b;

    ballVertSpeed -= 1;

    if (ballVertSpeed < -3)
    {
        ballVertSpeed = -5;
        return;
    }

    ballVertSpeed = -5;
}

// ---------------------------------------------------------
// SetHorizontalSpeed
// ---------------------------------------------------------
static void SetHorizontalSpeed(void)
{
    if (ballY < 255)
        return;

    s16 b = 0;

    if (hoopPhase == 0)
        b = 1;
    else if (hoopPhase == 5)
        b = -1;
    else
        b = 0;

    if (ballHorizSpeed == b)
    {
        ballHorizSpeed = 0;
        return;
    }

    if (ballVertSpeed < 2)
    {
        ballHorizSpeed = 0;
        return;
    }

    if (ballVertSpeed < 255)
        return;

    ballHorizSpeed = 0;
}

// ---------------------------------------------------------
// MoveBallHorizontally
// ---------------------------------------------------------
static void MoveBallHorizontally(void)
{
    ballX += ballHorizSpeed;
}

// ---------------------------------------------------------
// Rim bounce routines (Left Rim Left Side)
// ---------------------------------------------------------

static void ChangeHorizontalSpeedRimHit2(void)
{
    if (ballX >= (33 - 5))
    {
        ballHorizSpeed = 1;
        return;
    }

    if (ballX < (17 + 5))
    {
        ballHorizSpeed = -1;
        return;
    }

    if (ballHorizSpeed != 0)
    {
        ballHorizSpeed = 0;
        return;
    }

    ballHorizSpeed = 1;
}

static void ChangeVerticalSpeedRimHit2(void)
{
    if (ballY >= (200 - 5))
    {
        if (ballVertSpeed >= 0)
            return;

        ballVertSpeed = -ballVertSpeed - 1;
        return;
    }

    if (ballY < (180 + 5))
    {
        ballVertSpeed--;

        if (ballVertSpeed < 0)
        {
            ballVertSpeed = -ballVertSpeed;
            framecounter2 = 2;
        }
        return;
    }
}

static void CheckBounceOnLeftRimLeftSide(void)
{
    if (ballX >= 33) return;
    if (ballX < 17) return;

    if (ballY < 180) return;
    if (ballY >= 200) return;

    ChangeHorizontalSpeedRimHit2();
    ChangeVerticalSpeedRimHit2();
}

// ---------------------------------------------------------
// Rim bounce routines (Left Rim Right Side)
// ---------------------------------------------------------

static void ChangeHorizontalSpeedRimHit(void)
{
    if (ballX >= (53 - 5))
    {
        ballHorizSpeed = 1;
        return;
    }

    if (ballX < (38 + 5))
    {
        ballHorizSpeed = -1;
        return;
    }

    if (ballHorizSpeed != 0)
    {
        ballHorizSpeed = 0;
        return;
    }

    ballHorizSpeed = -1;
}

static void ChangeVerticalSpeedRimHit(void)
{
    if (ballY >= (200 - 5))
    {
        if (ballVertSpeed >= 0)
            return;

        ballVertSpeed = -ballVertSpeed - 1;
        return;
    }

    if (ballY < (180 + 5))
    {
        ballVertSpeed--;

        if (ballVertSpeed < 0)
        {
            ballVertSpeed = -ballVertSpeed;
            framecounter2 = 2;
        }
        return;
    }
}

static void CheckBounceOnLeftRimRightSide(void)
{
    if (ballX >= 53) return;
    if (ballX < 38) return;

    if (ballY < 180) return;
    if (ballY >= 200) return;

    ChangeHorizontalSpeedRimHit();
    ChangeVerticalSpeedRimHit();
}

// ---------------------------------------------------------
// Backboard bounce routines (Left)
// ---------------------------------------------------------

static void ChangeHorizontalSpeedBackboardHit(void)
{
    if (ballX >= (27 - 5))
    {
        ballHorizSpeed = 1;
        return;
    }

    if (ballX < (13 + 5))
    {
        ballHorizSpeed = -1;
        return;
    }

    if (ballHorizSpeed != 0)
    {
        ballHorizSpeed = 0;
        return;
    }

    ballHorizSpeed = 1;
}

static void ChangeVerticalSpeedBackboardHit(void)
{
    if (ballY >= (205 - 5))
    {
        if (ballVertSpeed >= 0)
            return;

        ballVertSpeed = -ballVertSpeed - 1;
        return;
    }

    if (ballY < (159 + 5))
    {
        ballVertSpeed--;

        if (ballVertSpeed < 0)
        {
            ballVertSpeed = -ballVertSpeed;
            framecounter2 = 2;
        }
        return;
    }
}

static void CheckBounceOnLeftBackboard(void)
{
    if (ballX >= 27) return;
    if (ballX < 13) return;

    if (ballY < 159) return;
    if (ballY >= 205) return;

    ChangeHorizontalSpeedBackboardHit();
    ChangeVerticalSpeedBackboardHit();
}

// ---------------------------------------------------------
// NEW: Right Rim Left Side bounce routine
// ---------------------------------------------------------

static void CheckBounceOnRightRimLeftSide(void)
{
    if (ballX >= (33 + XDifferenceBaskets)) return;
    if (ballX <  (17 + XDifferenceBaskets)) return;

    if (ballY < 180) return;
    if (ballY >= 200) return;

    ChangeHorizontalSpeedRimHit2();
    ChangeVerticalSpeedRimHit2();
}

// ---------------------------------------------------------
// NEW: Right Backboard bounce routine
// ---------------------------------------------------------

static void CheckBounceOnRightBackboard(void)
{
    if (ballX >= (27 + XDifferenceBackboards)) return;
    if (ballX <  (13 + XDifferenceBackboards)) return;

    if (ballY < 159) return;
    if (ballY >= 205) return;

    ChangeHorizontalSpeedBackboardHit();
    ChangeVerticalSpeedBackboardHit();
}

// ---------------------------------------------------------
// NEW: Right Rim Right Side bounce routine
// ---------------------------------------------------------

static void CheckBounceOnRightRimRightSide(void)
{
    if (ballX >= (53 + XDifferenceBaskets)) return;
    if (ballX <  (38 + XDifferenceBaskets)) return;

    if (ballY < 180) return;
    if (ballY >= 200) return;

    ChangeHorizontalSpeedRimHit();
    ChangeVerticalSpeedRimHit();
}

// ---------------------------------------------------------
// NEW: Scoring routines
// ---------------------------------------------------------

static void CheckScoredLeftSide(void)
{
    if (ballX < (33 - 1)) return;
    if (ballX >= (38 + 1)) return;

    if (ballY < 187) return;
    if (ballY >= (194 + 4)) return;

//    hoopPhase = 5;
    hoopPhase = 2;
    ballHorizSpeed = 0;
    ballX = 36;
    basketballFirstPointScored = 1;
}

static void CheckScoredRightSide(void)
{
    if (ballX < (33 + XDifferenceBaskets - 1)) return;
    if (ballX >= (38 + XDifferenceBaskets + 1)) return;

    if (ballY < 187) return;
    if (ballY >= (194 + 4)) return;

//    hoopPhase = 0;
    hoopPhase = 6;
    ballHorizSpeed = 0;
    ballX = 36 + XDifferenceBaskets - 3;
    basketballFirstPointScored = 1;
}

static void CheckScore(void)
{
    if (hoopPhase == 0)
    {
        CheckScoredLeftSide();
        return;
    }

    if (hoopPhase == 5)
    {
        CheckScoredRightSide();
        return;
    }
}

// ---------------------------------------------------------
// Other routines
// ---------------------------------------------------------
static void SetBallShadow(void) {}
static void HandlePhase(void) {}

static void HUD_DrawScore(void)
{
    char buf[6];
    sprintf(buf, "%5u", basketballScore);
    VDP_drawText("SCORE:", 10, 5);
    VDP_drawText(buf,      16, 5);
}

static void HUD_DrawCombo(void)
{
    if (basketballCombo <= 1)
    {
        VDP_drawText("      ", 10, 6); // clear
        return;
    }

    char buf[6];
    sprintf(buf, "%5u", basketballCombo);
    VDP_drawText("COMBO:", 10, 6);
    VDP_drawText(buf,      16, 6);
}

static void HUD_DrawTimeBar(void)
{
    if (!basketballFirstPointScored)
        return;

    char bar[13];
    u8 len = (basketballtime * 12) / basketballMaxtime;
    for (u8 i = 0; i < 12; i++)
        bar[i] = (i < len) ? '#' : ' ';
    bar[12] = '\0';

    VDP_drawText("TIME:",  2, 2);
    VDP_drawText(bar,      8, 2);
}

static void HUD_DrawComboBar(void)
{
    if (!basketballFirstPointScored)
        return;

    char bar[13];
    u8 len = (basketballCombotime * 12) / basketballMaxCombotime;
    for (u8 i = 0; i < 12; i++)
        bar[i] = (i < len) ? '#' : ' ';
    bar[12] = '\0';

    VDP_drawText("CMB:",  2, 3);
    VDP_drawText(bar,     8, 3);
}


static void HandleBasketBallgameHud(void)
{
    // If hoop is in a moving/scoring phase, run reset logic
    if ((hoopPhase != 0) && (hoopPhase != 5))
    {
        // ResetTimers equivalent
        basketballresettimervar++;

        if (basketballresettimervar < 4)
        {
            // FillTimerAndComboBars
            basketballHudHandler = 255;
            basketballtime       = basketballMaxtime;
            basketballCombotime  = basketballMaxCombotime;
            HUD_DrawTimeBar();
            HUD_DrawComboBar();
            return;
        }

        if (basketballresettimervar < 8)
        {
            // SetComboText
            HUD_DrawCombo();
            return;
        }

        if (basketballresettimervar < 12)
        {
            // Score
            HUD_DrawScore();
            return;
        }

        if (basketballresettimervar >= 40)
        {
            // ClearComboText
            VDP_drawText("      ", 10, 4);
        }

        return;
    }

    // Normal HUD handler state machine
    basketballHudHandler++;
    switch (basketballHudHandler)
    {
        case 0: // overflow to 0
            // ReduceTimeAndSetDX
            if (basketballFirstPointScored)
            {
                if (basketballtime > 0)
                    basketballtime--;
                HUD_DrawTimeBar();
            }
            break;

        case 1:
            // PutBarIn4Pages -> here just redraw bars
            HUD_DrawTimeBar();
            HUD_DrawComboBar();
            break;

        case 2:
            // ReduceComboTimeAndSetDX
            if (basketballFirstPointScored)
            {
                if (basketballCombotime > 0)
                    basketballCombotime--;
                HUD_DrawComboBar();
            }
            break;

        case 3:
            // PutBarIn4Pages again
            HUD_DrawTimeBar();
            HUD_DrawComboBar();
            break;

        default:
            basketballHudHandler = 255;
            break;
    }
}

static void SetwallCoverUps(void) {}
static void HandleBallShadowCoverUp(void) {}

static void HandleNet(void)
{
    // Default: hide open frame, show normal frame
    u8 frame = 0;
    s16 netX = 0;
    s16 netY = 0;

    switch (hoopPhase)
    {
        case 0: // Left side idle
        {
            frame = 0;
            netX = 34;
            netY = 53;
            break;
        }

        case 2: // Left just scored (wait)
        {
            if (ballY < 215)
            {
                // Open net
                frame = 1;
                netX = 34;
                netY = 53;
            }
            else
            {
                // Normal net
                frame = 0;
                netX = 34;
                netY = 53;
            }
            break;
        }

        case 5: // Right side idle
        {
            frame = 2;
            netX = 207;
            netY = 53;
            break;
        }

        case 6: // Right just scored (wait)
        {
            if (ballY < 215)
            {
                // Open net
                frame = 3;
                netX = 208;
                netY = 53;
            }
            else
            {
                // Normal net
                frame = 2;
                netX = 207;
                netY = 53;
            }
            break;
        }

        default:
        {
            // Offscreen / transition
            netX = 0;
            netY = 213;
            break;
        }
    }
    
    SPR_setFrame(basketballNetSprite, frame);
    SPR_setPosition(basketballNetSprite, netX, netY);
}

static void HandleBasketBallGameOver(BasketballState* bbState)
{
    // freezecontrols = 0
    freezeControls = 0;

    // If basketballtime != 1 → not game over yet
    if (basketballtime > 1)
        return;

    // freezecontrols = 1
    freezeControls = 1;

    // Ball must be lying on the ground (Y == 255)
    if (ballY != 255)
        return;

    // Vertical speed must be 0
    if (ballVertSpeed != 0)
        return;

    VDP_drawImageEx(BG_B, &basketballgameover, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX), 0, 0, FALSE, TRUE);

    // GAME OVER!
    *bbState = BB_STATE_GAMEOVER;
}

static void HandlePickUpCoin(void)
{
    // If coin is hidden, nothing to do
    if (coinY == 213)
        return;

    // -----------------------------------------
    // X collision check
    // -----------------------------------------
    // Equivalent to:
    // if (coinX - 14 <= ballX <= coinX + 13)
    if (ballX < (coinX - 14))
        return;
   if (ballX > (coinX + 13))
       return;

    // -----------------------------------------
    // Y collision check
    // -----------------------------------------
    // Equivalent to:
    // if (coinY - 14 <= ballY <= coinY + 13)
   if (ballY < (coinY - 14 +144))
       return;
    if (ballY > (coinY + 13 +144))
        return;

    // -----------------------------------------
    // Collision confirmed → pick up coin
    // -----------------------------------------
    basketballcoins++;

    // Hide coin (Z80 used Y=213 and X=255)
    coinY = 213;
    coinX = 255;

    SPR_setPosition(basketballCoinSprite, coinX, coinY);
}

static void HandleTriggerBForGameOver(void)
{
    u16 joy = playerGetJoyNew();

    // Trigger B pressed?
    if (joy & BUTTON_B)
    {
        basketballtime = 1;
    }
}

static void IncreaseScoreAndComboDecreaseMaxTimers(void)
{
    // -----------------------------------------
    // Decrease combo timer
    // -----------------------------------------
    basketballCombotime--;
    if (basketballCombotime == 0)
        basketballCombo = 0;

    // -----------------------------------------
    // Increase combo
    // -----------------------------------------
    basketballCombo++;

    // -----------------------------------------
    // Add combo to score
    // -----------------------------------------
    basketballScore += basketballCombo;

    // -----------------------------------------
    // Reset timer var
    // -----------------------------------------
    basketballresettimervar = 255;

    // -----------------------------------------
    // Decrease max time, clamp to 50
    // -----------------------------------------
    basketballMaxtime -= 2;
    if (basketballMaxtime < 50)
        basketballMaxtime = 50;

    // -----------------------------------------
    // Decrease max combo time, clamp to 50
    // -----------------------------------------
    basketballMaxCombotime -= 2;
    if (basketballMaxCombotime < 50)
        basketballMaxCombotime = 50;

    // -----------------------------------------
    // 50/50 random chance to stop here
    // -----------------------------------------
    if (framecounter2  & 1)
        return;

    // -----------------------------------------
    // If coin already visible, stop
    // -----------------------------------------
    if (coinY != 213)
        return;

    // -----------------------------------------
    // Random Y between 15 and 39
    // -----------------------------------------
    u8 y = (framecounter2  & 31) + 15;
    if (y >= 40)
        y = 24;

    coinY = y;

    // -----------------------------------------
    // Random X + 64
    // -----------------------------------------
    u8 x = (framecounter2  & 127) + 64;
    coinX = x;

    // -----------------------------------------
    // Move SGDK sprite
    // -----------------------------------------
    SPR_setPosition(basketballCoinSprite, coinX, coinY);
}

static void HandleHoopPhase(void)
{
    SPR_setPosition(basketballPoleSprite, hoopLeftX, 30);
    SPR_setPosition(basketballPoleSprite2, hoopRightX, 30);

    switch (hoopPhase)
    {
        case 0:
            // do nothing (left side)
            return;

        case 1: // Phase1: scroll into screen from left
        {
            hoopLeftX += 2;
            if (hoopLeftX == 21)
                hoopPhase = 0;            

            return;
        }

        case 2: // Phase2: wait
        {
            if (hoopVar1 == 0)
                IncreaseScoreAndComboDecreaseMaxTimers();

            hoopVar1 = (hoopVar1 + 1) & 15;

            if (hoopVar1 == 0)
                hoopPhase = 3;

            return;
        }

        case 3: // Phase3: scroll out left
        {
            hoopLeftX -= 2;
            if (hoopLeftX == -25)
                hoopPhase = 4;
  
            return;
        }

        case 4: // Phase4: scroll into screen from right
        {
            hoopRightX -= 2;
            if (hoopRightX == 204)
                hoopPhase = 5;            

            return;
        }

        case 5:
            // do nothing (right side)
            return;

        case 6: // Phase6: wait
        {
            if (hoopVar1 == 0)
                IncreaseScoreAndComboDecreaseMaxTimers();

            hoopVar1 = (hoopVar1 + 1) & 15;

            if (hoopVar1 == 0)
                hoopPhase = 7;

            return;
        }

        case 7: // Phase7: scroll out right
        {
            hoopRightX += 2;
            if (hoopRightX == 256)
                hoopPhase = 1;

            return;
        }
    }
}

// ---------------------------------------------------------
// Main ball routine
// ---------------------------------------------------------
static void HandleBasketBall(void)
{
    ApplyGravity();
    SetXYinSpat();
    MoveBallVertically();
    HandleTrigA();
    SetHorizontalSpeed();
    MoveBallHorizontally();
    CheckScore();

    if (hoopPhase == 0) CheckBounceLeftRimAndBackboard();
    if (hoopPhase == 5) CheckBounceRightRimAndBackboard();
}

// ---------------------------------------------------------
// 6. Room logic
// ---------------------------------------------------------
GameState runBasketball(void)
{
    drawRoomBackground(ROOM_BASKETBALL);
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
        &basketballingameexample,
        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX),
        0, 0,
        FALSE,
        TRUE
    );

    PAL_fadeIn(0, 15, basketballingameexample.palette->data, 8, FALSE);

    basketballNetSprite  = SPR_addSprite(&basketBallNetSpriteDef, 48, 48, TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    basketballSprite     = SPR_addSprite(&basketBallSpriteDef, ballX, ballY, TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    basketballPoleSprite = SPR_addSprite(&basketBallPoleSpriteDef, hoopLeftX, 30, TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    basketballPoleSprite2 = SPR_addSprite(&basketBallPoleSpriteDef, hoopRightX, 30, TILE_ATTR(PAL2, FALSE, FALSE, TRUE));
    basketballShadowSprite = SPR_addSprite(&basketBallShadowSpriteDef, ballX, ballY, TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    basketballCoinSprite = SPR_addSprite(&basketBallCoinSpriteDef, coinX, coinY, TILE_ATTR(PAL2, FALSE, FALSE, FALSE));

    BasketballState bbState = BB_STATE_GAME;

    while (1)
    {
        playerHandleInput();
        u16 joyNew = playerGetJoyNew();

        switch (bbState)
        {
            case BB_STATE_TITLE:
                if (joyNew & BUTTON_B)
                    return STATE_ARCADE1;

                if (joyNew & BUTTON_A)
                {
                    PAL_fadeOut(0, 15, 8, FALSE);
                    VDP_drawImageEx(BG_B, &basketballingameexample,
                        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX),
                        0, 0, FALSE, TRUE);
                    PAL_fadeIn(0, 15, basketballingameexample.palette->data, 8, FALSE);
                    bbState = BB_STATE_GAME;
                    //reset variables
                    // Reset ball position and sprite frame
                    ballX = 120;
                    ballY = 200;
                    ballHorizSpeed = 0;
                    ballVertSpeed = 0;

                    // Reset hoop phase
                    hoopPhase = 1;
                    hoopLeftX = -25;
                    hoopRightX = 256;
                    hoopVar1 = 0;

                    // Reset gameplay flags
                    freezeControls = 0;
                    basketballFirstPointScored = 0;
                    basketballcoins = 0;

                    // If you have this variable, reset it too:
                    // basketballTitleScreenButton = 0;

                    // Reset score + combo
                    basketballScore = 0;
                    basketballCombo = 0;

                    // Reset timers
                    basketballtime = 150;
                    basketballMaxtime = 150;

                    basketballCombotime = 100;
                    basketballMaxCombotime = 100;

                    // HUD handler reset
                    basketballHudHandler = 255;

                    // // Reset coin
                    // coinX = 255;
                    // coinY = 213;

                    // // Reset sprites (SGDK equivalent of SetBasketballGameSprites)
                    // SPR_setPosition(basketballSprite, ballX, ballY - 144);
                    // SPR_setPosition(basketballShadowSprite, ballX, 110);
                    // SPR_setPosition(basketballCoinSprite, coinX, coinY);

                    // // Reset net + poles
                    // SPR_setPosition(basketballPoleSprite, hoopLeftX, 30);
                    // SPR_setPosition(basketballPoleSprite2, hoopRightX, 30);
                    // SPR_setFrame(basketballNetSprite, 0);



                                    
                }
                break;

            case BB_STATE_GAME:
                framecounter2++;

                HandlePhase();
                HandleBasketBallgameHud();
                HandleBasketBall();
                SetwallCoverUps();
                SetBallShadow();
                HandleBallShadowCoverUp();
                HandleNet();
                HandleBasketBallGameOver(&bbState);
                HandlePickUpCoin();
                HandleTriggerBForGameOver();
                
                HandleHoopPhase();

                // if (joyNew & BUTTON_B)
                // {
                //     PAL_fadeOut(0, 15, 8, FALSE);
                //     VDP_drawImageEx(BG_B, &basketballtitlescreen,
                //         TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX),
                //         0, 0, FALSE, TRUE);
                //     PAL_fadeIn(0, 15, basketballtitlescreen.palette->data, 8, FALSE);
                //     bbState = BB_STATE_TITLE;
                // }
                break;
    
            case BB_STATE_GAMEOVER:
                if (joyNew & (BUTTON_A | BUTTON_B))
                {
                    PAL_fadeOut(0, 15, 8, FALSE);
                    VDP_drawImageEx(BG_B, &basketballtitlescreen,
                        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX),
                        0, 0, FALSE, TRUE);
                    PAL_fadeIn(0, 15, basketballtitlescreen.palette->data, 8, FALSE);
                    bbState = BB_STATE_TITLE;
                }
                break;
        }

        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_ARCADE1;
}
