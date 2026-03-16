#include <genesis.h>
#include "resources.h"
#include "music.h"
#include "rooms.h"
#include "room_arcade1.h"
#include "player.h"
#include "game_state.h"
#include "room_sleepingquarters.h"
#include "save_data.h"

// ---------------------------------------------------------
// 1. Externs
// ---------------------------------------------------------
extern const u8* currentColMap;
extern u8 tileContent;

extern void drawRoomBackground(u8 room);
extern void drawDebugInfo(void);

// ---------------------------------------------------------
// 2. Local sprite pointers
// ---------------------------------------------------------
static Sprite* girlSprite;
static Sprite* capgirlSprite;
static Sprite* redheadboySprite;

static Sprite* arcade1WallSprite;
static Sprite* arcade1OpenDoorSprite;

static Sprite* fingerSprite;
static Sprite* triggerASprite;

static Sprite* convoCloudSprite;   // conversation cloud

// ---------------------------------------------------------
// 3. NPC positions
// ---------------------------------------------------------
static int girlX       = 50;
static int girlY       = 98;

static int capgirlX    = 160;
static int capgirlY    = 108;

static int redheadboyX = 240;
static int redheadboyY = 78;

// ---------------------------------------------------------
// 4. Prop positions
// ---------------------------------------------------------
static int wallX       = 156;
static int wallY       = 76;

static int openDoorX   = 108;
static int openDoorY   = 57;

// Trigger icon positions
static int triggerAX   = 210;
static int triggerAY   = 110;

// ---------------------------------------------------------
// 5. Arcade machine positions
// ---------------------------------------------------------
static const int arcadeMachineY[4] = { 63, 63, 64, 64 };
static const int arcadeMachineX[4] = {
    0x16 - 10 + 8,
    0x42 - 10 + 6 + 6,
    0xC6 - 10 - 10 + 2,
    0xF6 - 10 - 16 + 8
};

// ---------------------------------------------------------
// 6. Trigger icon state
// ---------------------------------------------------------
static int showPressAIcon = 0;
static u16 iconFrameCounter = 0;

// ---------------------------------------------------------
// 7. Depth sorting
// ---------------------------------------------------------
static void updateDepth(void)
{
    SPR_setDepth(arcade1WallSprite,     900);
    SPR_setDepth(arcade1OpenDoorSprite, 850);

    SPR_setDepth(fingerSprite,          780);
    SPR_setDepth(triggerASprite,        760);

    SPR_setDepth(playerSprite,      -playerY);

    if (gSave.gamesPlayed > 1)
        SPR_setDepth(girlSprite, -girlY);

    if (gSave.gamesPlayed > 3)
        SPR_setDepth(capgirlSprite, -capgirlY);

    if (gSave.gamesPlayed > 8)
        SPR_setDepth(redheadboySprite, -redheadboyY);

    SPR_setDepth(convoCloudSprite, 0);
}

// ---------------------------------------------------------
// 8. Check if player is near a machine
// ---------------------------------------------------------
static bool isPlayerNearMachine(int mx, int my)
{
    int dx = abs(playerX - mx);
    int dy = abs(playerY - my);
    return (dx < 16 && dy < 16);
}

// ---------------------------------------------------------
// 9. Check & show arcade machine A‑icon
// ---------------------------------------------------------
static void checkShowPressAIcon(void)
{
    for (int i = 0; i < 4; i++)
    {
        if (isPlayerNearMachine(arcadeMachineX[i], arcadeMachineY[i]))
        {
            showPressAIcon = i + 1;
            triggerAX = arcadeMachineX[i];
            triggerAY = arcadeMachineY[i];
            return;
        }
    }

    showPressAIcon = 0;
}

// ---------------------------------------------------------
// 10. Update A‑icon sprites
// ---------------------------------------------------------
static void updatePressAIconSprites(void)
{
    if (showPressAIcon == 0)
    {
        SPR_setVisibility(fingerSprite, HIDDEN);
        SPR_setVisibility(triggerASprite, HIDDEN);
        return;
    }

    SPR_setVisibility(fingerSprite, VISIBLE);
    SPR_setVisibility(triggerASprite, VISIBLE);

    int bob = (iconFrameCounter & 31) < 16 ? 1 : -1;

    SPR_setPosition(fingerSprite, triggerAX - 2, triggerAY - 29 + bob);
    SPR_setPosition(triggerASprite, triggerAX, triggerAY - 14);
}

// ---------------------------------------------------------
// 11. NPC proximity check
// ---------------------------------------------------------
static bool isPlayerNearNPC(int npcX, int npcY)
{
    int dx = abs(playerX - npcX);
    int dy = abs(playerY - npcY);
    return (dx < 20 && dy < 20);
}

// ---------------------------------------------------------
// 12. Unique NPC routines
// ---------------------------------------------------------

// GIRL NPC — 2‑stage conversation via bit 0 of convGirl
static void updateGirlNPC(void)
{
    if (gSave.gamesPlayed <= 1)
        return;

    if (isPlayerNearNPC(girlX, girlY))
    {
        SPR_setVisibility(convoCloudSprite, VISIBLE);
        SPR_setPosition(convoCloudSprite, girlX, girlY);

        if (JOY_readJoypad(JOY_1) & BUTTON_A)
        {
            if (gSave.convGirl & 0b00000001)
            {
                // bit 0 set → second conversation
                runDialogue(002);
            }
            else
            {
                // bit 0 not set → set it and run first conversation
                gSave.convGirl |= 0b00000001;
                runDialogue(001);
            }
        }
    }
}

// CAPGIRL NPC — multi‑stage conversation using bits 0 and 1 of convCapGirl
static void updateCapGirlNPC(void)
{
    if (gSave.gamesPlayed <= 3)
        return;

    if (isPlayerNearNPC(capgirlX, capgirlY))
    {
        SPR_setVisibility(convoCloudSprite, VISIBLE);
        SPR_setPosition(convoCloudSprite, capgirlX, capgirlY);

        if (JOY_readJoypad(JOY_1) & BUTTON_A)
        {
            u8* flags = &gSave.convCapGirl;

            // Case 1: bit 0 not set → first conversation, set bit 0
            if ((*flags & 0b00000001) == 0)
            {
                *flags |= 0b00000001;
                runDialogue(003);   // NPCConv003
                return;
            }

            // Case 2: bit 0 set, but gamesPlayed <= 6 → still NPCConv003
            if (gSave.gamesPlayed <= 6)
            {
                runDialogue(003);
                return;
            }

            // Case 3: gamesPlayed > 6 and bit 1 not set → set bit 1, NPCConv004
            if ((*flags & 0b00000010) == 0)
            {
                *flags |= 0b00000010;
                runDialogue(004);   // NPCConv004
                return;
            }

            // Case 4: bit 1 set → NPCConv005
            runDialogue(005);       // NPCConv005
        }
    }
}

// REDHEAD BOY NPC — currently single‑stage
static void updateRedBoyNPC(void)
{
    if (gSave.gamesPlayed <= 8)
        return;

    if (isPlayerNearNPC(redheadboyX, redheadboyY))
    {
        SPR_setVisibility(convoCloudSprite, VISIBLE);
        SPR_setPosition(convoCloudSprite, redheadboyX, redheadboyY);

        if (JOY_readJoypad(JOY_1) & BUTTON_A)
            runDialogue(006);
    }
}

// ---------------------------------------------------------
// 13. Main room logic
// ---------------------------------------------------------
GameState runArcade1(void)
{
    drawRoomBackground(ROOM_ARCADE1);
    playMusic(tune_arcadehall);

    SPR_reset();

    // Player sprite
    playerSprite = SPR_addSprite(&playerSpriteDef, playerX, playerY,
                                 TILE_ATTR(PAL2, FALSE, FALSE, FALSE));

    // Props
    arcade1WallSprite = SPR_addSprite(&arcade1WallSpriteDef, wallX, wallY,
                                      TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    arcade1OpenDoorSprite = SPR_addSprite(&arcade1OpenDoorSpriteDef, openDoorX, openDoorY,
                                          TILE_ATTR(PAL2, FALSE, FALSE, FALSE));

    // Trigger icons
    fingerSprite = SPR_addSprite(&fingerSpriteDef, triggerAX, triggerAY,
                                 TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    triggerASprite = SPR_addSprite(&triggerASpriteDef, triggerAX, triggerAY,
                                   TILE_ATTR(PAL2, FALSE, FALSE, FALSE));

    SPR_setVisibility(fingerSprite, HIDDEN);
    SPR_setVisibility(triggerASprite, HIDDEN);

    // NPCs
    girlSprite = SPR_addSprite(&girlSpriteDef,
                               girlX + PLAYERANDNPC_OFFSET_X,
                               girlY + PLAYERANDNPC_OFFSET_Y,
                               TILE_ATTR(PAL2, FALSE, FALSE, FALSE));

    capgirlSprite = SPR_addSprite(&capgirlSpriteDef,
                                  capgirlX + PLAYERANDNPC_OFFSET_X,
                                  capgirlY + PLAYERANDNPC_OFFSET_Y,
                                  TILE_ATTR(PAL2, FALSE, FALSE, FALSE));

    redheadboySprite = SPR_addSprite(&redheadboySpriteDef,
                                     redheadboyX + PLAYERANDNPC_OFFSET_X,
                                     redheadboyY + PLAYERANDNPC_OFFSET_Y,
                                     TILE_ATTR(PAL2, FALSE, FALSE, FALSE));

    // Hide NPCs based on gamesPlayed
    if (gSave.gamesPlayed <= 1)
        SPR_setVisibility(girlSprite, HIDDEN);

    if (gSave.gamesPlayed <= 3)
        SPR_setVisibility(capgirlSprite, HIDDEN);

    if (gSave.gamesPlayed <= 8)
        SPR_setVisibility(redheadboySprite, HIDDEN);

    // Conversation cloud
    convoCloudSprite = SPR_addSprite(&textCloudSpriteDef, 0, 0,
                                     TILE_ATTR(PAL2, FALSE, FALSE, FALSE));
    SPR_setVisibility(convoCloudSprite, HIDDEN);

    // -----------------------------------------------------
    // Main loop
    // -----------------------------------------------------
    while (1)
    {
        playerHandleInput();
        updateDepth();

        // Arcade machine logic
        checkShowPressAIcon();
        updatePressAIconSprites();

        // NPC logic
        SPR_setVisibility(convoCloudSprite, HIDDEN);
        updateGirlNPC();
        updateCapGirlNPC();
        updateRedBoyNPC();

        // Arcade machine A‑press
        if ((JOY_readJoypad(JOY_1) & BUTTON_A) && showPressAIcon != 0)
        {
            switch (showPressAIcon)
            {
                case 1: return STATE_JUMPQUEST;
                case 2: return STATE_BASKETBALL;
                case 3: return STATE_BLOCKCANNON;
                case 4: return STATE_BIKERACE;
            }
        }

        // Left exit → Arcade2
        if (playerY == 54)
        {
            playerX = 206;
            playerY = 118;
            return STATE_ARCADE2;
        }

        // Holodeck exit
        if (playerY == 125 || playerX == 0 || playerX == 254)
        {
            playerX = 144;
            playerY = 69;
            return STATE_HOLODECK;
        }

        // Start → Sleeping Quarters
        if (JOY_readJoypad(JOY_1) & BUTTON_START)
            return STATE_SLEEPINGQUARTERS;

        drawDebugInfo();
        playerUpdateSprite();

        SPR_update();
        SYS_doVBlankProcess();

        iconFrameCounter++;
    }

    return STATE_QUIT;
}
