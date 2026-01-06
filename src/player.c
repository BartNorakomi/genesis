#include <genesis.h>
#include "player.h"

// ---------------------------------------------------------
// 1. Externs from other modules
// ---------------------------------------------------------
extern const u8* currentColMap;
extern u8 tileContent;

// ---------------------------------------------------------
// 2. Internal constants (module‑only)
// ---------------------------------------------------------
#define ADDXTOPLAYERSPRITE   -40
#define ADDYTOPLAYERSPRITE   22

// ---------------------------------------------------------
// 3. Player globals (owned by this module)
// ---------------------------------------------------------
int playerX = 100;
int playerY = 100;
bool playerFacingRight = true;   // default facing right

int playerSpritePose = POSE_SITTING;
Sprite *playerSprite;

// ---------------------------------------------------------
// 4. Internal helper functions
// ---------------------------------------------------------
void getTileContentPlayer(void)
{
    u16 tileX = playerX >> 2;
    u16 tileY = (playerY + 102) >> 2; // player's feet

    if (tileX < COL_MAP_WIDTH && tileY < COL_MAP_HEIGHT)
        tileContent = currentColMap[tileY * COL_MAP_WIDTH + tileX];
    else
        tileContent = 1; // treat out-of-bounds as solid
}

// ---------------------------------------------------------
// 5. Public API: sprite update
// ---------------------------------------------------------
void playerUpdateSprite(void)
{
    SPR_setAnim(playerSprite, playerSpritePose);
    SPR_setPosition(playerSprite, playerX + ADDXTOPLAYERSPRITE, playerY + ADDYTOPLAYERSPRITE);

    // Apply facing direction
    SPR_setHFlip(playerSprite, !playerFacingRight);
}


// ---------------------------------------------------------
// 6. Public API: input + movement + collision + SFX
// ---------------------------------------------------------
void playerHandleInput(void)
{
    static u16 prevJoy = 0;
    u16 joy = JOY_readJoypad(JOY_1);

    playerSpritePose = POSE_IDLE;

    // ---- Move Right ----
    if (joy & BUTTON_RIGHT)
    {
        playerX += 2;
        playerSpritePose = POSE_RUNNING;
        playerFacingRight = true;

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
    }

    // ---- Move Down ----
    if (joy & BUTTON_DOWN)
    {
        playerY += 1;
        playerSpritePose = POSE_RUNNING;
        getTileContentPlayer();
        if (tileContent == 1) playerY -= 1;
    }

    // ---- Sound effects ----
    if (joy & BUTTON_A) XGM_startPlayPCM(SFX_SF1HADOKEN, 15, SOUND_PCM_CH2);
    if (joy & BUTTON_B) XGM_startPlayPCM(SFX_SF2KENVOICE2, 15, SOUND_PCM_CH3);
    if (joy & BUTTON_C) XGM_startPlayPCM(SFX_SF3KENSHORYUKEN, 15, SOUND_PCM_CH4);

    // ---- Bounds ----
    if (playerX < 0) playerX = 0;
    if (playerX > 255) playerX = 255;
    if (playerY < 0) playerY = 0;
    if (playerY > 125) playerY = 125;

    prevJoy = joy;
}
