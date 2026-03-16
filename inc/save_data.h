#ifndef SAVE_DATA_H
#define SAVE_DATA_H

typedef struct {
    u8 gamesPlayed;

    // Conversation flags
    u8 convGirl;        // %0000 0000
    u8 convCapGirl;     // %0000 0000
    u8 convGingerBoy;   // %0000 0000
} SaveData;

extern SaveData gSave;

void initSaveData(void);
void saveSaveData(void);
void loadSaveData(void);

#endif
