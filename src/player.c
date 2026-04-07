#include <genesis.h>
#include "player.h"
#include "resources.h"

// ---------------------------------------------------------
// 1. Externs from other modules
// ---------------------------------------------------------
extern const u8* currentColMap;
extern u8 tileContent;

// From main.c
extern u16 our_level_palette[64];
extern const u16* const palette_black;

// ---------------------------------------------------------
// 2. Shared sprite offsets (player + NPCs)
// ---------------------------------------------------------
int PLAYERANDNPC_OFFSET_X = -40;
int PLAYERANDNPC_OFFSET_Y = 22;

// ---------------------------------------------------------
// 3. Player globals (owned by this module)
// ---------------------------------------------------------
int playerX = 100;
int playerY = 100;
bool playerFacingRight = true;

int playerSpritePose = POSE_SITTING;
Sprite *playerSprite;

// Healing state machine
static bool playerIsHealing = false;
static u8   healingPhase = 0;
static u16  healingTimer = 0;

// ---------------------------------------------------------
// 4. Input state (centralized)
// ---------------------------------------------------------
static u16 joy     = 0;
static u16 joyPrev = 0;
static u16 joyNew  = 0;

u16 playerGetJoy(void)     { return joy; }
u16 playerGetJoyNew(void)  { return joyNew; }

// ---------------------------------------------------------
// 5. Footstep SFX helper
// ---------------------------------------------------------
static void playerHandleFootsteps(void)
{
    static int prevFrame = -1;

    int f = playerSprite->frameInd;

    if ((f == 0 || f == 5) && prevFrame != f)
    {
        XGM2_playPCM(playerfootstep, sizeof(playerfootstep), SOUND_PCM_CH1);
    }

    prevFrame = f;
}

// ---------------------------------------------------------
// 6. Internal helper functions
// ---------------------------------------------------------
void getTileContentPlayer(void)
{
    u16 tileX = playerX >> 2;
    u16 tileY = (playerY + 102) >> 2;

    if (tileX < COL_MAP_WIDTH && tileY < COL_MAP_HEIGHT)
        tileContent = currentColMap[tileY * COL_MAP_WIDTH + tileX];
    else
        tileContent = 1;
}

// ---------------------------------------------------------
// 7. Public API: sprite update
// ---------------------------------------------------------
void playerUpdateSprite(void)
{
    SPR_setAnim(playerSprite, playerSpritePose);

    SPR_setPosition(
        playerSprite,
        playerX + PLAYERANDNPC_OFFSET_X,
        playerY + PLAYERANDNPC_OFFSET_Y
    );

    SPR_setHFlip(playerSprite, !playerFacingRight);
}

// ---------------------------------------------------------
// 8. NEW: Healing API (entry point)
// ---------------------------------------------------------
void playerStartHealing(void)
{
    if (playerIsHealing) return;

    playerIsHealing = true;
    healingPhase = 0;
    healingTimer = 0;
}

// ---------------------------------------------------------
// 9. Internal: Healing state machine
// ---------------------------------------------------------
static void playerUpdateHealing(void)
{
    healingTimer++;

    switch (healingPhase)
    {
        case 0:
        {
            playerX = 66;
            playerY = 84;
            playerFacingRight = true;

            u8 frame = healingTimer >> 3;
            if (frame > 6) frame = 6;

            SPR_setAnimAndFrame(playerSprite, POSE_HEALING, frame);

            if (healingTimer >= 80)
            {
                healingPhase = 1;
                healingTimer = 0;
            }
            break;
        }

        case 1:
        {
            PAL_fadeOut(0, 63, 8, FALSE);

            if (healingTimer >= 10)
            {
                PAL_setColors(0, palette_black, 64, DMA);
                healingPhase = 2;
                healingTimer = 0;
            }
            break;
        }

        case 2:
        {
            PAL_fadeIn(0, 63, our_level_palette, 8, TRUE);

            if (healingTimer >= 10)
            {
                healingPhase = 3;
                healingTimer = 0;
            }
            break;
        }

        case 3:
        {
            u8 frame = 6 - (healingTimer >> 3);
            if (frame > 6) frame = 0;

            SPR_setAnimAndFrame(playerSprite, POSE_HEALING, frame);

            if (healingTimer >= 80)
            {
                playerX = 60;
                playerY = 90;
                playerSpritePose = POSE_IDLE;

                playerIsHealing = false;
                healingPhase = 0;
                healingTimer = 0;
            }
            break;
        }
    }
}

// ---------------------------------------------------------
// 10. Room timing helpers
// ---------------------------------------------------------
static u32 roomEnterTick = 0;

void playerMarkRoomEntry(void)
{
    roomEnterTick = getTick();
}

bool playerHasBeenInRoomFor(u32 ticks)
{
    return (getTick() - roomEnterTick >= ticks);
}

bool playerIsCenterScreen(void)
{
    return (playerX >= 80 && playerX < 176);
}

void populateControls(void)
{
    // Centralized input read
    joy = JOY_readJoypad(JOY_1);
    joyNew = joy & ~joyPrev;
    joyPrev = joy;
}

// ---------------------------------------------------------
// 11. Public API: movement + collision + SFX
// ---------------------------------------------------------
void playerHandleInput(void)
{
    // Centralized input read
    joy = JOY_readJoypad(JOY_1);
    joyNew = joy & ~joyPrev;
    joyPrev = joy;

    // If healing, ignore movement and run healing logic
    if (playerIsHealing)
    {
        playerUpdateHealing();
        return;
    }

    playerSpritePose = POSE_IDLE;

    // ---- Move Right ----
    if (joy & BUTTON_RIGHT)
    {
        playerX += 2;
        playerSpritePose = POSE_RUNNING;
        playerFacingRight = true;

        playerHandleFootsteps();

        getTileContentPlayer();
        if (tileContent == 1)
        {
            playerY -= 4;
            getTileContentPlayer();
            if (tileContent == 1)
            {
                playerY += 8;
                getTileContentPlayer();
                if (tileContent == 1)
                {
                    playerX -= 2;
                    playerY -= 4;
                }
                else playerY -= 3;
            }
            else playerY += 3;
        }
    }

    // ---- Move Left ----
    if (joy & BUTTON_LEFT)
    {
        playerX -= 2;
        playerSpritePose = POSE_RUNNING;
        playerFacingRight = false;

        playerHandleFootsteps();

        getTileContentPlayer();
        if (tileContent == 1)
        {
            playerY -= 4;
            getTileContentPlayer();
            if (tileContent == 1)
            {
                playerY += 8;
                getTileContentPlayer();
                if (tileContent == 1)
                {
                    playerX += 2;
                    playerY -= 4;
                }
                else playerY -= 3;
            }
            else playerY += 3;
        }
    }

    // ---- Move Up ----
    if (joy & BUTTON_UP)
    {
        playerY -= 1;
        playerSpritePose = POSE_RUNNING;

        getTileContentPlayer();
        if (tileContent == 1) playerY += 1;

        playerHandleFootsteps();
    }

    // ---- Move Down ----
    if (joy & BUTTON_DOWN)
    {
        playerY += 1;
        playerSpritePose = POSE_RUNNING;

        getTileContentPlayer();
        if (tileContent == 1) playerY -= 1;

        playerHandleFootsteps();
    }

    // ---- Bounds ----
    if (playerX < 0) playerX = 0;
    if (playerX > 255) playerX = 255;
    if (playerY < 0) playerY = 0;
    if (playerY > 125) playerY = 125;
}
