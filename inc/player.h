#ifndef PLAYER_H
#define PLAYER_H

// ---------------------------------------------------------
// 1. Includes
// ---------------------------------------------------------
#include <genesis.h>

// ---------------------------------------------------------
// 2. Constants / Defines
// ---------------------------------------------------------
#define COL_MAP_WIDTH       64
#define COL_MAP_HEIGHT      58

// ---------------------------------------------------------
// 3. Types (enums, structs, typedefs)
// ---------------------------------------------------------
typedef enum
{
    POSE_IDLE,
    POSE_SITTING,
    POSE_RUNNING,
    POSE_HEALING
} PlayerPose;

// ---------------------------------------------------------
// 4. Extern variables (owned by player.c)
// ---------------------------------------------------------
extern int playerX;
extern int playerY;
extern int playerSpritePose;
extern Sprite *playerSprite;

extern int PLAYERANDNPC_OFFSET_X;
extern int PLAYERANDNPC_OFFSET_Y;

// ---------------------------------------------------------
// 5. Centralized Input API
// ---------------------------------------------------------
// NOTE: playerUpdateInput() was removed — do NOT declare it.
u16 playerGetJoy(void);
u16 playerGetJoyNew(void);

// ---------------------------------------------------------
// 6. Player logic API
// ---------------------------------------------------------
void playerHandleInput(void);
void playerUpdateSprite(void);
void getTileContentPlayer(void);

bool playerIsCenterScreen(void);
void playerMarkRoomEntry(void);
bool playerHasBeenInRoomFor(u32 ticks);

// Healing trigger
void playerStartHealing(void);

#endif
