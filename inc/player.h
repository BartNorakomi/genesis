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
    POSE_HEALING      // NEW pose for Medical Bay healing
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
// 5. Function prototypes
// ---------------------------------------------------------
void playerUpdateSprite(void);
void playerHandleInput(void);
void getTileContentPlayer(void);
bool playerIsCenterScreen(void);
void playerMarkRoomEntry(void);
bool playerHasBeenInRoomFor(u32 ticks);

// NEW: healing trigger
void playerStartHealing(void);

#endif
