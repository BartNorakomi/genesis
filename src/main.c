#include <genesis.h>
#include "res/res_sprite.h"
#include "resources.h"

int playerX = 100;
int playerY = 100;

#define LEVEL_COL level_col
#define COL_MAP_WIDTH   64
#define COL_MAP_HEIGHT  58




/* =========================================================
   Room IDs
   ========================================================= */

typedef enum
{
    ROOM_ARCADE1 = 0,
    ROOM_ARCADE2,
    ROOM_BIOPOD,
    ROOM_HYDROPONICSBAY,
    ROOM_HANGARBAY,
    ROOM_TRAININGDECK,
    ROOM_REACTORCHAMBER,
    ROOM_SLEEPINGQUARTERS,
    ROOM_ARMORYVAULT,
    ROOM_HOLODECK,
    ROOM_MEDICALBAY,
    ROOM_SCIENCELAB,

    ROOM_COUNT
} RoomId;


//0=idle, 1=sitting, 2=running
enum {
    POSE_IDLE,
    POSE_SITTING,
    POSE_RUNNING
};
int playerSpritePose = POSE_SITTING; 

/* =========================================================
   Globals
   ========================================================= */

Sprite *playerSprite;



u8 currentRoom = ROOM_SLEEPINGQUARTERS;

/* =========================================================
   Background rendering
   ========================================================= */

static void drawRoomBackground(u8 room)
{
    u16 ind = TILE_USER_INDEX;
    const Image *bg = NULL;

    switch (room)
    {
        case ROOM_ARCADE1:          bg = &arcade1; break;
        case ROOM_ARCADE2:          bg = &arcade2; break;
        case ROOM_BIOPOD:           bg = &biopod; break;
        case ROOM_HYDROPONICSBAY:   bg = &hydroponicsbay; break;
        case ROOM_HANGARBAY:        bg = &hangarbay; break;
        case ROOM_TRAININGDECK:     bg = &trainingdeck; break;
        case ROOM_REACTORCHAMBER:   bg = &reactorchamber; break;
        case ROOM_SLEEPINGQUARTERS: bg = &sleepingquarters; break;
        case ROOM_ARMORYVAULT:      bg = &armoryvault; break;
        case ROOM_HOLODECK:         bg = &holodeck; break;
        case ROOM_MEDICALBAY:       bg = &medicalbay; break;
        case ROOM_SCIENCELAB:       bg = &sciencelab; break;

        default:
            return;
    }

    VDP_setEnable(FALSE); // screen off
    PAL_setPalette(PAL0, bg->palette->data, DMA);
    VDP_drawImageEx(
        BG_B,
        bg,
        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, ind),
        0, 0,
        FALSE,
        TRUE
    );
    VDP_setEnable(TRUE); // screen on
}

/* =========================================================
   Input handling
   ========================================================= */

static void handleInput(void)
{
    static u16 prevJoy = 0;
    u16 joy = JOY_readJoypad(JOY_1);

    /* ---- Room change ---- */
    if ((joy & BUTTON_A) && !(prevJoy & BUTTON_A))
    {
        if (currentRoom < ROOM_COUNT - 1)
            currentRoom++;
        else
            currentRoom = 0;

        drawRoomBackground(currentRoom);
    }

    if ((joy & BUTTON_B) && !(prevJoy & BUTTON_B))
    {
        if (currentRoom > 0)
            currentRoom--;
        else
            currentRoom = ROOM_COUNT - 1;

        drawRoomBackground(currentRoom);
    }

    playerSpritePose = POSE_IDLE;

    /* ---- Sprite movement (UNCHANGED) ---- */
    if (joy & BUTTON_RIGHT)
    {
        playerX += 2;
        playerSpritePose = POSE_RUNNING;        
        SPR_setHFlip(playerSprite, FALSE);
    }
    if (joy & BUTTON_LEFT)
    {
        playerX -= 2;
        playerSpritePose = POSE_RUNNING;        
        SPR_setHFlip(playerSprite, TRUE);
    }
    if (joy & BUTTON_UP)    
    {
        playerY -= 1;
        playerSpritePose = POSE_RUNNING;        
    }
    if (joy & BUTTON_DOWN)
    {
        playerY += 1;
        playerSpritePose = POSE_RUNNING;        
    }

    /* ---- Bounds ---- */
    if (playerX < 0) playerX = 0;
    if (playerX > 320 - 32) playerX = 320 - 32;
    if (playerY < 0) playerY = 0;
    if (playerY > 224 - 32) playerY = 224 - 32;

    prevJoy = joy;
}

/* =========================================================
   Main
   ========================================================= */

int main(bool hardReset)
{
    VDP_setScreenWidth256();

    drawRoomBackground(currentRoom);

    SPR_init();
    PAL_setPalette(PAL2, sonicSpriteDef.palette->data, CPU);

    playerSprite = SPR_addSprite(
        &sonicSpriteDef,
        playerX, playerY,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );

    VDP_drawText("x:     y:     tile:", 0, 27);
    char text_8bit[3];

    while (TRUE)
    {
        sprintf(text_8bit, "%3i", playerX);
        VDP_drawTextBG(BG_A, text_8bit,2,27);
        sprintf(text_8bit, "%3i", playerY);
        VDP_drawTextBG(BG_A, text_8bit,9,27);


        // Compute tile coordinates
        u16 tileX = (playerX+40) / 4;
        u16 tileY = (playerY+80) / 4;

        // Compute tile index
        u16 tileIndex = tileY * COL_MAP_WIDTH + tileX;

        // Get the collision value (0 or 1)
        u8 tileContent = 0;
        if(tileX < COL_MAP_WIDTH && tileY < COL_MAP_HEIGHT)  // bounds check
            tileContent = LEVEL_COL[tileIndex];

        // Print the collision value
        char text[2]; // just one digit + null terminator
        sprintf(text, "%1u", tileContent);
        VDP_drawTextBG(BG_A, text, 19, 27);

        handleInput();
        SPR_setAnim(playerSprite, playerSpritePose);
        SPR_setPosition(playerSprite, playerX, playerY);
        SPR_update();
        SYS_doVBlankProcess();
    }

    return 0;
}
