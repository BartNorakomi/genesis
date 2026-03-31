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

extern const SpriteDefinition bikeRaceWarningIconSpriteDef;
extern const SpriteDefinition bikeRaceStarSpriteDef;
extern const SpriteDefinition bikeRacePizzaSpriteDef;
extern const SpriteDefinition bikeRaceExtraTimeSpriteDef;
extern const SpriteDefinition bikeRaceStoneSpriteDef;
extern const SpriteDefinition bikeRaceSpikeSpriteDef;
extern const SpriteDefinition bikeRaceMushroom1SpriteDef;
extern const SpriteDefinition bikeRaceMushroom2SpriteDef;


// ---------------------------------------------------------
// 2. Internal Bike Race states
// ---------------------------------------------------------
typedef enum
{
    BR_STATE_TITLE,
    BR_STATE_GAME,
    BR_STATE_GAMEOVER
} BikeRaceState;

BikeRaceState brState = BR_STATE_GAME;

// ---------------------------------------------------------
// 3. Penguin game globals
// ---------------------------------------------------------
static u16 framecounter2 = 0;
static u8 PenguinGameRandomValue = 0;

static u8 SpawnFrequencyPizza = 0;
static u8 SpawnFrequencyExtraTime = 0;
static u8 SpawnFrequencyStar = 0;

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
static u8 PenguinGameTimeSpeed = 0;

// Penguin position
static s16 penguinX = 30;
static s16 penguinY = 10;

//static u8 SpawnFrequencyWarning = 0;
static u8 SpawnFrequencyWarning = 1;


// ---------------------------------------------------------
// 4. Penguin game object struct
// ---------------------------------------------------------
typedef struct
{
    bool on;            // object active?
    u8 duration;        // countdown timer
    u16 distance;       // position along the track (0–439)
    bool insideOval;    // 0 = outside, 1 = inside

    s16 x;              // screen X
    s16 y;              // screen Y

    u8 animStep;        // spike animation step
    u8 type;            // optional: stone, spike, pizza, etc.

    Sprite* spr[4];     // up to 4 sprites per object
} PenguinObject;


// Forward declarations
static bool CheckDistanceAlreadyInUseByOtherObject(u16 dist);
static void HandleObject_SetXY(PenguinObject* obj);
static void SpawnObjectFromWarning(PenguinObject* warning);
static void AnimateSpike(PenguinObject* obj);

// Spawn helpers (will be added later)
static bool TrySpawnStone(PenguinObject* warning);
static bool TrySpawnSpike(PenguinObject* warning);
static bool TrySpawnMushroom1(PenguinObject* warning);
static bool TrySpawnMushroom2(PenguinObject* warning);
// Forward declaration (if you use it before this point)
static bool CheckCollisionPenguin(PenguinObject* obj);





// Forward declarations for object handlers
//static void HandleObjectWarning(PenguinObject* obj);
static void HandleObjectStar(PenguinObject* obj);
static void HandleObjectPizza(PenguinObject* obj);
static void HandleObjectExtraTime(PenguinObject* obj);
static void HandleObjectStone(PenguinObject* obj);
static void HandleObjectSpike(PenguinObject* obj);
static void HandleObjectMushroom1(PenguinObject* obj);
static void HandleObjectMushroom2(PenguinObject* obj);




// Spike animation tables converted to frame indices
// Each table has 32 entries (0–31)
static const u8 SpikeAnimUp[32] = {
    0,0,0,0,0,0,0,0,
    0,0,1,2,3,4,5,5,
    5,5,4,3,2,1,0,0,
    0,0,0,0,0,0,0,0
};

static const u8 SpikeAnimDown[32] = {
    6,6,6,6,6,6,6,6,
    6,6,7,8,9,10,11,11,
    11,11,10,9,8,7,6,6,
    6,6,6,6,6,6,6,6
};

static const u8 SpikeAnimRight[32] = {
    12,12,12,12,12,12,12,12,
    12,12,13,14,15,16,17,17,
    17,17,16,15,14,13,12,12,
    12,12,12,12,12,12,12,12
};

static const u8 SpikeAnimLeft[32] = {
    18,18,18,18,18,18,18,18,
    18,18,19,20,21,22,23,23,
    23,23,22,21,20,19,18,18,
    18,18,18,18,18,18,18,18
};


static const s16 CoordinateTableSpritesInsideOval[440][2] =
{
    // TopStraightSegment (130)
    {56,42},{57,42},{58,42},{59,42},{60,42},{61,42},{62,42},{63,42},{64,42},{65,42},
    {66,42},{67,42},{68,42},{69,42},{70,42},{71,42},{72,42},{73,42},{74,42},{75,42},
    {76,42},{77,42},{78,42},{79,42},{80,42},{81,42},{82,42},{83,42},{84,42},{85,42},
    {86,42},{87,42},{88,42},{89,42},{90,42},{91,42},{92,42},{93,42},{94,42},{95,42},
    {96,42},{97,42},{98,42},{99,42},{100,42},{101,42},{102,42},{103,42},{104,42},{105,42},
    {106,42},{107,42},{108,42},{109,42},{110,42},{111,42},{112,42},{113,42},{114,42},{115,42},
    {116,42},{117,42},{118,42},{119,42},{120,42},{121,42},{122,42},{123,42},{124,42},{125,42},
    {126,42},{127,42},{128,42},{129,42},{130,42},{131,42},{132,42},{133,42},{134,42},{135,42},
    {136,42},{137,42},{138,42},{139,42},{140,42},{141,42},{142,42},{143,42},{144,42},{145,42},
    {146,42},{147,42},{148,42},{149,42},{150,42},{151,42},{152,42},{153,42},{154,42},{155,42},
    {156,42},{157,42},{158,42},{159,42},{160,42},{161,42},{162,42},{163,42},{164,42},{165,42},
    {166,42},{167,42},{168,42},{169,42},{170,42},{171,42},{172,42},{173,42},{174,42},{175,42},
    {176,42},{177,42},{178,42},{179,42},{180,42},{181,42},{182,42},{183,42},{184,42},{185,42},

    // RightCurve (90)
    {185,41},{186,43},{186,43},{187,43},{187,43},{188,43},{189,43},{189,43},{190,44},{191,44},
    {191,44},{192,45},{192,45},{193,45},{194,46},{194,46},{195,47},{196,47},{196,48},{197,48},
    {197,49},{198,49},{198,50},{199,50},{199,51},{200,51},{200,52},{201,53},{201,53},{201,54},
    {202,54},{202,55},{202,56},{202,56},{203,57},{203,58},{203,58},{203,59},{203,60},{203,61},
    {203,61},{204,61},{203,62},{203,63},{203,64},{203,64},{203,65},{203,66},{203,66},{203,67},
    {203,68},{203,68},{203,69},{203,70},{203,71},{202,71},{202,72},{202,73},{202,73},{201,74},
    {201,75},{201,75},{201,75},{200,76},{200,77},{199,77},{199,78},{198,78},{198,79},{197,79},
    {197,80},{196,80},{196,81},{195,81},{194,81},{194,82},{193,82},{192,82},{192,82},{191,82},
    {191,82},{190,82},{189,83},{189,83},{188,83},{187,83},{187,83},{186,83},{186,83},{185,83},

    // BottomStraightSegment (130)
    {185,84},{184,84},{183,84},{182,84},{181,84},{180,84},{179,84},{178,84},{177,84},{176,84},
    {175,84},{174,84},{173,84},{172,84},{171,84},{170,84},{169,84},{168,84},{167,84},{166,84},
    {165,84},{164,84},{163,84},{162,84},{161,84},{160,84},{159,84},{158,84},{157,84},{156,84},
    {155,84},{154,84},{153,84},{152,84},{151,84},{150,84},{149,84},{148,84},{147,84},{146,84},
    {145,84},{144,84},{143,84},{142,84},{141,84},{140,84},{139,84},{138,84},{137,84},{136,84},
    {135,84},{134,84},{133,84},{132,84},{131,84},{130,84},{129,84},{128,84},{127,84},{126,84},
    {125,84},{124,84},{123,84},{122,84},{121,84},{120,84},{119,84},{118,84},{117,84},{116,84},
    {115,84},{114,84},{113,84},{112,84},{111,84},{110,84},{109,84},{108,84},{107,84},{106,84},
    {105,84},{104,84},{103,84},{102,84},{101,84},{100,84},{99,84},{98,84},{97,84},{96,84},
    {95,84},{94,84},{93,84},{92,84},{91,84},{90,84},{89,84},{88,84},{87,84},{86,84},
    {85,84},{84,84},{83,84},{82,84},{81,84},{80,84},{79,84},{78,84},{77,84},{76,84},
    {75,84},{74,84},{73,84},{72,84},{71,84},{70,84},{69,84},{68,84},{67,84},{66,84},
    {65,84},{64,84},{63,84},{62,84},{61,84},{60,84},{59,84},{58,84},{57,84},{56,84},

    // LeftCurve (90)
    {56,83},{55,82},{55,82},{54,82},{54,82},{53,82},{52,82},{52,82},{51,81},{50,81},
    {50,81},{49,80},{49,80},{48,80},{47,79},{47,79},{46,78},{45,78},{45,77},{44,77},
    {44,76},{43,76},{43,75},{42,75},{42,74},{41,74},{41,73},{40,72},{40,72},{40,71},
    {39,71},{39,70},{39,69},{39,69},{38,68},{38,67},{38,67},{38,66},{38,65},{38,64},
    {38,64},{37,64},{38,63},{38,62},{38,61},{38,61},{38,60},{38,59},{38,59},{38,58},
    {38,57},{38,57},{38,56},{38,55},{38,54},{39,54},{39,53},{39,52},{39,52},{40,51},
    {40,50},{40,50},{40,50},{41,49},{41,48},{42,48},{42,47},{43,47},{43,46},{44,46},
    {44,45},{45,45},{45,44},{46,44},{47,44},{47,43},{48,43},{49,43},{49,43},{50,43},
    {50,43},{51,43},{52,42},{52,42},{53,42},{54,42},{54,42},{55,42},{55,42},{56,42}
};

static const s16 CoordinateTableSpritesOutsideOval[440][2] =
{
    // TopStraightSegment (130)
    {56,26},{57,26},{58,26},{59,26},{60,26},{61,26},{62,26},{63,26},{64,26},{65,26},
    {66,26},{67,26},{68,26},{69,26},{70,26},{71,26},{72,26},{73,26},{74,26},{75,26},
    {76,26},{77,26},{78,26},{79,26},{80,26},{81,26},{82,26},{83,26},{84,26},{85,26},
    {86,26},{87,26},{88,26},{89,26},{90,26},{91,26},{92,26},{93,26},{94,26},{95,26},
    {96,26},{97,26},{98,26},{99,26},{100,26},{101,26},{102,26},{103,26},{104,26},{105,26},
    {106,26},{107,26},{108,26},{109,26},{110,26},{111,26},{112,26},{113,26},{114,26},{115,26},
    {116,26},{117,26},{118,26},{119,26},{120,26},{121,26},{122,26},{123,26},{124,26},{125,26},
    {126,26},{127,26},{128,26},{129,26},{130,26},{131,26},{132,26},{133,26},{134,26},{135,26},
    {136,26},{137,26},{138,26},{139,26},{140,26},{141,26},{142,26},{143,26},{144,26},{145,26},
    {146,26},{147,26},{148,26},{149,26},{150,26},{151,26},{152,26},{153,26},{154,26},{155,26},
    {156,26},{157,26},{158,26},{159,26},{160,26},{161,26},{162,26},{163,26},{164,26},{165,26},
    {166,26},{167,26},{168,26},{169,26},{170,26},{171,26},{172,26},{173,26},{174,26},{175,26},
    {176,26},{177,26},{178,26},{179,26},{180,26},{181,26},{182,26},{183,26},{184,26},{185,26},

    // RightCurve (90)
    {185,26},{186,26},{188,26},{189,26},{190,26},{191,27},{193,27},{194,27},{195,27},{197,28},
    {198,28},{199,29},{200,29},{201,30},{203,30},{204,31},{205,32},{206,32},{207,33},{208,34},
    {209,35},{210,36},{211,37},{212,38},{213,39},{214,39},{214,41},{215,42},{216,43},{217,44},
    {217,45},{218,46},{218,47},{219,48},{219,50},{220,51},{220,52},{221,53},{221,55},{221,56},
    {222,57},{222,58},{222,60},{222,61},{222,62},{222,64},{222,65},{222,66},{222,68},{222,69},
    {221,70},{221,71},{221,73},{220,74},{220,75},{219,76},{219,78},{218,79},{218,80},{217,81},
    {217,82},{216,83},{215,84},{214,85},{214,87},{213,87},{212,88},{211,89},{210,90},{209,91},
    {208,92},{207,93},{206,94},{205,94},{204,95},{203,96},{201,96},{200,97},{199,97},{198,98},
    {197,98},{195,99},{194,99},{193,99},{191,99},{190,100},{189,100},{188,100},{186,100},{185,100},

    // BottomStraightSegment (130)
    {185,100},{184,100},{183,100},{182,100},{181,100},{180,100},{179,100},{178,100},{177,100},{176,100},
    {175,100},{174,100},{173,100},{172,100},{171,100},{170,100},{169,100},{168,100},{167,100},{166,100},
    {165,100},{164,100},{163,100},{162,100},{161,100},{160,100},{159,100},{158,100},{157,100},{156,100},
    {155,100},{154,100},{153,100},{152,100},{151,100},{150,100},{149,100},{148,100},{147,100},{146,100},
    {145,100},{144,100},{143,100},{142,100},{141,100},{140,100},{139,100},{138,100},{137,100},{136,100},
    {135,100},{134,100},{133,100},{132,100},{131,100},{130,100},{129,100},{128,100},{127,100},{126,100},
    {125,100},{124,100},{123,100},{122,100},{121,100},{120,100},{119,100},{118,100},{117,100},{116,100},
    {115,100},{114,100},{113,100},{112,100},{111,100},{110,100},{109,100},{108,100},{107,100},{106,100},
    {105,100},{104,100},{103,100},{102,100},{101,100},{100,100},{99,100},{98,100},{97,100},{96,100},
    {95,100},{94,100},{93,100},{92,100},{91,100},{90,100},{89,100},{88,100},{87,100},{86,100},
    {85,100},{84,100},{83,100},{82,100},{81,100},{80,100},{79,100},{78,100},{77,100},{76,100},
    {75,100},{74,100},{73,100},{72,100},{71,100},{70,100},{69,100},{68,100},{67,100},{66,100},
    {65,100},{64,100},{63,100},{62,100},{61,100},{60,100},{59,100},{58,100},{57,100},{56,100},

    // LeftCurve (90)
    {56,100},{55,100},{53,100},{52,100},{51,100},{50,99},{48,99},{47,99},{46,99},{44,98},
    {43,98},{42,97},{41,97},{40,96},{38,96},{37,95},{36,94},{35,94},{34,93},{33,92},
    {32,91},{31,90},{30,89},{29,88},{28,87},{27,87},{27,85},{26,84},{25,83},{24,82},
    {24,81},{23,80},{23,79},{22,78},{22,76},{21,75},{21,74},{20,73},{20,71},{20,70},
    {19,69},{19,68},{19,66},{19,65},{19,64},{19,62},{19,61},{19,60},{19,58},{19,57},
    {20,56},{20,55},{20,53},{21,52},{21,51},{22,50},{22,48},{23,47},{23,46},{24,45},
    {24,44},{25,43},{26,42},{27,41},{27,39},{28,39},{29,38},{30,37},{31,36},{32,35},
    {33,34},{34,33},{35,32},{36,32},{37,31},{38,30},{40,30},{41,29},{42,29},{43,28},
    {44,28},{46,27},{47,27},{48,27},{50,27},{51,26},{52,26},{53,26},{55,26},{56,26}
};

static void HandleObjectWarning(PenguinObject* obj)
{
    if (!obj->on)
        return;

    // Blink: visible for 8 frames, hidden for 8 frames
    if ((framecounter2 & 15) < 8)
    {
        // Position sprite normally
        HandleObject_SetXY(obj);
        SPR_setVisibility(obj->spr[0], VISIBLE);
    }
    else
    {
        // Hide sprite by moving it offscreen
        SPR_setVisibility(obj->spr[0], HIDDEN);
    }

    // Countdown
    obj->duration--;
    if (obj->duration > 0)
        return;

    // Warning expired → turn off
    obj->on = false;

    // Hide sprite
    SPR_setVisibility(obj->spr[0], HIDDEN);

    // Spawn a new object (stone/spike/mushroom)
    SpawnObjectFromWarning(obj);
    obj->distance = 0;
}

static void SpawnObjectFromWarning(PenguinObject* warning)
{
        // TrySpawnMushroom2(warning);
        // return;

    // Random 0–3
    u8 r = random() & 3;

    // 0 → try mushroom2
    // 1 → try spike
    // 2 → try mushroom1
    // 3 → try stone
    // (same order as Z80)

    if (r == 0)
    {
        if (TrySpawnMushroom2(warning)) return;
        if (TrySpawnSpike(warning)) return;
        if (TrySpawnMushroom1(warning)) return;
        TrySpawnStone(warning);
        return;
    }

    if (r == 1)
    {
        if (TrySpawnSpike(warning)) return;
        if (TrySpawnMushroom1(warning)) return;
        if (TrySpawnMushroom2(warning)) return;
        TrySpawnStone(warning);
        return;
    }

    if (r == 2)
    {
        if (TrySpawnMushroom1(warning)) return;
        if (TrySpawnSpike(warning)) return;
        if (TrySpawnMushroom2(warning)) return;
        TrySpawnStone(warning);
        return;
    }

    // r == 3
    if (TrySpawnStone(warning)) return;
    if (TrySpawnSpike(warning)) return;
    if (TrySpawnMushroom1(warning)) return;
    TrySpawnMushroom2(warning);
}

static void HandleObject_SetXY(PenguinObject* obj)
{
    if (!obj->on)
        return;

    // 1. Convert distance → index into coordinate table
    u16 idx = obj->distance;      // 0–439
    idx <<= 1;                    // *2 because table is [440][2]

    // 2. Select inside/outside table
    const s16 (*table)[2] = obj->insideOval
        ? CoordinateTableSpritesInsideOval
        : CoordinateTableSpritesOutsideOval;

    // 3. Read coordinates
    s16 x = table[obj->distance][0];
    s16 y = table[obj->distance][1];

    obj->x = x;
    obj->y = y;

    // 4. Update all sprites (1–4 depending on object)
    for (int i = 0; i < 4; i++)
    {
        if (obj->spr[i])
        {
            SPR_setPosition(obj->spr[i], x, y);
        }
    }
}

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

    47,46,45,44,43,42,41,40,
    39,38,37,

    36,36,36,36,36,36,36,36,
    36,36,36,36,36,36,36,36,

    35,34,33,32,31,30,29,28,
    27,26,25,

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

static void HandleJumpInsideOrOutsideOval(void)
{
    // Read newly pressed buttons
    u16 joyNew = playerGetJoyNew();

    // Check for BUTTON_A (you can change to BUTTON_B or BUTTON_C if needed)
    if (joyNew & BUTTON_A)
    {
        // Toggle 0 ↔ 1
        PenguinInsideOval ^= 1;
    }
}

static void HandlePenguinGameObjects(void)
{
    HandleObjectWarning(&WarningObj);
    HandleObjectStar(&StarObj);
    HandleObjectPizza(&PizzaObj);
    HandleObjectExtraTime(&ExtraTimeObj);

    HandleObjectStone(&Stone1Obj);
    HandleObjectStone(&Stone2Obj);
    HandleObjectStone(&Stone3Obj);
    HandleObjectStone(&Stone4Obj);

    HandleObjectSpike(&Spike1Obj);

    HandleObjectMushroom1(&Mushroom1Obj);
    HandleObjectMushroom2(&Mushroom2Obj);
}

static void HandleObjectStar(PenguinObject* obj)
{
    if (!obj->on)
        return;

    // Duration countdown
    if (obj->duration > 0)
    {
        obj->duration--;
        if (obj->duration == 0)
        {
            // Remove star
            SPR_setVisibility(obj->spr[0], HIDDEN);
            obj->on = false;
            obj->distance = 0;
            return;
        }
    }

    // Update XY
    HandleObject_SetXY(obj);

    // Collision check
    if (CheckCollisionPenguin(obj))
    {
        // Star effect: invulnerability
        PenguinInvulnerable = 200;

        // Remove star
        SPR_setVisibility(obj->spr[0], HIDDEN);
        obj->on = false;
        obj->distance = 0;
        return;
    }
}

static void HandleObjectPizza(PenguinObject* obj)
{
    if (!obj->on)
        return;

    // Duration countdown
    if (obj->duration > 0)
    {
        obj->duration--;
        if (obj->duration == 0)
        {
            // Remove pizza
            SPR_setVisibility(obj->spr[0], HIDDEN);
            obj->on = false;
            obj->distance = 0;
            return;
        }
    }

    // Update XY
    HandleObject_SetXY(obj);

    // Collision check
    if (CheckCollisionPenguin(obj))
    {
        // Pizza effect: speed boost
        AddedPizzaSpeedBoost = 90;

        // Remove pizza
        SPR_setVisibility(obj->spr[0], HIDDEN);
        obj->on = false;
        obj->distance = 0;
    }
}

static void HandleObjectExtraTime(PenguinObject* obj)
{
    if (!obj->on)
        return;

    // Duration countdown
    if (obj->duration > 0)
    {
        obj->duration--;
        if (obj->duration == 0)
        {
            // Remove object
            SPR_setVisibility(obj->spr[0], HIDDEN);
            obj->on = false;
            obj->distance = 0;
            return;
        }
    }

    // Update XY
    HandleObject_SetXY(obj);

    // Collision check
    if (CheckCollisionPenguin(obj))
    {
        // Extra time effect: +5 seconds
        PenguinGameTimeExtended = 5;

        // Remove object
        SPR_setVisibility(obj->spr[0], HIDDEN);
        obj->on = false;
        obj->distance = 0;
        return;
    }
}

static void HandleObjectStone(PenguinObject* obj)
{
    if (!obj->on)
        return;

    // Update XY
    HandleObject_SetXY(obj);

    // Collision check only if not invulnerable
    if (PenguinInvulnerable == 0)
    {
        if (CheckCollisionPenguin(obj))
        {
            // --- CollisionStone ---
            SPR_setVisibility(obj->spr[0], HIDDEN);
            obj->on = false;
            obj->distance = 0;

            PenguinSpeed = 0;
            PenguinInvulnerable = 100;

            return;
        }
    }

    // Blink logic: update only on even frames
    if (framecounter2 & 1)
        return;

    // Decrement duration
    if (obj->duration > 0)
    {
        obj->duration--;
        if (obj->duration > 0)
            return;
    }

    // Duration expired → turn off
    obj->on = false;
    obj->distance = 0;
    SPR_setVisibility(obj->spr[0], HIDDEN);
}

static bool CheckCollisionPenguin(PenguinObject* obj)
{
    // Distance difference (object - penguin)
    s16 diff = (s16)obj->distance - (s16)PenguinDistance;

    // MSX logic:
    // If objectDistance - 10 > penguinDistance → no collision
    if (diff > 10)
        return false;

    // If objectDistance + 10 < penguinDistance → no collision
    if (diff < -10)
        return false;

    // Inside/outside must match
    if (obj->insideOval != PenguinInsideOval)
        return false;

    // Collision!
    return true;
}

static void AnimateSpike(PenguinObject* obj)
{
    // Determine direction based on distance and inside/outside
    const u8* table;

    u16 d = obj->distance;

    char buf[8];
    sprintf(buf, "%3d", d);
    VDP_drawTextBG(BG_A, buf, 8, 27);
    
    if (!obj->insideOval)
    {
        // Outside oval
        if (d < 180)          table = SpikeAnimUp;
        else if (d < 220)     table = SpikeAnimRight;
        else if (d < 360)     table = SpikeAnimDown;
        else                  table = SpikeAnimLeft;
    }
    else
    {
        // Inside oval
        if (d < 180)          table = SpikeAnimDown;
        else if (d < 220)     table = SpikeAnimLeft;
        else if (d < 360)     table = SpikeAnimUp;
        else                  table = SpikeAnimRight;
    }

    // Advance animation every 4 frames
    if ((framecounter2 & 3) == 0)
    {
        obj->animStep = (obj->animStep + 1) & 31;
    }

    // Apply frame
    u8 frame = table[obj->animStep];
    SPR_setFrame(obj->spr[0], frame);
}

static void HandleObjectSpike(PenguinObject* obj)
{
    // Animate first (matches MSX)
    AnimateSpike(obj);

    if (!obj->on)
        return;

    // Update XY
    HandleObject_SetXY(obj);

    // Spike only collides when fully extended (frames 14–17)
    if (obj->animStep >= 14 && obj->animStep < 18)
    {
        if (PenguinInvulnerable == 0)
        {
            if (CheckCollisionPenguin(obj))
            {
                // CollisionSpike
                SPR_setVisibility(obj->spr[0], HIDDEN);
                obj->on = false;
                obj->distance = 0;

                PenguinSpeed = 0;
                PenguinInvulnerable = 100;
                return;
            }
        }
    }

    // Blink logic: update only on even frames
    if (framecounter2 & 1)
        return;

    // Decrement duration
    if (obj->duration > 0)
    {
        obj->duration--;
        if (obj->duration > 0)
            return;
    }

    // Duration expired → turn off
    obj->on = false;
    obj->distance = 0;
    SPR_setVisibility(obj->spr[0], HIDDEN);
}

static void HandleObjectMushroom1(PenguinObject* obj)
{
    if (!obj->on)
        return;

    /// --- Duration countdown ---
    if (obj->duration > 0)
    {
        obj->duration--;
        if (obj->duration == 0)
        {
            // Remove mushroom
            SPR_setVisibility(obj->spr[0], HIDDEN);
            obj->on = false;
            obj->distance = 0;
            return;
        }
    }

    // --- Update XY ---
    HandleObject_SetXY(obj);

    // Position the single sprite
    SPR_setPosition(obj->spr[0], obj->x, obj->y-16);

    // --- Collision check ---
    if (PenguinInvulnerable)
        return;

    if (!CheckCollisionPenguin(obj))
        return;

    // --- Collision effect: reduce speed by 15 ---
    int newSpeed = (int)PenguinSpeed - 15;
    if (newSpeed < 0) newSpeed = 0;
    PenguinSpeed = (u8)newSpeed;

    /// --- Remove mushroom after collision ---
    SPR_setVisibility(obj->spr[0], HIDDEN);
    obj->on = false;
    obj->distance = 0;
}

static void HandleObjectMushroom2(PenguinObject* obj)
{
    if (!obj->on)
        return;

    /// --- Duration countdown ---
    if (obj->duration > 0)
    {
        obj->duration--;
        if (obj->duration == 0)
        {
            // Remove mushroom
            SPR_setVisibility(obj->spr[0], HIDDEN);
            obj->on = false;
            obj->distance = 0;
            return;
        }
    }

    // --- Update XY ---
    HandleObject_SetXY(obj);

    // Position the single sprite
    SPR_setPosition(obj->spr[0], obj->x, obj->y);

    // --- Collision check ---
    if (PenguinInvulnerable)
        return;

    if (!CheckCollisionPenguin(obj))
        return;

    // --- Collision effect: reduce speed by 15 ---
    int newSpeed = (int)PenguinSpeed - 15;
    if (newSpeed < 0) newSpeed = 0;
    PenguinSpeed = (u8)newSpeed;

    /// --- Remove mushroom after collision ---
    SPR_setVisibility(obj->spr[0], HIDDEN);
    obj->on = false;
    obj->distance = 0;
}


static bool TrySpawnStone(PenguinObject* warning)
{
    // Find a free stone slot
    PenguinObject* stone =
        !Stone1Obj.on ? &Stone1Obj :
        !Stone2Obj.on ? &Stone2Obj :
        !Stone3Obj.on ? &Stone3Obj :
        !Stone4Obj.on ? &Stone4Obj : NULL;

    if (!stone)
        return false;   // no free stone

    // Copy distance + insideOval from warning
    stone->on = true;
    stone->duration = 255;   // you must define this
    stone->distance = warning->distance;
    stone->insideOval = warning->insideOval;
    stone->animStep = 0;

    // Make sprite visible
    SPR_setVisibility(stone->spr[0], VISIBLE);

    return true;
}

static bool TrySpawnSpike(PenguinObject* warning)
{
    // Only one spike object exists in the MSX version
    PenguinObject* spike = &Spike1Obj;

    // If spike is already active → cannot spawn
    if (spike->on)
        return false;

    // Activate spike
    spike->on = true;

    // Copy distance + insideOval from warning
    spike->distance = warning->distance;
    spike->insideOval = warning->insideOval;

    // SpikeDuration from MSX (same as stone logic)
    spike->duration = 255;   // or define SPIKE_DURATION if you prefer

    // Reset animation step
    spike->animStep = 0;

    // Make sprite visible
    SPR_setVisibility(spike->spr[0], VISIBLE);

    return true;
}

static bool TrySpawnMushroom1(PenguinObject* warning)
{
    PenguinObject* mush = &Mushroom1Obj;

    // If already active → cannot spawn
    if (mush->on)
        return false;

    u16 dist = warning->distance;

    // --- Placement rules ---
    if (warning->insideOval)
    {
        // Inside oval → only bottom straight (220 ≤ dist < 360)
        if (dist < 220 || dist >= 360)
            return false;
    }
    else
    {
        // Outside oval → only top straight (distance < 180)
        if (dist >= 180)
            return false;
    }

    // --- Place Mushroom1 ---
    mush->on = true;
    mush->distance = dist;
    mush->insideOval = warning->insideOval;
    mush->duration = 255;     // MushroomDuration in MSX
    mush->animStep = 0;

    // Make sprite visible
    SPR_setVisibility(mush->spr[0], VISIBLE);

    return true;
}

static bool TrySpawnMushroom2(PenguinObject* warning)
{
    PenguinObject* mush = &Mushroom2Obj;

    if (mush->on)
        return false;

    u16 dist = warning->distance;

    if (!warning->insideOval)
    {
        // OUTSIDE oval → only bottom straight (140 ≤ d < 220)
        if (dist < 220 || dist >= 360)
            return false;
    }
    else
    {
        // INSIDE oval → only top straight (d < 180)
        if (dist >= 180)
            return false;
    }

    // Place Mushroom2
    mush->on = true;
    mush->distance = dist;
    mush->insideOval = warning->insideOval;
    mush->duration = 255;
    mush->animStep = 0;

    SPR_setVisibility(mush->spr[0], VISIBLE);

    return true;
}

static void HandlePenguinGameHud(void)
{
    u8 phase = framecounter2 & 15;

    // 0: adjust time + laps copy + time ticking
    if (phase == 0)
    {
        // Apply extended time (from ExtraTime / level up)
        if (PenguinGameTimeExtended > 0)
        {
            PenguinGameTime += PenguinGameTimeExtended;
            PenguinGameTimeExtended = 0;
        }

        // Copy laps
        PenguinGameLapsCopy = PenguinGameLaps;

        // Time speed logic (every 3 ticks, decrement time)
        PenguinGameTimeSpeed++;
        if (PenguinGameTimeSpeed >= 3)
        {
            PenguinGameTimeSpeed = 0;

            if (PenguinGameTime > 0)
                PenguinGameTime--;
        }

        return;
    }

    // 1–4: draw time
    if (phase < 5)
    {
        char buf[4];
        sprintf(buf, "%3d", PenguinGameTime);
        // TimeDX=29 → ~x=4, TimeDY=8 → y=1
        VDP_drawTextBG(BG_A, "   ", 4, 1);      // erase
        VDP_drawTextBG(BG_A, buf, 4, 1);
        return;
    }

    // 5–8: draw level
    if (phase < 9)
    {
        char buf[4];
        sprintf(buf, "%3d", PenguinGameLevel);
        // LevelDX=140 → ~x=18, LevelDY=8 → y=1
        VDP_drawTextBG(BG_A, "   ", 18, 1);     // erase
        VDP_drawTextBG(BG_A, buf, 18, 1);
        return;
    }

    // 9–15: draw laps (using PenguinGameLapsCopy)
    {
        char buf[4];
        sprintf(buf, "%3d", PenguinGameLapsCopy);
        // LapsDX=231 → ~x=29, LapsDY=8 → y=1
        VDP_drawTextBG(BG_A, "   ", 27, 1);     // erase
        VDP_drawTextBG(BG_A, buf, 27, 1);
    }
}




static void HandlePenguinGameOver(void)
{
    u16 joy = playerGetJoy();

    // If trigger B is pressed → jump directly to GAME OVER
    if (joy & BUTTON_B)
        goto GAME_OVER;

    // If time > 0 → not game over
    if (PenguinGameTime > 0)
        return;

    // If speed > 0 → not game over
    if (PenguinSpeed > 0)
        return;

    // Time == 0 AND Speed == 0 → game over
GAME_OVER:

    // Draw game over gfx
    VDP_drawImageEx(
        BG_B,
        &basketballgameover,
        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX),
        0, 0,
        FALSE,
        TRUE
    );

    // Switch state
    brState = BR_STATE_GAMEOVER;
}

static const u16 DistanceTable[33] =
{
    20, 60, 100, 140, 180, 220, 260, 300, 340, 380, 420,
    20, 60, 100, 140, 180, 220, 260, 300, 340, 380, 420,
    20, 60, 100, 140, 180, 220, 260, 300, 340, 380, 420
};


static void PutNewWarningObjects(void)
{
    // Countdown spawn frequency
    SpawnFrequencyWarning--;
    if (SpawnFrequencyWarning != 0)
        return;

    // Compute new spawn frequency based on level
    u8 lvl = PenguinGameLevel * 2;
    s8 freq = 30 - lvl;
    if (freq < 0) freq = 0;
    freq++;
    SpawnFrequencyWarning = freq;

    // If warning already active → do nothing
    if (WarningObj.on)
        return;

    // Pick random distance index (0–31)
    u8 r = random() & 31;
    u16 dist = DistanceTable[r];

    // Check if distance is already used by another object
    if (CheckDistanceAlreadyInUseByOtherObject(dist))
        return;

    // Activate warning object
    WarningObj.on = true;
    WarningObj.duration = 50;
    WarningObj.distance = dist;
    WarningObj.animStep = 0;

    // Random inside/outside
    WarningObj.insideOval = (random() & 1);

    // Update XY
    HandleObject_SetXY(&WarningObj);

    // Make sprite visible
    SPR_setVisibility(WarningObj.spr[0], VISIBLE);
}

static bool CheckDistanceAlreadyInUseByOtherObject(u16 dist)
{
    // Compare with every object's distance
    if (WarningObj.on     && WarningObj.distance     == dist) return true;
    if (StarObj.on        && StarObj.distance        == dist) return true;
    if (PizzaObj.on       && PizzaObj.distance       == dist) return true;
    if (ExtraTimeObj.on   && ExtraTimeObj.distance   == dist) return true;
    if (Stone1Obj.on      && Stone1Obj.distance      == dist) return true;
    if (Stone2Obj.on      && Stone2Obj.distance      == dist) return true;
    if (Stone3Obj.on      && Stone3Obj.distance      == dist) return true;
    if (Stone4Obj.on      && Stone4Obj.distance      == dist) return true;
    if (Mushroom1Obj.on   && Mushroom1Obj.distance   == dist) return true;
    if (Mushroom2Obj.on   && Mushroom2Obj.distance   == dist) return true;
    if (Spike1Obj.on      && Spike1Obj.distance      == dist) return true;

    return false;
}




static void PutNewPizzaObjects(void)
{
    PenguinObject* obj = &PizzaObj;

    // Already active?
    if (obj->on)
        return;

    // Spawn frequency countdown
    if (SpawnFrequencyPizza > 0)
    {
        SpawnFrequencyPizza--;
        if (SpawnFrequencyPizza > 0)
            return;
    }

    // Reset spawn timer (MSX uses 100)
    SpawnFrequencyPizza = 100;

    // Pick random distance index (0–31)
    u8 r = random() & 31;
    u16 dist = DistanceTable[r];

    // Check if distance is already used
    if (CheckDistanceAlreadyInUseByOtherObject(dist))
        return;

    // Activate pizza
    obj->on = true;
    obj->duration = 250;
    obj->distance = dist;

    // insideOval = r & 1
    obj->insideOval = (r & 1);

        // Update XY
    HandleObject_SetXY(obj);

    SPR_setVisibility(obj->spr[0], VISIBLE);
}

static void PutNewStarObjects(void)
{
    PenguinObject* obj = &StarObj;

    // Already active?
    if (obj->on)
        return;

    // Spawn frequency countdown
    if (SpawnFrequencyStar > 0)
    {
        SpawnFrequencyStar--;
        if (SpawnFrequencyStar > 0)
            return;
    }

    // Reset spawn timer (MSX uses 50)
    SpawnFrequencyStar = 50;

    // Extra MSX rule: only spawn if PenguinGameRandomValue & 7 == 0
    if (PenguinGameRandomValue & 7)
        return;

    // Pick random distance index (0–31)
    u8 r = random() & 31;
    u16 dist = DistanceTable[r];

    // Check if distance is already used
    if (CheckDistanceAlreadyInUseByOtherObject(dist))
        return;

    // Activate star
    obj->on = true;
    obj->duration = 250;
    obj->distance = dist;

    // insideOval = r & 1
    obj->insideOval = (r & 1);

    // Update XY
    HandleObject_SetXY(obj);

    SPR_setVisibility(obj->spr[0], VISIBLE);
}

static void PutNewExtraTimeObjects(void)
{
    PenguinObject* obj = &ExtraTimeObj;

    // Already active?
    if (obj->on)
        return;

    // Spawn frequency countdown
    if (SpawnFrequencyExtraTime > 0)
    {
        SpawnFrequencyExtraTime--;
        if (SpawnFrequencyExtraTime > 0)
            return;
    }

    // NOTE:
    // MSX commented out the reset:
    //   ld a,250
    //   ld (SpawnFrequencyExtraTime),a
    // So we DO NOT reset it here.
    // ExtraTime spawns only once per countdown cycle.

    // Pick random distance index (0–31)
    u8 r = random() & 31;
    u16 dist = DistanceTable[r];

    // Check if distance is already used
    if (CheckDistanceAlreadyInUseByOtherObject(dist))
        return;

    // Activate ExtraTime
    obj->on = true;
    obj->duration = 190;
    obj->distance = dist;

    // insideOval = r & 1
    obj->insideOval = (r & 1);

        // Update XY
    HandleObject_SetXY(obj);

    SPR_setVisibility(obj->spr[0], VISIBLE);
}


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
    SpawnFrequencyStar = 200;

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

    // Create penguin sprite
    penguinSprite = SPR_addSprite(
        &penguinSpriteDef,
        penguinX,
        penguinY,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );

    // Load penguin palette
    PAL_setPalette(PAL2, penguinSpriteDef.palette->data, DMA);

    WarningObj.spr[0] = SPR_addSprite(
        &bikeRaceWarningIconSpriteDef,
        0, 0,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );
    SPR_setVisibility(WarningObj.spr[0], HIDDEN);

    StarObj.spr[0] = SPR_addSprite(
        &bikeRaceStarSpriteDef,
        0, 0,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );
    SPR_setVisibility(StarObj.spr[0], HIDDEN);

    PizzaObj.spr[0] = SPR_addSprite(
        &bikeRacePizzaSpriteDef,
        0, 0,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );
    SPR_setVisibility(PizzaObj.spr[0], HIDDEN);

    ExtraTimeObj.spr[0] = SPR_addSprite(
        &bikeRaceExtraTimeSpriteDef,
        0, 0,
        TILE_ATTR(PAL2, FALSE, FALSE, FALSE)
    );
    SPR_setVisibility(ExtraTimeObj.spr[0], HIDDEN);

    Stone1Obj.spr[0] = SPR_addSprite(&bikeRaceStoneSpriteDef, 0, 0, TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    SPR_setVisibility(Stone1Obj.spr[0], HIDDEN);

    Stone2Obj.spr[0] = SPR_addSprite(&bikeRaceStoneSpriteDef, 0, 0, TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    SPR_setVisibility(Stone2Obj.spr[0], HIDDEN);

    Stone3Obj.spr[0] = SPR_addSprite(&bikeRaceStoneSpriteDef, 0, 0, TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    SPR_setVisibility(Stone3Obj.spr[0], HIDDEN);

    Stone4Obj.spr[0] = SPR_addSprite(&bikeRaceStoneSpriteDef, 0, 0, TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    SPR_setVisibility(Stone4Obj.spr[0], HIDDEN);

    Spike1Obj.spr[0] = SPR_addSprite(&bikeRaceSpikeSpriteDef, 0, 0, TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    SPR_setVisibility(Spike1Obj.spr[0], HIDDEN);

    Mushroom1Obj.spr[0] = SPR_addSprite(&bikeRaceMushroom1SpriteDef, 0, 0, TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    SPR_setVisibility(Mushroom1Obj.spr[0], HIDDEN);

    Mushroom2Obj.spr[0] = SPR_addSprite(&bikeRaceMushroom2SpriteDef, 0, 0, TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    SPR_setVisibility(Mushroom2Obj.spr[0], HIDDEN);

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
                break;

            case BR_STATE_GAMEOVER:
            {
                // Wait for button press to exit
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
        }

        SPR_update();
        SYS_doVBlankProcess();
    }

    return STATE_ARCADE1;
}
