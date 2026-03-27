#include <genesis.h>
#include <string.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_bikerace.h"
#include "game_state.h"
#include "room_arcade1.h"
#include "player.h"

// ---------------------------------------------------------
// 1. Externs from other modules
// ---------------------------------------------------------
extern const u8* currentColMap;
extern u8 tileContent;

extern void drawRoomBackground(u8 room);
extern void drawDebugInfo(void);

// Shared global tile index from main.c
extern u16 globalTileIndex;

// Penguin sprite sheet (48 frames)
extern const SpriteDefinition penguinSpriteDef;

// ---------------------------------------------------------
// 2. Internal Bike Race states
// ---------------------------------------------------------
typedef enum
{
    BR_STATE_TITLE,
    BR_STATE_GAME
} BikeRaceState;

// ---------------------------------------------------------
// 3. Penguin game globals
// ---------------------------------------------------------
static u16 framecounter2 = 0;
static u8 PenguinGameRandomValue = 0;

static u8 SpawnFrequencyPizza = 0;
static u8 SpawnFrequencyExtraTime = 0;

static u16 PenguinDistance = 0;
static u8 PenguinMaxSpeed = 30;
static u8 PenguinInsideOval = 0;
static u8 PenguinDistanceTravelledThisFrame = 0;
static u8 PenguinSpeed = 0;
static u8 PenguinInvulnerable = 0;
static u8 PenguinGameTimeExtended = 0;
static u8 AddedPizzaSpeedBoost = 1;

static u8 PenguinGameLevel = 1;
static u8 PenguinGameLaps = 1;
static u8 PenguinGameLapsCopy = 1;
static u8 PenguinGameTime = 61;

// Penguin position
static s16 penguinX = 30;
static s16 penguinY = 10;

// ---------------------------------------------------------
// 4. Penguin game object struct
// ---------------------------------------------------------
typedef struct
{
    u8  on;
    u8  duration;
    u16 distance;
    u8  insideOval;
    s16 ySpat;
    s16 xSpat;
} PenguinObject;

// ---------------------------------------------------------
// 5. Penguin game objects
// ---------------------------------------------------------
static PenguinObject WarningObj;
static PenguinObject StarObj;
static PenguinObject PizzaObj;
static PenguinObject ExtraTimeObj;
static PenguinObject Stone1Obj;
static PenguinObject Stone2Obj;
static PenguinObject Stone3Obj;
static PenguinObject Stone4Obj;
static PenguinObject Mushroom1Obj;
static PenguinObject Mushroom2Obj;
static PenguinObject Spike1Obj;

// ---------------------------------------------------------
// 6. Penguin sprite + curvature tables (frame indices)
// ---------------------------------------------------------
static Sprite* penguinSprite;

static const u8 PenguinCurvatureTableOutsideOval[] =
{
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,

    1,2,3,4,5,6,7,8,
    9,10,11,

    12,12,12,12,12,12,12,12,
    12,12,12,12,12,12,12,12,

    13,14,15,16,17,18,19,20,
    21,22,23,

    0
};

static const u8 PenguinCurvatureTableInsideOval[] =
{
    24,24,24,24,24,24,24,24,
    24,24,24,24,24,24,24,24,24,

    25,26,27,28,29,30,31,32,
    33,34,35,

    36,36,36,36,36,36,36,36,
    36,36,36,36,36,36,36,36,

    37,38,39,40,41,42,43,44,
    45,46,47,

    24
};

static const s16 CoordinateTablePenguin[440][2] =
{
    // TopStraightSegment (130)
    {64,-22},{65,-22},{66,-22},{67,-22},{68,-22},{69,-22},{70,-22},{71,-22},{72,-22},{73,-22},
    {74,-22},{75,-22},{76,-22},{77,-22},{78,-22},{79,-22},{80,-22},{81,-22},{82,-22},{83,-22},
    {84,-22},{85,-22},{86,-22},{87,-22},{88,-22},{89,-22},{90,-22},{91,-22},{92,-22},{93,-22},
    {94,-22},{95,-22},{96,-22},{97,-22},{98,-22},{99,-22},{100,-22},{101,-22},{102,-22},{103,-22},
    {104,-22},{105,-22},{106,-22},{107,-22},{108,-22},{109,-22},{110,-22},{111,-22},{112,-22},{113,-22},
    {114,-22},{115,-22},{116,-22},{117,-22},{118,-22},{119,-22},{120,-22},{121,-22},{122,-22},{123,-22},
    {124,-22},{125,-22},{126,-22},{127,-22},{128,-22},{129,-22},{130,-22},{131,-22},{132,-22},{133,-22},
    {134,-22},{135,-22},{136,-22},{137,-22},{138,-22},{139,-22},{140,-22},{141,-22},{142,-22},{143,-22},
    {144,-22},{145,-22},{146,-22},{147,-22},{148,-22},{149,-22},{150,-22},{151,-22},{152,-22},{153,-22},
    {154,-22},{155,-22},{156,-22},{157,-22},{158,-22},{159,-22},{160,-22},{161,-22},{162,-22},{163,-22},
    {164,-22},{165,-22},{166,-22},{167,-22},{168,-22},{169,-22},{170,-22},{171,-22},{172,-22},{173,-22},
    {174,-22},{175,-22},{176,-22},{177,-22},{178,-22},{179,-22},{180,-22},{181,-22},{182,-22},{183,-22},
    {184,-22},{185,-22},{186,-22},{187,-22},{188,-22},{189,-22},{190,-22},{191,-22},{192,-22},{193,-22},

    // RightCurve (90)
    {194,-22},{195,-22},{196,-22},{197,-22},{198,-22},{199,-22},{200,-21},{201,-21},{202,-21},{203,-21},
    {204,-20},{205,-20},{206,-20},{207,-19},{208,-18},{209,-18},{210,-17},{211,-17},{212,-16},{213,-16},
    {213,-15},{214,-14},{215,-14},{216,-13},{216,-12},{217,-11},{218,-10},{219,-9},{219,-8},{220,-7},
    {220,-6},{221,-6},{221,-5},{222,-4},{222,-3},{222,-2},{223,-1},{223,0},{223,1},{223,2},
    {224,3},{224,4},{224,5},{224,6},{224,7},{224,8},{224,9},{224,10},{224,11},{224,12},
    {224,13},{223,14},{223,15},{223,16},{223,17},{222,18},{222,19},{222,20},{221,20},{221,21},
    {220,22},{220,23},{219,24},{219,25},{218,26},{217,27},{216,28},{216,29},{215,29},{214,30},
    {214,31},{213,31},{212,32},{211,33},{210,33},{209,34},{208,35},{207,35},{206,36},{205,36},
    {204,36},{203,37},{202,37},{201,37},{200,37},{199,38},{198,38},{197,38},{196,38},{195,38},

    // BottomStraightSegment (130)
    {194,38},{193,38},{192,38},{191,38},{190,38},{189,38},{188,38},{187,38},{186,38},{185,38},
    {184,38},{183,38},{182,38},{181,38},{180,38},{179,38},{178,38},{177,38},{176,38},{175,38},
    {174,38},{173,38},{172,38},{171,38},{170,38},{169,38},{168,38},{167,38},{166,38},{165,38},
    {164,38},{163,38},{162,38},{161,38},{160,38},{159,38},{158,38},{157,38},{156,38},{155,38},
    {154,38},{153,38},{152,38},{151,38},{150,38},{149,38},{148,38},{147,38},{146,38},{145,38},
    {144,38},{143,38},{142,38},{141,38},{140,38},{139,38},{138,38},{137,38},{136,38},{135,38},
    {134,38},{133,38},{132,38},{131,38},{130,38},{129,38},{128,38},{127,38},{126,38},{125,38},
    {124,38},{123,38},{122,38},{121,38},{120,38},{119,38},{118,38},{117,38},{116,38},{115,38},
    {114,38},{113,38},{112,38},{111,38},{110,38},{109,38},{108,38},{107,38},{106,38},{105,38},
    {104,38},{103,38},{102,38},{101,38},{100,38},{99,38},{98,38},{97,38},{96,38},{95,38},
    {94,38},{93,38},{92,38},{91,38},{90,38},{89,38},{88,38},{87,38},{86,38},{85,38},
    {84,38},{83,38},{82,38},{81,38},{80,38},{79,38},{78,38},{77,38},{76,38},{75,38},
    {74,38},{73,38},{72,38},{71,38},{70,38},{69,38},{68,38},{67,38},{66,38},{65,38},

    // LeftCurve (90)
    {64,38},{63,38},{62,38},{61,38},{60,38},{59,38},{58,37},{57,37},{56,37},{55,37},
    {54,36},{53,36},{52,36},{51,35},{50,35},{49,34},{48,33},{47,33},{46,32},{45,31},
    {44,31},{44,30},{43,29},{42,29},{42,28},{41,27},{40,26},{39,25},{39,24},{38,23},
    {38,22},{37,21},{37,20},{36,20},{36,19},{36,18},{35,17},{35,16},{35,15},{35,14},
    {34,13},{34,12},{34,11},{34,10},{34,9},{34,8},{34,7},{34,6},{34,5},{34,4},
    {34,3},{35,2},{35,1},{35,0},{35,-1},{36,-2},{36,-3},{36,-4},{37,-5},{37,-6},
    {38,-6},{38,-7},{39,-8},{39,-9},{40,-10},{41,-11},{42,-12},{42,-13},{43,-14},{44,-14},
    {45,-15},{45,-16},{46,-16},{47,-17},{48,-17},{49,-18},{50,-18},{51,-19},{52,-20},{53,-20},
    {54,-20},{55,-21},{56,-21},{57,-21},{58,-21},{59,-22},{60,-22},{61,-22},{62,-22},{63,-22}
};


// ---------------------------------------------------------
// 7. Penguin game routine stubs
// ---------------------------------------------------------
static void HandlePhase(void) {}

static void IncreasePenguinSpeed(void)
{
    // Run only every 8 frames (framecounter2 & 7 == 0)
    if (framecounter2 & 7)
        return;

    // If time > 0 → accelerate
    if (PenguinGameTime > 0)
    {
        u8 max = PenguinMaxSpeed;
        u8 spd = PenguinSpeed;

        spd++;              // inc a
        if (spd == max)     // cp b / ret z
            return;

        PenguinSpeed = spd; // ld (PenguinSpeed),a
        return;
    }

    // If time == 0 → decelerate
    {
        u8 spd = PenguinSpeed;

        if (spd == 0)       // ret m (if negative, but unsigned → check zero)
            return;

        spd--;              // dec a
        PenguinSpeed = spd;
    }
}

#define TOTAL_TRACK_DISTANCE 440

static void IncreaseDistanceAndSetXYPenguin(void)
{
    // 1. Pizza speed boost countdown
    if (AddedPizzaSpeedBoost > 1)
        AddedPizzaSpeedBoost--;

    u8 d = AddedPizzaSpeedBoost >> 2;

    // 2. Speed this frame
    u8 spd = PenguinSpeed + d;
    u8 distanceThisFrame = 0;

    // 3. Fractional accumulator
    u8 frac = PenguinDistanceTravelledThisFrame;
    frac += spd;

    // 4. Convert fractional to whole steps of 17
    while (frac >= 17)
    {
        frac -= 17;
        distanceThisFrame++;
    }

    PenguinDistanceTravelledThisFrame = frac;

    // 5. Add to PenguinDistance
    u16 dist = PenguinDistance;
    dist += distanceThisFrame;

    // 6. Lap check
    if (dist >= TOTAL_TRACK_DISTANCE)
    {
        dist -= TOTAL_TRACK_DISTANCE;

        PenguinGameLaps++;

        if (PenguinGameLaps == 11)
        {
            PenguinGameLaps = 1;
            PenguinGameLevel++;
            PenguinMaxSpeed += 2;

            s8 t = 36 - (PenguinGameLevel * 2);
            if (t < 0) t = 0;
            PenguinGameTimeExtended = t;
        }
    }

    PenguinDistance = dist;

    char buf[8];
    sprintf(buf, "%3d", PenguinDistance);
    VDP_drawTextBG(BG_A, buf, 25, 27);

    // 7. Lookup coordinates
    penguinX = CoordinateTablePenguin[dist][0];
    penguinY = CoordinateTablePenguin[dist][1];
}

static void HandleJumpInsideOrOutsideOval(void) {}
static void HandlePenguinGameObjects(void) {}
static void HandlePenguinGameHud(void) {}
static void HandlePenguinGameOver(void) {}
static void PutNewWarningObjects(void) {}
static void PutNewPizzaObjects(void) {}
static void PutNewStarObjects(void) {}
static void PutNewExtraTimeObjects(void) {}

static void SetPenguinSprite(void)
{
    const u8* table =
        (PenguinInsideOval ? PenguinCurvatureTableInsideOval
                           : PenguinCurvatureTableOutsideOval);

    // distance / 4, clear bit 0
    u16 index = (PenguinDistance >> 3);
 
    char buf[8];
    sprintf(buf, "%3d", index);
    VDP_drawTextBG(BG_A, buf, 13, 27);

    // // 🔥 SAFE bounds (important)
    // u16 tableSize = sizeof(PenguinCurvatureTableOutsideOval);

    // if (index >= tableSize)
    //     index %= tableSize;

    // Get frame index
    u8 frame = table[index];

    char buf2[8];
    sprintf(buf2, "%3d", frame);
    VDP_drawTextBG(BG_A, buf2, 18, 27);


    // Apply frame
    SPR_setFrame(penguinSprite, frame);

    // Invulnerability flicker
    if (PenguinInvulnerable)
    {
        PenguinInvulnerable--;

        if (PenguinInvulnerable & 1)
            SPR_setVisibility(penguinSprite, HIDDEN);
        else
            SPR_setVisibility(penguinSprite, VISIBLE);
    }
    else
    {
        SPR_setVisibility(penguinSprite, VISIBLE);
    }

    // Update sprite position
    SPR_setPosition(penguinSprite, penguinX - 78, penguinY);
}

// ---------------------------------------------------------
// 9. ResetVariablesPenguinRace (Z80 port)
// ---------------------------------------------------------
static void ResetVariablesPenguinRace(void)
{
    SpawnFrequencyPizza = random() & 0xFF;
    SpawnFrequencyExtraTime = 255;

    PenguinDistance = 0;
    PenguinMaxSpeed = 30;

    PenguinInsideOval = 0;
    PenguinDistanceTravelledThisFrame = 0;
    PenguinSpeed = 0;
    framecounter2 = 0;
    PenguinInvulnerable = 0;
    PenguinGameTimeExtended = 0;

    AddedPizzaSpeedBoost = 1;

    WarningObj.on         = 0;
    WarningObj.duration   = 50;
    WarningObj.distance   = 150;
    WarningObj.insideOval = 1;

    StarObj.on         = 0;
    StarObj.duration   = 50;
    StarObj.distance   = 150;
    StarObj.insideOval = 1;

    PizzaObj.on         = 0;
    PizzaObj.duration   = 50;
    PizzaObj.distance   = 100;
    PizzaObj.insideOval = 0;

    ExtraTimeObj.on         = 0;
    ExtraTimeObj.duration   = 50;
    ExtraTimeObj.distance   = 100;
    ExtraTimeObj.insideOval = 0;

    Stone1Obj.on         = 0;
    Stone1Obj.duration   = 100;
    Stone1Obj.distance   = 20;
    Stone1Obj.insideOval = 1;

    Stone2Obj.on         = 0;
    Stone2Obj.duration   = 100;
    Stone2Obj.distance   = 0;
    Stone2Obj.insideOval = 0;

    Stone3Obj.on         = 0;
    Stone3Obj.duration   = 100;
    Stone3Obj.distance   = 0;
    Stone3Obj.insideOval = 0;

    Stone4Obj.on         = 0;
    Stone4Obj.duration   = 100;
    Stone4Obj.distance   = 0;
    Stone4Obj.insideOval = 0;

    Mushroom1Obj.on         = 0;
    Mushroom1Obj.duration   = 100;
    Mushroom1Obj.distance   = 0;
    Mushroom1Obj.insideOval = 0;

    Mushroom2Obj.on         = 0;
    Mushroom2Obj.duration   = 100;
    Mushroom2Obj.distance   = 0;
    Mushroom2Obj.insideOval = 0;

    Spike1Obj.on         = 0;
    Spike1Obj.duration   = 100;
    Spike1Obj.distance   = 0;
    Spike1Obj.insideOval = 0;

    PenguinGameLevel    = 1;
    PenguinGameLaps     = 1;
    PenguinGameLapsCopy = PenguinGameLaps;
    PenguinGameTime     = 61;

    PenguinInvulnerable = 1;
}

// ---------------------------------------------------------
// 10. PenguinMovementRoutine (Z80 port)
// ---------------------------------------------------------
static void PenguinMovementRoutine(void)
{
    framecounter2++;

    HandlePhase();
    IncreasePenguinSpeed();
    IncreaseDistanceAndSetXYPenguin();
    HandleJumpInsideOrOutsideOval();
    HandlePenguinGameObjects();
    HandlePenguinGameHud();
    HandlePenguinGameOver();
    PutNewWarningObjects();
    PutNewPizzaObjects();
    PutNewStarObjects();
    PutNewExtraTimeObjects();
    SetPenguinSprite();

    PenguinGameRandomValue = random() & 0x7F;
}

// ---------------------------------------------------------
// 11. Room logic (Bike Race minigame)
// ---------------------------------------------------------
GameState runBikeRace(void)
{
    drawRoomBackground(ROOM_BIKERACE);
    playMusic(tune_ship);

    VDP_loadTileSet(arcademachine.tileset, globalTileIndex, DMA);
    VDP_drawImageEx(
        BG_A,
        &arcademachine,
        TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, globalTileIndex),
        0, 0,
        FALSE,
        TRUE
    );
    globalTileIndex += arcademachine.tileset->numTile;

    SPR_reset();
    SPR_update();
    waitMs(120);

    BikeRaceState brState = BR_STATE_GAME;

    VDP_drawImageEx(
        BG_B,
        &bikeraceingameexample,
        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX),
        0, 0,
        FALSE,
        TRUE
    );
    PAL_fadeIn(0, 15, bikeraceingameexample.palette->data, 8, FALSE);

    // Create penguin sprite
    penguinSprite = SPR_addSprite(
        &penguinSpriteDef,
        penguinX,
        penguinY,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );

    // Load penguin palette
    PAL_setPalette(PAL2, penguinSpriteDef.palette->data, DMA);

    ResetVariablesPenguinRace();

    while (1)
    {
        playerHandleInput();
        u16 joyNew = playerGetJoyNew();

        switch (brState)
        {
            case BR_STATE_TITLE:

                if (joyNew & BUTTON_B)
                {
                    return STATE_ARCADE1;
                }

                if (joyNew & BUTTON_A)
                {
                    PAL_fadeOut(0, 15, 8, FALSE);

                    VDP_drawImageEx(
                        BG_B,
                        &bikeraceingameexample,
                        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX),
                        0, 0,
                        FALSE,
                        TRUE
                    );

                    PAL_fadeIn(0, 15, bikeraceingameexample.palette->data, 8, FALSE);

                    ResetVariablesPenguinRace();

                    brState = BR_STATE_GAME;
                }
                break;

            case BR_STATE_GAME:

                PenguinMovementRoutine();

                if (joyNew & BUTTON_B)
                {
                    PAL_fadeOut(0, 15, 8, FALSE);

                    VDP_drawImageEx(
                        BG_B,
                        &bikeracetitlescreen,
                        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX),
                        0, 0,
                        FALSE,
                        TRUE
                    );

                    PAL_fadeIn(0, 15, bikeracetitlescreen.palette->data, 8, FALSE);

                    brState = BR_STATE_TITLE;
                }

                break;
        }

        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_ARCADE1;
}
