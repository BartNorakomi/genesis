#include <genesis.h>
#include "resources.h"
#include "npc_dialogue.h"
#include "game_state.h"

#define TILE_DIALOGUE_BASE 600

// Textbox in tile coordinates
#define TEXT_X 14
#define TEXT_Y 2
#define TEXT_W 17
#define TEXT_H 12

// Palette range for PAL1
#define PAL1_FIRST   (PAL1 * 16)
#define PAL1_LAST    (PAL1_FIRST + 15)

static u16 prevJoy = 0;
static bool EndConversation = FALSE;   // <-- NEW FLAG

static const u16 blackPalette[16] = { 0 };

// ---------------------------------------------------------
// Dialogue script system
// ---------------------------------------------------------
typedef struct {
    const Image* portrait;
    const char* text;
} DialogueEntry;

static const DialogueEntry conversation100[] = {
    { &portrait_soldier, "Hello there, traveler! Welcome to the world of SGDK." },
    { &portrait_ai, "Hi, I'm AI! Nice to meet you." },
    { &portrait_soldier, "Nice to meet you too." },
};

static const DialogueEntry conversation1[] = {
    { &portrait_girl, 
      "Well, well, look who's back! Haven't seen your gears spinning in ages, mate!" },
    { &portrait_vessel, 
      "Oh, you know, been tangled up in life's cogs and wheels!" },
    { &portrait_girl, 
      "Hold onto your hat. Some ginger whirlwind's been smashing your records lately, leaving a trail of sparks!" },
    { &portrait_vessel, 
      "Seriously?!" },
    { &portrait_girl, 
      "Oh, absolutely! You've got a fiery challenger on your hands. Bet he'll strut in later to stir the pot!" },
};

static const u8 conversation1Count =
    sizeof(conversation1) / sizeof(DialogueEntry);

// ---------------------------------------------------------
// Wait for NEW press of A or B
// If B is pressed → EndConversation = TRUE
// ---------------------------------------------------------
static void waitForNewPressAorB()
{
    while (1)
    {
        u16 joy = JOY_readJoypad(JOY_1);

        bool newA = (joy & BUTTON_A) && !(prevJoy & BUTTON_A);
        bool newB = (joy & BUTTON_B) && !(prevJoy & BUTTON_B);

        prevJoy = joy;

        if (newA)
            return;

        if (newB)
        {
            EndConversation = TRUE;   // <-- NEW BEHAVIOR
            return;
        }

        SYS_doVBlankProcess();
    }
}

// ---------------------------------------------------------
// Clear only the text area
// ---------------------------------------------------------
static void clearTextArea()
{
    for (u16 ty = TEXT_Y; ty < TEXT_Y + TEXT_H; ty++)
        for (u16 tx = TEXT_X; tx < TEXT_X + TEXT_W; tx++)
            VDP_drawTextEx(BG_A, " ",
                TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, 0),
                tx, ty, FALSE);
}

// ---------------------------------------------------------
// Draw text with paging + skip animation
// ---------------------------------------------------------
void npcDialogueDrawText(const char* text)
{
    VDP_loadFont(myFont.tileset, DMA);

    u16 x = TEXT_X;
    u16 y = TEXT_Y;

    u16 i = 0;
    u16 len = strlen(text);

    bool skip = FALSE;

    while (i < len)
    {
        if (EndConversation) return;   // <-- NEW SAFETY CHECK

        if (text[i] == ' ')
        {
            i++;
            continue;
        }

        char word[64];
        u16 w = 0;

        while (i < len && text[i] != ' ' && text[i] != '\n')
            word[w++] = text[i++];

        word[w] = '\0';

        if (text[i] == '\n')
        {
            i++;
            x = TEXT_X;
            y++;

            if (y >= TEXT_Y + TEXT_H)
            {
                waitForNewPressAorB();
                if (EndConversation) return;

                clearTextArea();
                x = TEXT_X;
                y = TEXT_Y;
                skip = FALSE;
            }
        }

        if (x + w > TEXT_X + TEXT_W)
        {
            x = TEXT_X;
            y++;

            if (y >= TEXT_Y + TEXT_H)
            {
                waitForNewPressAorB();
                if (EndConversation) return;

                clearTextArea();
                x = TEXT_X;
                y = TEXT_Y;
                skip = FALSE;
            }
        }

        for (u16 k = 0; k < w; k++)
        {
            if (EndConversation) return;

            u16 joy = JOY_readJoypad(JOY_1);
            bool newA = (joy & BUTTON_A) && !(prevJoy & BUTTON_A);
            bool newB = (joy & BUTTON_B) && !(prevJoy & BUTTON_B);
            prevJoy = joy;

            if (newA)
                skip = TRUE;

            if (newB)
            {
                EndConversation = TRUE;   // <-- NEW BEHAVIOR
                skip = TRUE;
            }

            char buf[2] = { word[k], 0 };

            VDP_drawTextEx(BG_A, buf,
                TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, 0),
                x, y, FALSE);

            x++;

            if (!skip)
                SYS_doVBlankProcess();
        }

        if (text[i] == ' ')
        {
            if (x + 1 > TEXT_X + TEXT_W)
            {
                x = TEXT_X;
                y++;

                if (y >= TEXT_Y + TEXT_H)
                {
                    waitForNewPressAorB();
                    if (EndConversation) return;

                    clearTextArea();
                    x = TEXT_X;
                    y = TEXT_Y;
                    skip = FALSE;
                }
            }
            else
            {
                VDP_drawTextEx(BG_A, " ",
                    TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, 0),
                    x, y, FALSE);
                x++;
            }
            i++;
        }
    }
}

void npcDialogueOpenWindow(void)
{
    PAL_setPalette(PAL1, blackPalette, DMA);

    VDP_loadTileData(
        portrait_soldier.tileset->tiles,
        TILE_DIALOGUE_BASE,
        portrait_soldier.tileset->numTile,
        DMA
    );

    VDP_drawImageEx(
        BG_A,
        &portrait_soldier,
        TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, TILE_DIALOGUE_BASE),
        0, 1,
        FALSE,
        TRUE
    );
}

// ---------------------------------------------------------
// Show portrait
// ---------------------------------------------------------
static void npcDialogueShowPortrait(const Image* portrait)
{
    if (EndConversation) return;

    VDP_drawImageEx(
        BG_A,
        portrait,
        TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, TILE_DIALOGUE_BASE),
        0, 1,
        FALSE,
        TRUE
    );
    
    PAL_fadeIn(
        PAL1_FIRST,
        PAL1_LAST,
        portrait->palette->data,
        20,
        FALSE
    );
}

// ---------------------------------------------------------
// Close dialogue window
// ---------------------------------------------------------
void npcDialogueCloseWindow(void)
{
    VDP_clearTileMapRect(BG_A, 0, 1, 40, 28);
}

// ---------------------------------------------------------
// Main dialogue runner
// ---------------------------------------------------------
void runDialogue(u8 whichText)
{
    (void)whichText;

    EndConversation = FALSE;   // <-- RESET FLAG

    npcDialogueOpenWindow();

    for (u8 i = 0; i < conversation1Count; i++)
    {
        npcDialogueShowPortrait(conversation1[i].portrait);
        npcDialogueDrawText(conversation1[i].text);
        if (!EndConversation) waitForNewPressAorB();
        PAL_fadeOut(PAL1_FIRST, PAL1_LAST, 10, FALSE);
        if (EndConversation) break;
    }

    npcDialogueCloseWindow();
}
