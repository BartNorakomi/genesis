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
static Sprite* basketballNetSprite;
static Sprite* basketballShadowSprite;
static Sprite* basketballCoinSprite;

// ---------------------------------------------------------
// 4. Ball physics variables
// ---------------------------------------------------------
static s16 ballX = 120;
static s16 ballY = 111;
static s16 ballVertSpeed = 0;     // (iy+var1)
static u16 framecounter2 = 0;

// ---------------------------------------------------------
// 5. Subroutine stubs (converted from 68k structure)
// ---------------------------------------------------------

// --- Bounce helpers ---
static void CheckBounceOnRightRimLeftSide(void) {}
static void CheckBounceOnRightBackboard(void) {}
static void CheckBounceOnRightRimRightSide(void) {}

static void CheckBounceOnLeftRimLeftSide(void) {}
static void CheckBounceOnLeftBackboard(void) {}
static void CheckBounceOnLeftRimRightSide(void) {}

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
// ApplyGravity (C version of .ApplyGravity)
// ---------------------------------------------------------
static void ApplyGravity(void)
{
    if ((framecounter2 & 3) != 0)
        return;

    ballVertSpeed++;   // gravity
}

// ---------------------------------------------------------
// MoveBallVertically (C version of .MoveBallVertically)
// ---------------------------------------------------------
static void MoveBallVertically(void)
{
    // Moving UP (negative speed)
    if (ballVertSpeed < 0)
    {
        ballY += ballVertSpeed;

        if (ballY < 103)
            return;

        ballY = 103;
        return;
    }

    // Moving DOWN (positive speed)
    ballY += ballVertSpeed;

    if (ballY >= 0)
        return;

    // Hit the floor
    ballY = 255;

    ballVertSpeed--;

    if (ballVertSpeed < 0)
        return;

    // Bounce
    ballVertSpeed = -ballVertSpeed;
    framecounter2 = 2;
}

// ---------------------------------------------------------
// SetXYinSpat (updates sprite positions)
// ---------------------------------------------------------
static void SetXYinSpat(void)
{
    SPR_setPosition(basketballSprite, ballX, ballY);
    SPR_setPosition(basketballShadowSprite, ballX, ballY + 20);
}

// ---------------------------------------------------------
// Other routines (stubs for now)
// ---------------------------------------------------------
static void HandleTrigA(void) {}
static void SetHorizontalSpeed(void) {}
static void MoveBallHorizontally(void) {}
static void CheckScore(void) {}
static void SetBallShadow(void) {}   // <-- MUST be here, before HandleBasketBall()

static void HandlePhase(void) {}
static void HandleBasketBallgameHud(void) {}
static void SetwallCoverUps(void) {}
static void HandleBallShadowCoverUp(void) {}
static void HandleNet(void) {}
static void HandleBasketBallGameOver(void) {}
static void HandlePickUpCoin(void) {}
static void HandleTriggerBForGameOver(void) {}

// ---------------------------------------------------------
// Main ball routine (converted from 68k)
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

    CheckBounceRightRimAndBackboard();
    CheckBounceLeftRimAndBackboard();
}

// ---------------------------------------------------------
// 6. Room logic (Basketball minigame)
// ---------------------------------------------------------
GameState runBasketball(void)
{
    drawRoomBackground(ROOM_BASKETBALL);
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

    // Draw game screen
    VDP_drawImageEx(
        BG_B,
        &basketballingameexample,
        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX),
        0, 0,
        FALSE,
        TRUE
    );

    PAL_fadeIn(0, 15, basketballingameexample.palette->data, 8, FALSE);

    // -----------------------------------------------------
    // Create basketball pole (left side of court)
    // -----------------------------------------------------
    basketballPoleSprite = SPR_addSprite(
        &basketBallPoleSpriteDef,
        20, 31,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );

    // -----------------------------------------------------
    // Create basketball sprite (using ballX, ballY)
    // -----------------------------------------------------
    basketballSprite = SPR_addSprite(
        &basketBallSpriteDef,
        ballX, ballY,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );

    // -----------------------------------------------------
    // Create basketball shadow
    // -----------------------------------------------------
    basketballShadowSprite = SPR_addSprite(
        &basketBallShadowSpriteDef,
        ballX, ballY + 20,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );

    // -----------------------------------------------------
    // Create basketball net
    // -----------------------------------------------------
    basketballNetSprite = SPR_addSprite(
        &basketBallNetSpriteDef,
        48, 48,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );

    // -----------------------------------------------------
    // Create basketball coin
    // -----------------------------------------------------
    basketballCoinSprite = SPR_addSprite(
        &basketBallCoinSpriteDef,
        200, 20,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );

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

                    VDP_drawImageEx(
                        BG_B,
                        &basketballingameexample,
                        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX),
                        0, 0,
                        FALSE,
                        TRUE
                    );

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

                    VDP_drawImageEx(
                        BG_B,
                        &basketballtitlescreen,
                        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX),
                        0, 0,
                        FALSE,
                        TRUE
                    );

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
