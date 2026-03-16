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

typedef struct {
    const DialogueEntry* entries;
    u8 count;
} ConversationDef;

// ---------------------------------------------------------
// Conversations
// ---------------------------------------------------------
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

// NEW: Conversation 014
static const DialogueEntry conversation014[] = {
    { &portrait_ai, "Welcome to your new world, traveler. This vessel is now your home. The seed of something greater. A beginning. You are the first colonist of Proxima Centauri b. Your task: awaken the future. Shape the colony. Make it endure." },
    { &portrait_vessel, "A new beginning... Okay. What's first?" },
    { &portrait_ai, "Begin by exploring the ship. Familiarize yourself with its systems, quarters, and decks. I am integrated into every corridor, every console. I will speak through the intercom - always watching, always listening. When you need guidance, ask. When you're lost, I will find you." },
    { &portrait_vessel, "Okay. That's... reassuring, I think." },
};

static const DialogueEntry conversation041[] = {
    { &portrait_vessel, "How are they doing?" },
    { &portrait_ai, "Embryonic life signs are stable. Nutrient intake and metabolic activity remain within optimal thresholds." },
    { &portrait_vessel, "How much time is left?" },
    { &portrait_ai, "  years, " },
    { &portrait_ai, "   months, " },
    { &portrait_ai, "   days." },
    { &portrait_vessel, "Feels faster every time I check..." },
    { &portrait_ai, "Temporal perception under duress often accelerates. But the count remains absolute." },
    { &portrait_vessel, "Right. No margin for error." },
    { &portrait_ai, "Correct. When the timer reaches zero, survival must be sustainable-or it will not be." },
};

// ---------------------------------------------------------
// Unified conversation table (IDs 0–50)
// ---------------------------------------------------------
static const ConversationDef conversations[51] = {

    // 0 unused
    { NULL, 0 },

    // 1–6 existing
    { conversation001, sizeof(conversation001) / sizeof(DialogueEntry) }, // 1
    { conversation002, sizeof(conversation002) / sizeof(DialogueEntry) }, // 2
    { conversation003, sizeof(conversation003) / sizeof(DialogueEntry) }, // 3
    { conversation004, sizeof(conversation004) / sizeof(DialogueEntry) }, // 4
    { conversation005, sizeof(conversation005) / sizeof(DialogueEntry) }, // 5
    { conversation006, sizeof(conversation006) / sizeof(DialogueEntry) }, // 6

    // 7–13 empty for now
    { NULL, 0 }, // 7
    { NULL, 0 }, // 8
    { NULL, 0 }, // 9
    { NULL, 0 }, // 10
    { NULL, 0 }, // 11
    { NULL, 0 }, // 12
    { NULL, 0 }, // 13

    // 14 = conversation014 (your intro)
    { conversation014, sizeof(conversation014) / sizeof(DialogueEntry) }, // 14

    // 15–40 empty
    { NULL, 0 }, // 15
    { NULL, 0 }, // 16
    { NULL, 0 }, // 17
    { NULL, 0 }, // 18
    { NULL, 0 }, // 19
    { NULL, 0 }, // 20
    { NULL, 0 }, // 21
    { NULL, 0 }, // 22
    { NULL, 0 }, // 23
    { NULL, 0 }, // 24
    { NULL, 0 }, // 25
    { NULL, 0 }, // 26
    { NULL, 0 }, // 27
    { NULL, 0 }, // 28
    { NULL, 0 }, // 29
    { NULL, 0 }, // 30
    { NULL, 0 }, // 31
    { NULL, 0 }, // 32
    { NULL, 0 }, // 33
    { NULL, 0 }, // 34
    { NULL, 0 }, // 35
    { NULL, 0 }, // 36
    { NULL, 0 }, // 37
    { NULL, 0 }, // 38
    { NULL, 0 }, // 39
    { NULL, 0 }, // 40

    // 41 = conversation041 (embryo dialogue)
    { conversation041, sizeof(conversation041) / sizeof(DialogueEntry) }, // 41

    // 42–50 empty
    { NULL, 0 }, // 42
    { NULL, 0 }, // 43
    { NULL, 0 }, // 44
    { NULL, 0 }, // 45
    { NULL, 0 }, // 46
    { NULL, 0 }, // 47
    { NULL, 0 }, // 48
    { NULL, 0 }, // 49
    { NULL, 0 }, // 50
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

        if (newA) return;
        if (newB) { EndConversation = TRUE; return; }

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

    // Remap MSX-style 041 to index 8
    u8 idx = whichText;
    if (whichText == 041)
        idx = 8;

    const ConversationDef* c = &conversations[idx];

    npcDialogueOpenWindow();

    for (u8 i = 0; i < c->count; i++)
    {
        npcDialogueShowPortrait(c->entries[i].portrait);
        npcDialogueDrawText(c->entries[i].text);

        if (!EndConversation)
            waitForNewPressAorB();

        PAL_fadeOut(PAL1_FIRST, PAL1_LAST, 10, FALSE);

        if (EndConversation)
            break;
    }

    npcDialogueCloseWindow();
}
