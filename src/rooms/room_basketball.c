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
    BB_STATE_GAME
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
static s16 hoopPhase = 0;

// NEW: scoring flag
static u8 basketballFirstPointScored = 0;

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
    SPR_setPosition(basketballShadowSprite, ballX, 130);
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

    hoopPhase = 5;
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

    hoopPhase = 0;
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
static void HandleBasketBallgameHud(void) {}
static void SetwallCoverUps(void) {}
static void HandleBallShadowCoverUp(void) {}
static void HandleNet(void) {}
static void HandleBasketBallGameOver(void) {}
static void HandlePickUpCoin(void) {}
static void HandleTriggerBForGameOver(void) {}

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

    basketballSprite     = SPR_addSprite(&basketBallSpriteDef, ballX, ballY, TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    basketballPoleSprite = SPR_addSprite(&basketBallPoleSpriteDef, 21, 30, TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    basketballPoleSprite2 = SPR_addSprite(&basketBallPoleSpriteDef, 204, 30, TILE_ATTR(PAL2, FALSE, FALSE, TRUE));
    basketballShadowSprite = SPR_addSprite(&basketBallShadowSpriteDef, ballX, ballY, TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    basketballNetSprite  = SPR_addSprite(&basketBallNetSpriteDef, 48, 48, TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    basketballCoinSprite = SPR_addSprite(&basketBallCoinSpriteDef, 200, 20, TILE_ATTR(PAL2, FALSE, FALSE, FALSE));

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
                HandleBasketBallGameOver();
                HandlePickUpCoin();
                HandleTriggerBForGameOver();

                if (joyNew & BUTTON_B)
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
