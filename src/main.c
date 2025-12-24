#include <genesis.h>
#include "res/res_sprite.h"
#include "resources.h"

Sprite *primarySprite;

int x = 100;
int y = 100;

// Track which background is currently displayed
u8 currentBG = 0; // 0 = arcade1, 1 = arcade2

int main(bool hardReset)
{
    VDP_setScreenWidth320();

    u16 ind = TILE_USER_INDEX;

    // --- Draw initial background ---
    PAL_setPalette(PAL0, arcade1.palette->data, DMA);
    VDP_drawImageEx(
        BG_B,
        &arcade1,
        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, ind),
        0, 0,
        FALSE,
        TRUE
    );

    SPR_init();

    PAL_setPalette(PAL2, sonicSpriteDef.palette->data, CPU);
    
    primarySprite = SPR_addSprite(
        &sonicSpriteDef,
        x, y,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );

    SPR_setAnim(primarySprite, 0);

    // Track previous A button state to detect presses
    u16 prevJoy = 0;

    while (TRUE)
    {
        u16 joy = JOY_readJoypad(JOY_1);

        // --- Background toggle on A press ---
        if ((joy & BUTTON_A) && !(prevJoy & BUTTON_A))
        {
            // Flip background
            if (currentBG == 0)
            {
                PAL_setPalette(PAL0, arcade2.palette->data, DMA);
                VDP_drawImageEx(BG_B, &arcade2, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, ind), 0, 0, FALSE, TRUE);
                currentBG = 1;
            }
            else
            {
                PAL_setPalette(PAL0, arcade1.palette->data, DMA);
                VDP_drawImageEx(BG_B, &arcade1, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, ind), 0, 0, FALSE, TRUE);
                currentBG = 0;
            }
        }

        // --- Movement ---
        if (joy & BUTTON_RIGHT)
        {
            x += 2;
            SPR_setHFlip(primarySprite, FALSE);
        }
        if (joy & BUTTON_LEFT)
        {
            x -= 2;
            SPR_setHFlip(primarySprite, TRUE);
        }
        if (joy & BUTTON_UP) y -= 2;
        if (joy & BUTTON_DOWN) y += 2;

        // --- Keep sprite inside screen bounds ---
        if (x < 0) x = 0;
        if (x > 320 - 32) x = 320 - 32;
        if (y < 0) y = 0;
        if (y > 224 - 32) y = 224 - 32;

        // --- Update sprite ---
        SPR_setPosition(primarySprite, x, y);
        SPR_update();
        SYS_doVBlankProcess();

        prevJoy = joy; // save previous state
    }

    return 0;
}
