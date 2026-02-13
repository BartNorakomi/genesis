#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_blockcannon.h"
#include "player.h"
#include "game_state.h"
#include "room_sciencelab.h"
#include "room_arcade1.h"

// ---------------------------------------------------------
// 1. Externs from other modules
// ---------------------------------------------------------
extern const u8* currentColMap;
extern u8 tileContent;

extern void drawRoomBackground(u8 room);
extern void drawDebugInfo(void);

// ---------------------------------------------------------
// 5. Room logic
// ---------------------------------------------------------
GameState runBlockCannon(void)
{
    drawRoomBackground(ROOM_BLOCKCANNON);
    playMusic(tune_ship);
    SPR_reset();

    while (1)
    {
        playerHandleInput();

        // Debug + sprite update
        drawDebugInfo();
        playerUpdateSprite();

        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_QUIT;
}
