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
#include "room_jumpquest.h"

// #include "room_jumpquest.h"
// #include "room_basketball.h"
// #include "room_blockcannon.h"
// #include "room_bikerace.h"
// #include "room_neonhorizon.h"
// #include "room_trainingdeckgame.h"
// #include "room_drillinggame.h"


// ---------------------------------------------------------
// 1. Shared global tile index (used by ALL rooms)
// ---------------------------------------------------------
u16 globalTileIndex = TILE_USER_INDEX;

// ---------------------------------------------------------
// 2. Global engine state
// ---------------------------------------------------------
u16 our_level_palette[64];
u8 tileContent = 0;
const u8* currentColMap;        // Active collision map
u32 currentMusic = 0xFFFFFFFF;   // invalid pointer

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
    level_col_sciencelab,

    // New rooms
    NULL,   // ROOM_JUMPQUEST
    NULL,   // ROOM_BASKETBALL
    NULL,   // ROOM_BLOCKCANNON
    NULL,   // ROOM_BIKERACE
    NULL,   // ROOM_NEONHORIZON
    NULL,   // ROOM_TRAININGDECKGAME
    NULL    // ROOM_DRILLINGGAME
};


// ---------------------------------------------------------
// 4. Background rendering
// ---------------------------------------------------------
void drawRoomBackground(u8 room)
{
    // Reset tile index for each room load
    globalTileIndex = TILE_USER_INDEX;

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

        // --- New rooms ---
        case ROOM_JUMPQUEST:        bg = &jumpquesttitlescreen; break;
        case ROOM_BASKETBALL:       bg = &basketballtitlescreen; break;
        case ROOM_BLOCKCANNON:      bg = &blockcannontitlescreen; break;
        case ROOM_BIKERACE:         bg = &bikeracetitlescreen; break;
        case ROOM_NEONHORIZON:      bg = &neonhorizon; break;
        case ROOM_TRAININGDECKGAME: bg = &trainingdeckgame; break;
        case ROOM_DRILLINGGAME:     bg = &drillinggame; break;

        default:
            return;
    }

    // Update active collision map
    currentColMap = collisionMaps[room];

    // Fade out current room
    PAL_fadeOut(0, 63, 8, FALSE);
    
    VDP_setEnable(FALSE);
    VDP_clearPlane(BG_A, TRUE);
    PAL_setPalette(PAL0, bg->palette->data, DMA);

    // Draw background using global tile index
    VDP_drawImageEx(
        BG_B,
        bg,
        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, globalTileIndex),
        0, 0,
        FALSE,
        TRUE
    );

    // Advance tile index by background tileset size
    globalTileIndex += bg->tileset->numTile;

    // Build palette 0
    memcpy(&our_level_palette[0], bg->palette->data, 16 * 2);
    // Build palette 1
    if (room == ROOM_HYDROPONICSBAY) memcpy(&our_level_palette[16], hydroponicsbay_fg.palette->data, 16 * 2);
    if (room == ROOM_JUMPQUEST) memcpy(&our_level_palette[16], arcademachine.palette->data, 16 * 2);
    if (room == ROOM_BASKETBALL) memcpy(&our_level_palette[16], arcademachine.palette->data, 16 * 2);
    if (room == ROOM_BLOCKCANNON) memcpy(&our_level_palette[16], arcademachine.palette->data, 16 * 2);
    if (room == ROOM_BIKERACE) memcpy(&our_level_palette[16], arcademachine.palette->data, 16 * 2);
    // Build palette 2
    memcpy(&our_level_palette[32], playerSpriteDef.palette->data, 16 * 2);
    // Build palette 3
    if (room == ROOM_REACTORCHAMBER) memcpy(&our_level_palette[48], reactorSpriteDef.palette->data, 16 * 2);
    if (room == ROOM_MEDICALBAY) memcpy(&our_level_palette[48], medicalBayChairSpriteDef.palette->data, 16 * 2);
    if (room == ROOM_TRAININGDECK) memcpy(&our_level_palette[48], trainingDeckTreadmillSpriteDef.palette->data, 16 * 2);
    if (room == ROOM_HANGARBAY) memcpy(&our_level_palette[48], hangarBayDrillingMachineSpriteDef.palette->data, 16 * 2);
    if (room == ROOM_SCIENCELAB) memcpy(&our_level_palette[48], scienceLabHelixSpriteDef.palette->data, 16 * 2);
    if (room == ROOM_BIOPOD) memcpy(&our_level_palette[48], biopodRightPodSpriteDef.palette->data, 16 * 2);
    if (room == ROOM_HYDROPONICSBAY) memcpy(&our_level_palette[48], hydroponicsBayFoodLeftSpriteDef.palette->data, 16 * 2);
    if (room == ROOM_HOLODECK) memcpy(&our_level_palette[48], holodeckDoorSpriteDef.palette->data, 16 * 2);
    if (room == ROOM_ARCADE2) memcpy(&our_level_palette[48], arcade2TableSpriteDef.palette->data, 16 * 2);
     
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

// ---------------------------------------------------------
// 6. Music
// ---------------------------------------------------------
void playMusic(const u8* track)
{
    if (currentMusic != (u32)track)
    {
        XGM2_play(track);
        currentMusic = (u32)track;
    }
}

// ---------------------------------------------------------
// 7. Main entry point
// ---------------------------------------------------------
int main(bool hardReset)
{
    VDP_setScreenWidth256();
    SPR_init();
    VDP_drawText("x:     y:     tile:", 0, 27);

    GameState state = STATE_MEDICALBAY;

    while (state != STATE_QUIT)
    {
        switch (state)
        {
            case STATE_TITLE:            state = runTitleScreen(); break;
            case STATE_ARCADE1:          state = runArcade1(); break;
            case STATE_ARCADE2:          state = runArcade2(); break;
            case STATE_BIOPOD:           state = runBioPod(); break;
            case STATE_HYDROPONICSBAY:   state = runHydroponicsBay(); break;
            case STATE_HANGARBAY:        state = runHangarBay(); break;
            case STATE_TRAININGDECK:     state = runTrainingDeck(); break;
            case STATE_REACTORCHAMBER:   state = runReactorChamber(); break;
            case STATE_SLEEPINGQUARTERS: state = runSleepingQuarters(); break;
            case STATE_ARMORYVAULT:      state = runArmoryVault(); break;
            case STATE_HOLODECK:         state = runHoloDeck(); break;
            case STATE_MEDICALBAY:       state = runMedicalBay(); break;
            case STATE_SCIENCELAB:       state = runScienceLab(); break;

            // --- New rooms below ---
            case STATE_JUMPQUEST:        state = runJumpQuest(); break;
            case STATE_BASKETBALL:       state = runBasketball(); break;
            case STATE_BLOCKCANNON:      state = runBlockCannon(); break;
            case STATE_BIKERACE:         state = runBikeRace(); break;
            case STATE_NEONHORIZON:      state = runNeonHorizon(); break;
            case STATE_TRAININGDECKGAME: state = runTrainingDeckGame(); break;
            case STATE_DRILLINGGAME:     state = runDrillingGame(); break;

            default:
                state = STATE_QUIT;
                break;
        }
    }

    return 0;
}
