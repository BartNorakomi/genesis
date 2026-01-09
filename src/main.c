#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "game_state.h"
#include "player.h"
#include "room_sleepingquarters.h"
#include "room_arcade1.h"
#include "room_titlescreen.h"
#include "room_medicalbay.h"


// ---------------------------------------------------------
// 2. Global engine state
// ---------------------------------------------------------
u16 our_level_palette[64];
u8 tileContent = 0;
const u8* currentColMap;        // Active collision map
u32 currentMusic = 0xFFFFFFFF;   // invalid pointer

//order rooms:  science lab, hangar bay, training deck, sleeping quarters,medical bay, hydroponics bay, armory vault, bio pod, reactor chamber, holodeck 

// ---------------------------------------------------------
// 3. Collision map table
// ---------------------------------------------------------
const u8* const collisionMaps[ROOM_COUNT] =
{
    NULL,                   // ROOM_TITLESCREEN
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

// ---------------------------------------------------------
// 4. Background rendering
// ---------------------------------------------------------
void drawRoomBackground(u8 room)
{
    u16 ind = TILE_USER_INDEX;
    const Image *bg = NULL;

    switch (room)
    {
        case ROOM_TITLESCREEN:      bg = &titlescreen; break;
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

    // Fade out current room
    PAL_fadeOut(0, 63, 8, FALSE);

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

    memcpy(&our_level_palette[0], bg->palette->data, 16 * 2);
    memcpy(&our_level_palette[32], playerSpriteDef.palette->data, 16 * 2);

    PAL_setColors(0, palette_black, 64, DMA);

    VDP_setEnable(TRUE);
    PAL_fadeIn(0, 63, our_level_palette, 8, TRUE);
}

// ---------------------------------------------------------
// 5. Debug info
// ---------------------------------------------------------
void drawDebugInfo(void)
{
    char buf[4];

    sprintf(buf, "%u", tileContent);
    VDP_drawTextBG(BG_A, buf, 19, 27);

    sprintf(buf, "%3i", playerX);
    VDP_drawTextBG(BG_A, buf, 2, 27);

    sprintf(buf, "%3i", playerY);
    VDP_drawTextBG(BG_A, buf, 9, 27);
}

void playMusic(const u8* track)
{
    if (currentMusic != (u32)track)
    {
        //XGM_startPlay(track); // old
        XGM2_play(track); // new
        currentMusic = (u32)track;
    }
}

// ---------------------------------------------------------
// 6. Main entry point
// ---------------------------------------------------------
int main(bool hardReset)
{
    VDP_setScreenWidth256();
    SPR_init();
    VDP_drawText("x:     y:     tile:", 0, 27);
//    XGM_setPCM(SFX_HADOKEN,    sfx_hadoken,    sizeof(sfx_hadoken));
 //   XGM_setPCM(SFX_KENVOICE2,  sfx_kenvoice2,  sizeof(sfx_kenvoice2));
  //  XGM_setPCM(SFX_SHORYUKEN,  sfx_shoryuken,  sizeof(sfx_shoryuken));

    GameState state = STATE_ARCADE1;

    while (state != STATE_QUIT)
    {
        switch (state)
        {
            case STATE_TITLE:
                state = runTitleScreen();
                break;

            case STATE_ARCADE1:
                state = runArcade1();
                break;

            case STATE_ARCADE2:
                state = runArcade2();
                break;

            case STATE_BIOPOD:
                state = runBioPod(); 
                break;

            case STATE_HYDROPONICSBAY:
                state = runHydroponicsBay(); 
                break;

            case STATE_HANGARBAY:
                state = runHangarBay(); 
                break;

            case STATE_TRAININGDECK:
                state = runTrainingDeck(); 
                break;
                
            case STATE_REACTORCHAMBER:
                state = runReactorChamber(); 
                break;

            case STATE_SLEEPINGQUARTERS:
                state = runSleepingQuarters();
                break;

            case STATE_ARMORYVAULT:
                state = runArmoryVault(); 
                break;
                
            case STATE_HOLODECK:
                state = runHoloDeck(); 
                break;

            case STATE_MEDICALBAY:
                state = runMedicalBay(); 
                break;

            case STATE_SCIENCELAB:
                state = runScienceLab(); 
                break;

        }
    }

    return 0;
}
