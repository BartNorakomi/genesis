#include <genesis.h>
#include <string.h>
#include "save_data.h"

//Girl appears only after 2+ games played
//CapGirl appears only after 4+ games played
//RedheadBoy appears only after 9+ games played
SaveData gSave = {
    .gamesPlayed     = 0,
    .convGirl        = 0b00000000,
    .convCapGirl     = 0b00000000,
    .convGingerBoy   = 0b00000000
};

void saveSaveData(void)
{
    //sm_save(slot, dataPointer, dataLength);
    //sm_save(0, &gSave, sizeof(SaveData));
    sm_save(0, &gSave, 1);
}

void loadSaveData(void)
{
    int16_t result = sm_load(0, &gSave, sizeof(SaveData));

    if (result < 0)
    {
        // No save exists → initialize defaults
        gSave.gamesPlayed = 0;
        saveSaveData();
    }
}