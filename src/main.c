#include <genesis.h>
#include "resources.h"

// =========================================================
// Player globals
// =========================================================
int playerX = 100;
int playerY = 100;

#define COL_MAP_WIDTH   64
#define COL_MAP_HEIGHT  58

#define ADDXTOPLAYERSPRITE   -40
#define ADDYTOPLAYERSPRITE   22

#define SFX_SF1HADOKEN 64
#define SFX_SF2KENVOICE2 65
#define SFX_SF3KENSHORYUKEN 66

u8 tileContent = 0;
const u8* currentColMap;   // Active collision map

// 0=idle, 1=sitting, 2=running
enum {
    POSE_IDLE,
    POSE_SITTING,
    POSE_RUNNING
};
int playerSpritePose = POSE_SITTING; 
Sprite *playerSprite;

// =========================================================
// Room IDs
// =========================================================
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

u8 currentRoom = ROOM_SLEEPINGQUARTERS;

// =========================================================
// Collision maps table
// =========================================================
const u8* const collisionMaps[ROOM_COUNT] =
{
    level_col_arcade1,
    level_col_arcade2,
    level_col_biopod,
    level_col_hydroponicsbay,
    level_col_hangarbay,
    level_col_trainingdeck,
    level_col_reactorchamber,
    level_col_sleepingquarters,
    level_col_armoryvault,
    level_col_holodeck,
    level_col_medicalbay,
    level_col_sciencelab
};

// =========================================================
// Background rendering
// =========================================================
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
        default: return;
    }

    // Update active collision map
    currentColMap = collisionMaps[room];

    VDP_setEnable(FALSE);
    PAL_setPalette(PAL0, bg->palette->data, DMA);
    VDP_drawImageEx(
        BG_B,
        bg,
        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, ind),
        0, 0,
        FALSE,
        TRUE
    );
    VDP_setEnable(TRUE);
}

// =========================================================
// Collision query
// =========================================================
static void getTileContentPlayer(void)
{
    u16 tileX = playerX >> 2;
    u16 tileY = (playerY + 102) >> 2; // player's feet

    if(tileX < COL_MAP_WIDTH && tileY < COL_MAP_HEIGHT)
        tileContent = currentColMap[tileY * COL_MAP_WIDTH + tileX];
    else
        tileContent = 1; // treat out-of-bounds as solid
}

// =========================================================
// Input handling
// =========================================================
static void handleInput(void)
{
    static u16 prevJoy = 0;
    u16 joy = JOY_readJoypad(JOY_1);

    // ---- Room change ----
    if ((joy & BUTTON_A) && !(prevJoy & BUTTON_A))
    {
        if (currentRoom < ROOM_COUNT - 1) currentRoom++;
        else currentRoom = 0;
        drawRoomBackground(currentRoom);
    }

    if ((joy & BUTTON_B) && !(prevJoy & BUTTON_B))
    {
        if (currentRoom > 0) currentRoom--;
        else currentRoom = ROOM_COUNT - 1;
        drawRoomBackground(currentRoom);
    }

    playerSpritePose = POSE_IDLE;

    // ---- Sprite movement ----
    if (joy & BUTTON_RIGHT)
    {
        playerX += 2;
        playerSpritePose = POSE_RUNNING;        
        SPR_setHFlip(playerSprite, FALSE);

        getTileContentPlayer();
        if (tileContent == 1) 
        {
            // try top
            playerY -= 4;
            getTileContentPlayer();
            if (tileContent == 1) 
            {
                // try bottom
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

    if (joy & BUTTON_LEFT)
    {
        playerX -= 2;
        playerSpritePose = POSE_RUNNING;        
        SPR_setHFlip(playerSprite, TRUE);

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

    if (joy & BUTTON_UP)    
    {
        playerY -= 1;
        playerSpritePose = POSE_RUNNING;
        getTileContentPlayer();
        if (tileContent == 1) playerY += 1;
    }

    if (joy & BUTTON_DOWN)
    {
        playerY += 1;
        playerSpritePose = POSE_RUNNING;        
        getTileContentPlayer();
        if (tileContent == 1) playerY -= 1;
    }

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

// =========================================================
// Main
// =========================================================
int main(bool hardReset)
{
    VDP_setScreenWidth256();

    drawRoomBackground(currentRoom); // sets background + collision map

    SPR_init();
    PAL_setPalette(PAL2, playerSpriteDef.palette->data, CPU);

    playerSprite = SPR_addSprite(
        &playerSpriteDef,
        playerX, playerY,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );

    VDP_drawText("x:     y:     tile:", 0, 27);

    XGM_startPlay(my_track1);
    
    XGM_setPCM(SFX_SF1HADOKEN, sfx_hadoken, sizeof(sfx_hadoken));
    XGM_setPCM(SFX_SF2KENVOICE2, sfx_kenvoice2, sizeof(sfx_kenvoice2));
    XGM_setPCM(SFX_SF3KENSHORYUKEN, sfx_shoryuken, sizeof(sfx_shoryuken));

    while (TRUE)
    {
        handleInput();

        getTileContentPlayer();
        char text[2];
        sprintf(text, "%1u", tileContent);
        VDP_drawTextBG(BG_A, text, 19, 27);

        SPR_setAnim(playerSprite, playerSpritePose);
        SPR_setPosition(playerSprite,
            playerX + ADDXTOPLAYERSPRITE,
            playerY + ADDYTOPLAYERSPRITE);

        SPR_update();
        SYS_doVBlankProcess();
    }

    return 0;
}
