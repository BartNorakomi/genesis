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

// Helper: palette index range for PAL1
#define PAL1_FIRST   (PAL1 * 16)
#define PAL1_LAST    (PAL1_FIRST + 15)

// Input state for new-press detection
static u16 prevJoy = 0;

// ---------------------------------------------------------
// Dialogue texts
// ---------------------------------------------------------
const char* DIALOGUE_1 =
    "Hello there, traveler! Welcome to the world of SGDK. "
    "This ancient land is filled with secrets, forgotten ruins, "
    "and strange creatures that wander beneath the moonlit sky. "
    "Your journey is only beginning.";

const char* DIALOGUE_2 =
    "You feel it too, don't you? The air is heavy tonight. "
    "Something ancient stirs beneath the soil, whispering through "
    "the cracks of forgotten temples. "
    "Tread carefully, traveler... not all shadows are empty.";

const char* DIALOGUE_3 =
    "Ah, there you are! I was hoping you'd arrive soon. "
    "The others have already begun their preparations. "
    "If you wish to stand among them, you'll need courage, "
    "a sharp mind, and a heart that refuses to yield.";


// ---------------------------------------------------------
// Clear only the text area by writing spaces
// ---------------------------------------------------------
static void clearTextArea()
{
    for (u16 ty = TEXT_Y; ty < TEXT_Y + TEXT_H; ty++)
    {
        for (u16 tx = TEXT_X; tx < TEXT_X + TEXT_W; tx++)
        {
            VDP_drawTextEx(
                BG_A,
                " ",
                TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, 0),
                tx,
                ty,
                FALSE
            );
        }
    }
}

// ---------------------------------------------------------
// Wait for a NEW press of A
// ---------------------------------------------------------
static void waitForNewPressA()
{
    while (1)
    {
        u16 joy = JOY_readJoypad(JOY_1);
        bool newPressA = (joy & BUTTON_A) && !(prevJoy & BUTTON_A);
        prevJoy = joy;

        if (newPressA)
            break;

        SYS_doVBlankProcess();
    }
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
        // Skip leading spaces
        if (text[i] == ' ')
        {
            i++;
            continue;
        }

        // Extract next word
        char word[64];
        u16 w = 0;

        while (i < len && text[i] != ' ' && text[i] != '\n')
        {
            word[w++] = text[i++];
        }
        word[w] = '\0';

        // Handle newline
        if (text[i] == '\n')
        {
            i++;
            x = TEXT_X;
            y++;

            if (y >= TEXT_Y + TEXT_H)
            {
                waitForNewPressA();
                clearTextArea();
                x = TEXT_X;
                y = TEXT_Y;
                skip = FALSE;
            }
        }

        // Check if word fits
        if (x + w > TEXT_X + TEXT_W)
        {
            x = TEXT_X;
            y++;

            if (y >= TEXT_Y + TEXT_H)
            {
                waitForNewPressA();
                clearTextArea();
                x = TEXT_X;
                y = TEXT_Y;
                skip = FALSE;
            }
        }

        // Draw letters
        for (u16 k = 0; k < w; k++)
        {
            u16 joy = JOY_readJoypad(JOY_1);
            bool newPressA = (joy & BUTTON_A) && !(prevJoy & BUTTON_A);
            prevJoy = joy;

            if (newPressA)
                skip = TRUE;

            char buf[2] = { word[k], 0 };

            VDP_drawTextEx(
                BG_A,
                buf,
                TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, 0),
                x,
                y,
                FALSE
            );

            x++;

            if (!skip)
                SYS_doVBlankProcess();
        }

        // Add space
        if (text[i] == ' ')
        {
            if (x + 1 > TEXT_X + TEXT_W)
            {
                x = TEXT_X;
                y++;

                if (y >= TEXT_Y + TEXT_H)
                {
                    waitForNewPressA();
                    clearTextArea();
                    x = TEXT_X;
                    y = TEXT_Y;
                    skip = FALSE;
                }
            }
            else
            {
                VDP_drawTextEx(
                    BG_A,
                    " ",
                    TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, 0),
                    x,
                    y,
                    FALSE
                );
                x++;
            }
            i++;
        }
    }
}

// ---------------------------------------------------------
// Open dialogue window
// ---------------------------------------------------------
void npcDialogueOpenWindow(const Image* portrait)
{
    u16 blackPal[16] = { 0 };
    PAL_setPalette(PAL1, blackPal, CPU);

    VDP_loadTileData(
        portrait->tileset->tiles,
        TILE_DIALOGUE_BASE,
        portrait->tileset->numTile,
        DMA
    );

    VDP_drawImageEx(
        BG_A,
        portrait,
        TILE_ATTR_FULL(PAL1, TRUE, FALSE, FALSE, TILE_DIALOGUE_BASE),
        0, 1,
        FALSE,
        TRUE
    );

    for (u16 i = 0; i < 10; i++)
        SYS_doVBlankProcess();

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
    PAL_fadeOut(
        PAL1_FIRST,
        PAL1_LAST,
        20,
        FALSE
    );

    VDP_clearTileMapRect(BG_A, 0, 1, 40, 28);
}

// ---------------------------------------------------------
// Main dialogue runner
// ---------------------------------------------------------
void runDialogue(const Image* portrait, u8 whichText)
{
    npcDialogueOpenWindow(portrait);

    switch (whichText)
    {
        case 1: npcDialogueDrawText(DIALOGUE_1); break;
        case 2: npcDialogueDrawText(DIALOGUE_2); break;
        case 3: npcDialogueDrawText(DIALOGUE_3); break;
        default: npcDialogueDrawText("Invalid dialogue ID."); break;
    }

    // Wait for B to close
    while (1)
    {
        u16 joy = JOY_readJoypad(JOY_1);
        prevJoy = joy;

        if (joy & BUTTON_B)
        {
            npcDialogueCloseWindow();
            return;
        }

        SYS_doVBlankProcess();
    }
}
