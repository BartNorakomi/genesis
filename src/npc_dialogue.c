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
static bool EndConversation = FALSE;

static const u16 blackPalette[16] = { 0 };

// ---------------------------------------------------------
// Dialogue script system
// ---------------------------------------------------------
typedef struct {
    const Image* portrait;
    const char* text;
} DialogueEntry;

static const DialogueEntry conversation001[] = {
    { &portrait_girl, "Well, well, look who's back! Haven't seen your gears spinning in ages, mate!" },
    { &portrait_vessel, "Oh, you know, been tangled up in life's cogs and wheels!" },
    { &portrait_girl, "Hold onto your hat. Some ginger whirlwind's been smashing your records lately, leaving a trail of sparks!" },
    { &portrait_vessel, "Seriously?!" },
    { &portrait_girl, "Oh, absolutely! You've got a fiery challenger on your hands. Bet he'll strut in later to stir the pot!" },
};

static const DialogueEntry conversation002[] = {
    { &portrait_girl, "Get out there and crank those record-breaking engines, you legend!" },
};

static const DialogueEntry conversation003[] = {
    { &portrait_capgirl, "Well, hotshot, what's the word on the street with you today?" },
    { &portrait_vessel, "I'm doing alright, I guess." },
    { &portrait_capgirl, "Just alright? C'mon, you've gotta bring more energy than that!" },
    { &portrait_vessel, "Heh, maybe I'm still waking up." },
    { &portrait_capgirl, "Then consider this your official hype-up call! Big plays only, got it?" },
    { &portrait_vessel, "Alright, alright, I'll try to keep up." },
    { &portrait_capgirl, "That's the spirit! Now let's see what you've got today!" },
};

static const DialogueEntry conversation004[] = {
    { &portrait_capgirl, "You're absolutely obliterating it out there, superstar!" },
    { &portrait_vessel, "Oh, thanks, that's cool!" },
    { &portrait_capgirl, "Hold the phone—I spotted a recruiter prowling around this morning!" },
    { &portrait_vessel, "A recruiter? What's the deal?" },
    { &portrait_capgirl, "Yeah, some ginger guy's been shattering records, and it's drawn some big-shot attention. Watch out!" },
};

static const DialogueEntry conversation005[] = {
    { &portrait_capgirl, "Rumor has it there's a hidden backroom with a mind-blowing game! Nail an average score above 80% across all games, and you're in. Exclusive access, baby!" },
};

static const DialogueEntry conversation006[] = {
    { &portrait_redheadboy, "Check this out—my average score's a slick 75%! Pretty epic, huh?" },
    { &portrait_vessel, "Whoa, that's insane!" },
    { &portrait_redheadboy, "You bet! Snagged a recruiter's eye, and he's hyping this backroom game—beat it, and it's a ticket to some top-secret government gig. I'm on fire!" },
};

static const DialogueEntry conversation100[] = {
    { &portrait_soldier, "Hello there, traveler! Welcome to the world of SGDK." },
    { &portrait_ai, "Hi, I'm AI! Nice to meet you." },
    { &portrait_soldier, "Nice to meet you too." },
};

// ---------------------------------------------------------
// Conversation lookup tables
// ---------------------------------------------------------
static const DialogueEntry* conversations[] = {
    NULL,              // 0 unused
    conversation001,   // 1
    conversation002,   // 2
    conversation003,   // 3
    conversation004,   // 4
    conversation005,   // 5
    conversation006,   // 6
    conversation100,   // 7
};

static const u8 conversationCounts[] = {
    0,
    sizeof(conversation001) / sizeof(DialogueEntry),
    sizeof(conversation002) / sizeof(DialogueEntry),
    sizeof(conversation003) / sizeof(DialogueEntry),
    sizeof(conversation004) / sizeof(DialogueEntry),
    sizeof(conversation005) / sizeof(DialogueEntry),
    sizeof(conversation006) / sizeof(DialogueEntry),
    sizeof(conversation100) / sizeof(DialogueEntry),
};

// ---------------------------------------------------------
// Wait for NEW press of A or B
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
            EndConversation = TRUE;
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
        if (EndConversation) return;

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

            if (newA || newB)
            {
                if (newB) EndConversation = TRUE;
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

// ---------------------------------------------------------
// Open dialogue window
// ---------------------------------------------------------
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
    EndConversation = FALSE;

    const DialogueEntry* convo = conversations[whichText];
    u8 count = conversationCounts[whichText];

    npcDialogueOpenWindow();

    for (u8 i = 0; i < count; i++)
    {
        npcDialogueShowPortrait(convo[i].portrait);
        npcDialogueDrawText(convo[i].text);

        if (!EndConversation)
            waitForNewPressAorB();

        PAL_fadeOut(PAL1_FIRST, PAL1_LAST, 10, FALSE);

        if (EndConversation)
            break;
    }

    npcDialogueCloseWindow();
}
