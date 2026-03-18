#include <genesis.h>
#include <string.h>
#include "save_data.h"

// I had a quick look at GPGX, it's not a standalone emulator ? What do I need to run/install it ?
// doragasu: Yup, as far as I know, it currently works only on Genesis Plus GX. You need a version more recent than Jan 14th or it will not work. I wrote a lengthy README file, I would recommend you give it a read (emulator support and many other things are explained there). Also make sure you are using latest version integrated into SGDK.
// https://github.com/Stephane-D/SGDK/blob/master/src/ext/flash-save/README.md
// I think they were now asking how to get GPGX running in the first place (since again it's only an emulation core without its own frontend)
// Most people use it along with Retroarch Install Retroarch, then deploy the up-to-date GPGX libretro core

// ---------------------------------------------------------
// DEFAULT VALUES (converted from your ASM block)
// ---------------------------------------------------------
SaveData gSave = {

    .totalLoopsDone = 0,
    .totalCoinsBasketball = 0,
    .ballsPurchased = 0,
    .currentBallsSelected = 0,

//0=arcadehall1, 1=arcadehall2, 2=biopod, 3=hydroponicsbay, 4=hangarbay, 5=trainingdeck, 6=reactorchamber, 7=sleepingquarters, 8=armoryvault, 9=holodeck, 10=medicalbay
//11=sciencelab, 12=drillinggame, 13=upgrademenu, 14=drillinglocations, 15=racinggame, 16=racing game title screen, 17=racing game level progress, 18=racing game congratulations
//19=basketball game, 20=penguin bike race, 21=blockhit game, 22=jumpdown game, 23=game over, 24=good ending, 25=title screen
    .currentRoom = 4,
    .gamesPlayed = 0,
    .highScoreTotalAverageAtLeast80Percent = 0,
    .highScoreBackroomGame = 0,

    .basketballCompletePercentage = 0,
    .highScoreBasketball = 0,

    .jumpDownCompletePercentage = 0,
    .highScoreJumpDown = 0,

    .blockHitCompletePercentage = 0,
    .highScoreBlockHit = 0,

    .bikeRaceCompletePercentage = 0,
    .highScoreBikeRace = 0,

    .penguinGameLevelHighest = 1,
    .penguinGameLapsHighest = 1,

    .neonHorizonDifficultyUnlocked = 0xFF,

    .youCanEndGameWithTrigBMessageDone = 0,
    .convGirl = 0,
    .convCapGirl = 0,
    .convGingerBoy = 0,

    .convHost = 0,
    .convEntity = 0x80,
    .convEntityShipExplanations = 0,

    .startWakeUpEvent = 1,

    .dailyContinuesUsed = 0,

    .totalMinutesUntilLand = 4320,

    .totalTimeUntilChildrenAreBornCounter1 = 0,
    .totalTimeUntilChildrenAreBornCounter2 = 0,
    .daysUntilChildrenAreBorn = 360 * 5,

    .year1234StatusUpdated = 0,
    .lowOnOxygenFoodWaterUpdated = 0,

    .oxygenGeneratorPurchased = 0,
    .waterRecyclerPurchased = 0,
    .colonyExpansionPurchased = 0,

    .oxygenOnShip = 250,
    .maxOxygenOnShip = 250,
    .foodOnShip = 300,
    .maxFoodOnShip = 300,
    .waterOnShip = 200,
    .maxWaterOnShip = 200,

    .amountOfDigSitesUnlocked = 5,

    .playerCollapsedNoOxygen = 0,
    .playerCollapsedNoFood = 0,
    .playerCollapsedNoWater = 0,
    .playerCollapsedHighRadiation = 0,
    .playerCollapsedNoFuel = 0,
    .playerCollapsedNoEnergy = 0,

    .totalCredits = 0,

    .conicalDrillBit = 0,
    .minerSpeedLevel = 1,
    .fuelTankLevel = 1,
    .cargoSizeLevel = 1,

    .level1Resources = 0,
    .level2Resources = 0,
    .level3Resources = 0,
    .level4Resources = 0,
    .level5Resources = 0,
    .level6Resources = 0,
    .level7Resources = 0,

    .fuel = FUEL_TANK_LEVEL1_MAX_FUEL,
    .fuelMax = FUEL_TANK_LEVEL1_MAX_FUEL,
    .storage = 0,
    .storageMax = CARGO_SIZE_LEVEL1_MAX_STORAGE,

    .energyLevel = 1,
    .energy = 50,
    .energyMax = 50,
    .energyXP = 0,
    .energyXPNextLevel = 100,

    .treadMillGameLastLevelUnlocked = 1,
    .playerHasTrainedToday = 0,
    .playerIsAbleToTrainToday = 1,

    .radiation = 0,
    .radiationMax = 200,
    .radiationProtectionLevel = 0,

    .convSoldier = 0
};

// ---------------------------------------------------------
// RESET SAVE DATA
// ---------------------------------------------------------
void resetSaveData(void)
{
    gSave.totalLoopsDone = 0;
    gSave.totalCoinsBasketball = 0;
    gSave.ballsPurchased = 0;
    gSave.currentBallsSelected = 0;

    gSave.currentRoom = 4;
    gSave.gamesPlayed = 0;
    gSave.highScoreTotalAverageAtLeast80Percent = 0;
    gSave.highScoreBackroomGame = 0;

    gSave.basketballCompletePercentage = 0;
    gSave.highScoreBasketball = 0;

    gSave.jumpDownCompletePercentage = 0;
    gSave.highScoreJumpDown = 0;

    gSave.blockHitCompletePercentage = 0;
    gSave.highScoreBlockHit = 0;

    gSave.bikeRaceCompletePercentage = 0;
    gSave.highScoreBikeRace = 0;

    gSave.penguinGameLevelHighest = 1;
    gSave.penguinGameLapsHighest = 1;

    gSave.neonHorizonDifficultyUnlocked = 0xFF;

    gSave.youCanEndGameWithTrigBMessageDone = 0;
    gSave.convGirl = 0;
    gSave.convCapGirl = 0;
    gSave.convGingerBoy = 0;

    gSave.convHost = 0;
    gSave.convEntity = 0b00000000; //bit 7=science lab ship explainer
//    gSave.convEntity = 0b10000000; //bit 7=science lab ship explainer
    gSave.convEntityShipExplanations = 0b00000000; //b7=medical bay , b6=holodeck, b5=armory vault, b4=sleeping quarters, b3=reactor chamber, b2=training deck, b1=hangar bay, b0=hydroponics
//    gSave.convEntityShipExplanations = 0b11111111; //b7=medical bay , b6=holodeck, b5=armory vault, b4=sleeping quarters, b3=reactor chamber, b2=training deck, b1=hangar bay, b0=hydroponics

    gSave.startWakeUpEvent = 1;

    gSave.dailyContinuesUsed = 0;

    gSave.totalMinutesUntilLand = 4320;

    gSave.totalTimeUntilChildrenAreBornCounter1 = 0;
    gSave.totalTimeUntilChildrenAreBornCounter2 = 0;
    gSave.daysUntilChildrenAreBorn = 360 * 5;

    gSave.year1234StatusUpdated = 0;
    gSave.lowOnOxygenFoodWaterUpdated = 0;

    gSave.oxygenGeneratorPurchased = 0;
    gSave.waterRecyclerPurchased = 0;
    gSave.colonyExpansionPurchased = 0;

    gSave.oxygenOnShip = 250;
    gSave.maxOxygenOnShip = 250;
    gSave.foodOnShip = 300;
    gSave.maxFoodOnShip = 300;
    gSave.waterOnShip = 200;
    gSave.maxWaterOnShip = 200;

    gSave.amountOfDigSitesUnlocked = 5;

    gSave.playerCollapsedNoOxygen = 0;
    gSave.playerCollapsedNoFood = 0;
    gSave.playerCollapsedNoWater = 0;
    gSave.playerCollapsedHighRadiation = 0;
    gSave.playerCollapsedNoFuel = 0;
    gSave.playerCollapsedNoEnergy = 0;

    gSave.totalCredits = 0;

    gSave.conicalDrillBit = 0;
    gSave.minerSpeedLevel = 1;
    gSave.fuelTankLevel = 1;
    gSave.cargoSizeLevel = 1;

    gSave.level1Resources = 0;
    gSave.level2Resources = 0;
    gSave.level3Resources = 0;
    gSave.level4Resources = 0;
    gSave.level5Resources = 0;
    gSave.level6Resources = 0;
    gSave.level7Resources = 0;

    gSave.fuel = FUEL_TANK_LEVEL1_MAX_FUEL;
    gSave.fuelMax = FUEL_TANK_LEVEL1_MAX_FUEL;
    gSave.storage = 0;
    gSave.storageMax = CARGO_SIZE_LEVEL1_MAX_STORAGE;

    gSave.energyLevel = 1;
    gSave.energy = 50;
    gSave.energyMax = 50;
    gSave.energyXP = 0;
    gSave.energyXPNextLevel = 100;

    gSave.treadMillGameLastLevelUnlocked = 1;
    gSave.playerHasTrainedToday = 0;
    gSave.playerIsAbleToTrainToday = 1;

    gSave.radiation = 0;
    gSave.radiationMax = 200;
    gSave.radiationProtectionLevel = 0;

    gSave.convSoldier = 0;

    saveSaveData();
}

// ---------------------------------------------------------
// SAVE ENTIRE STRUCT TO SRAM
// ---------------------------------------------------------
void saveSaveData(void)
{
    SRAM_enable();

    const u8* src = (const u8*)&gSave;
    for (u16 i = 0; i < sizeof(SaveData); i++)
        SRAM_writeByte(i, src[i]);

    SRAM_disable();
}

// ---------------------------------------------------------
// LOAD ENTIRE STRUCT FROM SRAM
// ---------------------------------------------------------
void loadSaveData(void)
{
    SRAM_enable();

    u8* dst = (u8*)&gSave;
    for (u16 i = 0; i < sizeof(SaveData); i++)
        dst[i] = SRAM_readByte(i);

    SRAM_disable();

    if (gSave.gamesPlayed == 0xFF)
        resetSaveData();
}
