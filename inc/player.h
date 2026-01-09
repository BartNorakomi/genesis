#ifndef PLAYER_H
#define PLAYER_H

// ---------------------------------------------------------
// 1. Includes (must come before anything that uses them)
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
    POSE_RUNNING
} PlayerPose;

// ---------------------------------------------------------
// 4. Extern variables (owned by player.c)
// ---------------------------------------------------------
extern int playerX;
extern int playerY;
extern int playerSpritePose;
extern Sprite *playerSprite;
extern u8 playerFacing;   // 0 = right, 1 = left

extern int PLAYERANDNPC_OFFSET_X;
extern int PLAYERANDNPC_OFFSET_Y;

// ---------------------------------------------------------
// 5. Function prototypes
// ---------------------------------------------------------
void playerUpdateSprite(void);
void playerHandleInput(void);
void getTileContentPlayer(void);

#endif
