#ifndef SAVE_DATA_H
#define SAVE_DATA_H

#include <genesis.h>

// ---------------------------------------------------------
// CONSTANTS (from equ)
// ---------------------------------------------------------
#define VALUE_LEVEL1_RESOURCES  2
#define VALUE_LEVEL2_RESOURCES  5
#define VALUE_LEVEL3_RESOURCES  8
#define VALUE_LEVEL4_RESOURCES  12
#define VALUE_LEVEL5_RESOURCES  18
#define VALUE_LEVEL6_RESOURCES  30
#define VALUE_LEVEL7_RESOURCES  50

#define FUEL_TANK_LEVEL1_MAX_FUEL 300
#define FUEL_TANK_LEVEL2_MAX_FUEL 500
#define FUEL_TANK_LEVEL3_MAX_FUEL 700
#define FUEL_TANK_LEVEL4_MAX_FUEL 1000

#define CARGO_SIZE_LEVEL1_MAX_STORAGE 32
#define CARGO_SIZE_LEVEL2_MAX_STORAGE 45
#define CARGO_SIZE_LEVEL3_MAX_STORAGE 60
#define CARGO_SIZE_LEVEL4_MAX_STORAGE 90

// ---------------------------------------------------------
// FULL SAVE STRUCT (camelCase)
// ---------------------------------------------------------
typedef struct {

    // --- StartSaveGameData ---
    u8  totalLoopsDone;
    u16 totalCoinsBasketball;
    u8  ballsPurchased;
    u8  currentBallsSelected;

    // --- StartCleanSaveData ---
    u8  currentRoom;
    u8  gamesPlayed;
    u8  highScoreTotalAverageAtLeast80Percent;
    u8  highScoreBackroomGame;

    u8  basketballCompletePercentage;
    u16 highScoreBasketball;

    u8  jumpDownCompletePercentage;
    u16 highScoreJumpDown;

    u8  blockHitCompletePercentage;
    u16 highScoreBlockHit;

    u8  bikeRaceCompletePercentage;
    u16 highScoreBikeRace;

    u8  penguinGameLevelHighest;
    u8  penguinGameLapsHighest;

    u8  neonHorizonDifficultyUnlocked;

    u8  youCanEndGameWithTrigBMessageDone;
    u8  convGirl;
    u8  convCapGirl;
    u8  convGingerBoy;

    u8  convHost;
    u8  convEntity;
    u8  convEntityShipExplanations;

    u8  startWakeUpEvent;

    u8  dailyContinuesUsed;

    u16 totalMinutesUntilLand;

    u8  totalTimeUntilChildrenAreBornCounter1;
    u8  totalTimeUntilChildrenAreBornCounter2;
    u16 daysUntilChildrenAreBorn;

    u8  year1234StatusUpdated;
    u8  lowOnOxygenFoodWaterUpdated;

    u8  oxygenGeneratorPurchased;
    u8  waterRecyclerPurchased;
    u8  colonyExpansionPurchased;

    u16 oxygenOnShip;
    u16 maxOxygenOnShip;
    u16 foodOnShip;
    u16 maxFoodOnShip;
    u16 waterOnShip;
    u16 maxWaterOnShip;

    u8  amountOfDigSitesUnlocked;

    u8  playerCollapsedNoOxygen;
    u8  playerCollapsedNoFood;
    u8  playerCollapsedNoWater;
    u8  playerCollapsedHighRadiation;
    u8  playerCollapsedNoFuel;
    u8  playerCollapsedNoEnergy;

    u16 totalCredits;

    // Drilling game
    u8  conicalDrillBit;
    u8  minerSpeedLevel;
    u8  fuelTankLevel;
    u8  cargoSizeLevel;

    u16 level1Resources;
    u16 level2Resources;
    u16 level3Resources;
    u16 level4Resources;
    u16 level5Resources;
    u16 level6Resources;
    u16 level7Resources;

    u16 fuel;
    u16 fuelMax;
    u16 storage;
    u16 storageMax;

    u8  energyLevel;
    u16 energy;
    u16 energyMax;
    u16 energyXP;
    u16 energyXPNextLevel;

    u8  treadMillGameLastLevelUnlocked;
    u8  playerHasTrainedToday;
    u8  playerIsAbleToTrainToday;

    u16 radiation;
    u16 radiationMax;
    u8  radiationProtectionLevel;

    u8  convSoldier;

} SaveData;

extern SaveData gSave;

void saveSaveData(void);
void loadSaveData(void);
void resetSaveData(void);

#endif
