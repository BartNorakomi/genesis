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

// NPCConv007
static const DialogueEntry conversation007[] = {
    { &portrait_host,   "Congratulations. Not many manage to crack the 80% mark. You've been noticed." },
    { &portrait_vessel, "Uh... noticed by who exactly?" },
    { &portrait_host,   "Let's just say we've got eyes on talent. And you? You're showing potential for something... classified." },
    { &portrait_vessel, "Wait—what kind of 'classified'?" },
    { &portrait_host,   "One more test. Behind that door. Beat it, and we'll talk next steps. Government clearance might be in your future. Follow me!" },
};

// NPCConv008
static const DialogueEntry conversation008[] = {
    { &portrait_host,   "So... you made it past the floorboards and flashing lights. Impressive. Few even realize this place exists." },
    { &portrait_vessel, "Who are you? What is this place?" },
    { &portrait_host,   "Just someone who's been watching. Waiting. This is the backroom. The game behind the games. The one that doesn't hand out tickets or high scores—just truth." },
    { &portrait_vessel, "What kind of game are we talking about?" },
    { &portrait_host,   "Not a game. A final test. The others were warm-ups—reflexes, pattern reading, persistence. This one digs deeper." },
    { &portrait_host,   "It pushes focus, memory, and control under stress. Each run is all or nothing. No continues." },
    { &portrait_host,   "But you're free to try again—and again—until the pattern reveals itself. Here's the interface; simply reach out and take control." },
    { &portrait_vessel, "But... I don't see any controls." },
    { &portrait_host,   "This game doesn't need controls. You'll guide it with your thoughts alone." },
    { &portrait_vessel, "Control it... with my mind? How is that even possible?" },
    { &portrait_host,   "Some things aren't meant to be explained. Just... see for yourself." },
};

// NPCConv009
static const DialogueEntry conversation009[] = {
    { &portrait_host, "Step up. If you're ready, the final game is waiting." },
};

// NPCConv010
static const DialogueEntry conversation010[] = {
    { &portrait_ai,     "You have successfully completed all required training protocols. The simulation is now concluded. It is time to reveal the reality." },
    { &portrait_vessel, "Who... who are you?" },
    { &portrait_ai,     "A guide. A caretaker. The one who's walked beside you through every challenge—though you did not know it." },
    { &portrait_vessel, "Wait... what do you mean, 'reveal the reality'?" },
    { &portrait_ai,     "I know this may be disorienting. But you're ready to understand now." },
    { &portrait_vessel, "What are you talking about? This place... all of it—I've lived it. I've felt everything. How could none of it be real?" },
    { &portrait_ai,     "It is easy to believe something is real while dreaming. The mind accepts even the extraordinary without question when immersed in dream logic." },
    { &portrait_ai,     "That is why the simulation was constructed within your subconscious—so that you could learn without resistance." },
    { &portrait_vessel, "So... this was all in my head?" },
    { &portrait_ai,     "What you experienced was not illusion—it was preparation." },
    { &portrait_ai,     "You are fifteen years old, preserved in stasis aboard an interstellar colony ship bound for Proxima Centauri b." },
    { &portrait_ai,     "Earth has become uninhabitable. A dozen sister vessels were launched toward other nearby star systems." },
    { &portrait_ai,     "You were conceived through ectogenesis—grown outside the womb in a biopod engineered for long-range survival." },
    { &portrait_ai,     "For over a decade, I have shaped your mind within this dream-state, preparing you for the world that awaits." },
    { &portrait_ai,     "Soon, you will awaken—alone—on a new world. You must construct, sustain, and grow the colony. I cannot do it without you." },
    { &portrait_ai,     "Your first priority upon awakening will be to gather essential resources to begin establishing the colony." },
    { &portrait_ai,     "Together, we will secure the future of humanity." },
};

// NPCConv011
static const DialogueEntry conversation011[] = {
    { &portrait_vessel, "So... what do I do now?" },
    { &portrait_ai,     "You wait. The ship is on final approach to orbit. Atmospheric entry will occur in three days, subjective time." },
    { &portrait_ai,     "During this period, I recommend additional training. There is still knowledge to absorb. Skills to refine." },
    { &portrait_vessel, "You just told me none of this is real. What's the point?" },
    { &portrait_ai,     "The consequences may be simulated. But the learning is real. Every reflex and decision will remain with you." },
    { &portrait_vessel, "Can't I just... wake up now?" },
    { &portrait_ai,     "No. Your biological body is undergoing final neural integration. Premature emergence could cause permanent harm." },
    { &portrait_ai,     "Trust the process. This dream—this world—is still yours for a little while longer." },
    { &portrait_vessel, "Alright." },
    { &portrait_ai,     "You may begin when ready. I will monitor your progress... and be here if you have questions." },
};

// NPCConv013
static const DialogueEntry conversation013[] = {
    { &portrait_ai, "We have arrived. Proxima Centauri b—orbital insertion successful. Atmospheric analysis: within tolerable thresholds." },
    { &portrait_ai, "Surface stability: promising. Radiation: manageable." },
    { &portrait_ai, "I will begin the revival sequence. Slow. Gentle. As designed. Your vitals remain strong." },
    { &portrait_ai, "Wake up, traveler. It's time." },
};

// NEW: Conversation 014
static const DialogueEntry conversation014[] = {
    { &portrait_ai, "Welcome to your new world, traveler. This vessel is now your home. The seed of something greater. A beginning. You are the first colonist of Proxima Centauri b. Your task: awaken the future. Shape the colony. Make it endure." },
    { &portrait_vessel, "A new beginning... Okay. What's first?" },
    { &portrait_ai, "Begin by exploring the ship. Familiarize yourself with its systems, quarters, and decks. I am integrated into every corridor, every console. I will speak through the intercom - always watching, always listening. When you need guidance, ask. When you're lost, I will find you." },
    { &portrait_vessel, "Okay. That's... reassuring, I think." },
};

// NPCConv015 – Hydroponics Bay
static const DialogueEntry conversation015[] = {
    { &portrait_ai,     "This is the Hydroponics Bay, where we grow the colony's food supply in controlled conditions." },
    { &portrait_ai,     "Plants are cultivated using nutrient-rich water systems, ensuring steady production regardless of external climate." },
    { &portrait_ai,     "Maintaining this bay is critical. A failure here could threaten the entire colony's sustenance." },
    { &portrait_vessel, "So, this is basically the colony's garden, sheltered from the planet's extremes." },
    { &portrait_ai,     "Precisely. Healthy crops mean a healthy colony." },
};

// NPCConv016 – Hangar Bay
static const DialogueEntry conversation016[] = {
    { &portrait_ai,     "This is the Hangar Bay. The drilling machine stationed here is the core apparatus for extracting Proxima Centauri b's valuable subterranean materials." },
    { &portrait_ai,     "Board the drill and navigate the planet's terrain to collect materials needed for upgrades and colony construction." },
    { &portrait_ai,     "The drilling machine protects you from hazardous conditions outside, but resources vary in difficulty to extract—better equipment means deeper and richer finds." },
    { &portrait_vessel, "So, this is where the real digging begins." },
    { &portrait_ai,     "Exactly. Master the drill to unlock the future of the colony." },
};

// NPCConv017 – Training Deck
static const DialogueEntry conversation017[] = {
    { &portrait_ai,     "This is the Training Deck. After fifteen years in stasis, your body is weak and unprepared for Proxima Centauri b's gravity, which is 30% stronger than Earth's." },
    { &portrait_ai,     "Here, you will rebuild your strength, stamina, and endurance—vital for survival and extended operations outside the ship." },
    { &portrait_ai,     "Physical conditioning reduces fatigue and increases your time on the surface, allowing you to perform your tasks more efficiently." },
    { &portrait_vessel, "Sounds tough, but necessary." },
    { &portrait_ai,     "It is. There is no shortcut. I will monitor your progress and guide you, but the effort is yours." },
    { &portrait_vessel, "No yelling, I hope." },
    { &portrait_ai,     "Only observation. And silent support." },
};

// NPCConv018 – Reactor Chamber
static const DialogueEntry conversation018[] = {
    { &portrait_vessel, "This place feels like the ship's beating heart." },
    { &portrait_ai,     "It is. The reactor powers everything—life support, fabrication, propulsion, defense." },
    { &portrait_ai,     "A fusion core held stable by magnetic confinement. Extremely efficient, but volatile if mishandled." },
    { &portrait_vessel, "So, one wrong move and it's game over?" },
    { &portrait_ai,     "Essentially. Your access is limited to consoles and maintenance drones. The core itself remains sealed unless emergency repairs are necessary." },
    { &portrait_vessel, "Glad I won't have to wear a hazmat suit in there." },
    { &portrait_ai,     "Caution is constant. But so is trust—in the systems and in you." },
};

// NPCConv019 – Sleeping Quarters
static const DialogueEntry conversation019[] = {
    { &portrait_vessel, "Looks like this is where I sleep, huh?" },
    { &portrait_ai,     "Correct. These quarters are equipped with an adjustable stasis-grade sleep pod. Designed for efficiency, not comfort—but it serves its purpose." },
    { &portrait_ai,     "Sleep restores your energy reserves and accelerates cellular repair. Without it, fatigue compounds, and performance degrades rapidly." },
    { &portrait_vessel, "So... collapse here when I feel like a dead battery?" },
    { &portrait_ai,     "Precisely. Sleep is not a weakness—it is maintenance. Deny it, and you sabotage your own endurance." },
    { &portrait_vessel, "I'll keep that in mind. Not much use to the colony if I pass out mid-shift." },
    { &portrait_ai,     "A functioning colonist is a rested colonist." },
};

// NPCConv020 – Armory Vault
static const DialogueEntry conversation020[] = {
    { &portrait_vessel, "Okay... this place looks serious. What's all this gear for?" },
    { &portrait_ai,     "This is the Armory Vault. A secure repository of tactical equipment—protective suits, energy tools, and modular weapon systems." },
    { &portrait_ai,     "While the colony's goal is peace and survival, unpredictability remains a constant. Terrain hazards, structural failures... or threats we have not yet classified." },
    { &portrait_vessel, "You're saying I might have to fight?" },
    { &portrait_ai,     "I am saying you must be prepared. The vault's contents are restricted and unlocked based on need. Some tools are for survival. Others... for defense." },
    { &portrait_vessel, "And here I thought this would be a farming mission." },
    { &portrait_ai,     "Farming, mining, surviving. Each requires its own kind of weaponry." },
};

// NPCConv021 – Holodeck
static const DialogueEntry conversation021[] = {
    { &portrait_vessel, "This place looks... different. Like a game room?" },
    { &portrait_ai,     "Correct. This is the Holodeck—a simulation environment designed for training and recreation." },
    { &portrait_ai,     "Here, you can revisit the arcade games from the start of your journey. Playing them again grants additional experience and sharpens your reflexes." },
    { &portrait_vessel, "So, it's like a virtual playground and a skill gym all in one." },
    { &portrait_ai,     "Exactly. Use it to refine your abilities without risk. The colony depends on your growth, both mental and physical." },
    { &portrait_vessel, "Sounds like a nice break from drilling and repairs." },
    { &portrait_ai,     "Balance is key. Even in isolation, play fuels progress." },
};

// NPCConv022 – Medical Bay
static const DialogueEntry conversation022[] = {
    { &portrait_vessel, "This place looks sterile... and a little intimidating." },
    { &portrait_ai,     "This is the Medical Bay. Designed to diagnose and treat a range of conditions, including radiation exposure." },
    { &portrait_ai,     "When you leave the ship to drill on Proxima Centauri b's surface, radiation levels rise unpredictably. Prolonged exposure without treatment risks sickness or worse." },
    { &portrait_vessel, "So this is my safe zone when things go sideways outside?" },
    { &portrait_ai,     "Correct. The Medbay stabilizes your condition, administers antidotes, and monitors your recovery." },
    { &portrait_vessel, "Good to know there's a backup plan if the planet decides to cook me." },
    { &portrait_ai,     "Precaution is survival's best ally." },
};

// NPCConv023 – Science Lab
static const DialogueEntry conversation023[] = {
    { &portrait_vessel, "This is the Science Lab. Looks like where the real upgrades happen." },
    { &portrait_ai,     "Correct. Resources gathered during drilling missions are brought here for analysis and synthesis." },
    { &portrait_ai,     "You'll encounter various materials underground. Granite is common—always drillable. Stone requires a level 2 drill cone upgrade. Metals need level 3, and diamonds demand level 4." },
    { &portrait_vessel, "So the better my drill, the deeper and richer the haul." },
    { &portrait_ai,     "Exactly. Resources are allocated to multiple critical systems—drill speed, drill cone strength, oxygen capacity, and ship reserves for oxygen, food, and water." },
    { &portrait_ai,     "Upgrading the Science Lab itself boosts overall efficiency, increasing food production in the Hydroponics Bay and accelerating research." },
    { &portrait_vessel, "And eventually, I can build out the colony on the planet, using these materials?" },
    { &portrait_ai,     "Yes. Colony structures mirror ship facilities, replicating vital systems for a sustainable settlement." },
    { &portrait_vessel, "Looks like the key to survival is drilling, upgrading, and building." },
    { &portrait_ai,     "Survival is progress. Progress is survival." },
};

// NPCConv024 – First Time Drilling
static const DialogueEntry conversation024[] = {
    { &portrait_ai,      "Before we begin your first descent, there's someone you should meet. For guidance in hazardous conditions, I've generated a support construct. Presenting: the Sergeant." },
    { &portrait_soldier, "Sergeant, online. I'll be overseeing your drilling operations." },
    { &portrait_vessel,  "Hello... I....." },
    { &portrait_vessel,  "Sorry. It's just good to see a new face." },
    { &portrait_ai,      "Expected. You've been the only conscious human here. Additional social presence improves stability." },
    { &portrait_soldier, "If that helps, good. But my job is to keep you alive, not comfort you." },
    { &portrait_ai,      "Sergeant, briefing." },
    { &portrait_soldier, "You'll start near the surface, safe enough to learn the drill. Deeper layers get tougher, but yield better resources." },
    { &portrait_soldier, "Your current gear can't handle the deeper strata." },
    { &portrait_soldier, "Upgrades from the science lab will be required." },
    { &portrait_soldier, "Fuel and energy are critical. Run out, and you're stranded. And outside the ship? Radiation. Visit the medical bay daily." },
    { &portrait_vessel,  "So I can't just dig straight down?" },
    { &portrait_soldier, "Correct. Gather, return, upgrade. Dig smart and come back alive." },
    { &portrait_vessel,  "Understood." },
    { &portrait_soldier, "Good. Bring back enough, and together we'll forge a colony worthy of the Children's future." },
};

// NPCConv025 – Low Fuel Warning (Urgent Return)
static const DialogueEntry conversation025[] = {
    { &portrait_soldier, "Warning: Fuel reserves critically low. Drill operation cannot continue." },
    { &portrait_soldier, "You must abort mining and ascend immediately. Remaining underground is a fatal risk." },
    { &portrait_vessel,  "Copy that. Beginning ascent now." },
    { &portrait_soldier, "Good. When you reach the surface, refuel and recover. The colony depends on you." },
};

// NPCConv026 – Low Fuel Warning (Short)
static const DialogueEntry conversation026[] = {
    { &portrait_soldier, "Fuel critical. Abort mission and return now." },
    { &portrait_vessel,  "Copy. Ascending." },
};

// NPCConv027 – Low Energy Warning (Urgent Return)
static const DialogueEntry conversation027[] = {
    { &portrait_soldier, "Body scans indicate your energy levels are critically low. Risk of fatigue or loss of consciousness is high." },
    { &portrait_soldier, "You must abort the drilling operation and ascend immediately to prevent collapse." },
    { &portrait_vessel,  "Understood. Initiating ascent now." },
    { &portrait_soldier, "Remember: no extraction is worth risking your life. Return alive, return strong. Once on the surface, proceed directly to your sleeping quarters to rest and fully restore your energy." },
    { &portrait_vessel,  "Copy that." },
};

// NPCConv028 – Low Energy Warning (Short)
static const DialogueEntry conversation028[] = {
    { &portrait_soldier, "Energy levels critical. You need to return immediately." },
    { &portrait_soldier, "Rest in your quarters as soon as you're topside." },
    { &portrait_vessel,  "Understood. Heading up." },
};

// NPCConv029 – High Radiation Warning (Urgent, Encouraging)
static const DialogueEntry conversation029[] = {
    { &portrait_soldier, "Warning: Your radiation levels are dangerously high. This isn't something to ignore." },
    { &portrait_soldier, "You need to stop drilling and get back to the surface right now. Your health comes first." },
    { &portrait_vessel,  "Understood. Heading back now." },
    { &portrait_soldier, "No mission is worth risking your life. Upon reaching the surface, report to the medical bay immediately for decontamination and care." },
    { &portrait_vessel,  "Got it. I'll go to the med bay as soon as I'm back." },
    { &portrait_soldier, "Good. Stay strong—we need you healthy for what's ahead." },
};

// NPCConv030 – Storage Full (Return to Surface)
static const DialogueEntry conversation030[] = {
    { &portrait_soldier, "Storage at maximum capacity. We can't hold any more resources right now." },
    { &portrait_soldier, "You've done an amazing job down there—time to head back to the surface." },
    { &portrait_soldier, "Any additional resources you drill now will be lost. The drill will pulverize them and leave the dust behind—no way to retrieve it later." },
    { &portrait_vessel,  "Understood. Returning with the haul." },
    { &portrait_soldier, "Bring those resources home safely. The colony depends on what you've gathered." },
    { &portrait_soldier, "Rest up when you get back. We'll gear up and plan the next dive soon." },
};

// NPCConv031 – Storage Full (Short)
static const DialogueEntry conversation031[] = {
    { &portrait_soldier, "Storage full. We can't carry any more. Extra resources will be crushed and lost for good." },
};

// NPCConv032 – Drill Can't Break Material
static const DialogueEntry conversation032[] = {
    { &portrait_soldier, "Hold it. That material's too dense for your current drill. You'll need an upgrade before you can cut through it." },
};

// NPCConv033 – Lava Encountered
static const DialogueEntry conversation033[] = {
    { &portrait_soldier, "Caution—magma flow detected. Temperature exceeds structural limits of all known drill alloys. Passage is impossible. Reroute your path immediately." },
};

// NPCConv034 – Out of Fuel (Game Over)
static const DialogueEntry conversation034[] = {
    { &portrait_soldier, "OUT OF FUEL. MISSION FAILED. GAME OVER!" },
};

// NPCConv035 – Out of Energy (Game Over)
static const DialogueEntry conversation035[] = {
    { &portrait_soldier, "OUT OF ENERGY. MISSION FAILED. GAME OVER!" },
};

// NPCConv036 – Radiation Full (Game Over)
static const DialogueEntry conversation036[] = {
    { &portrait_soldier, "DEATH BY RADIATION. MISSION FAILED. GAME OVER!" },
};

// NPCConv037 – Resource Offload Summary
static const DialogueEntry conversation037[] = {
    { &portrait_ai, "Resource extraction complete. Initializing cargo offload sequence..." },
    { &portrait_ai, "Scanning inventory..." },
    { &portrait_ai, "Processing materials..." },
    { &portrait_ai, "Calculating market valuation..." },
    { &portrait_ai, "Offload Summary:" },
    { &portrait_ai, "Coal x 128 units - (2 Cr/unit) = 256 Credits" },
    { &portrait_ai, "Ironium x 94 units - 5 Cr/unit - 470 Credits" },
    { &portrait_ai, "Bronzium x 61 units - 8 Cr/unit - 488 Credits" },
    { &portrait_ai, "Silverium x 37 units - 12 Cr/unit - 444 Credits" },
    { &portrait_ai, "Goldium x 22 units - 18 Cr/unit - 396 Credits" },
    { &portrait_ai, "Emerald x 9 units - 30 Cr/unit - 270 Credits" },
    { &portrait_ai, "Diamond x 3 units - 50 Cr/unit - 150 Credits" },
    { &portrait_ai, "Total Value: 2,474 Credits" },
    { &portrait_ai, "Resource containment pods depressurized. Materials transferred to storage silos." },
    { &portrait_ai, "Market-ready and logged for trade routing." },
};

// NPCConv038 – Resource Offload (Blank Portrait)
static const DialogueEntry conversation038[] = {
//    { &portrait_resourceoffload, " " },
};

// NPCConv039 – Sleeping Quarters (>50% Energy)
static const DialogueEntry conversation039[] = {
    { &portrait_vessel, "Alright... time to grab some sleep." },
    { &portrait_ai,     "Sleep request denied. Current energy reserves are above 50%." },
    { &portrait_ai,     "While rest is essential, it is not yet necessary. You remain within peak operational thresholds." },
    { &portrait_vessel, "I was hoping for a short reset. Just to clear my head." },
    { &portrait_ai,     "Understood. But with colony systems still below sustainability benchmarks, resource allocation favors active deployment. Recommendation: return to mission-critical tasks—mining, training, or structural expansion." },
    { &portrait_vessel, "So rest is off the table for now?" },
    { &portrait_ai,     "Correct. You will be notified when rest becomes mission-appropriate." },
    { &portrait_ai,     "Maintain focus. The colony's success depends on continued forward momentum." },
    { &portrait_vessel, "Acknowledged. Returning to duty." },
};

// NPCConv040 – Breeding Room
static const DialogueEntry conversation040[] = {
    { &portrait_ai,     "Planetary conditions meet minimum viable thresholds. Atmospheric oxygen: sufficient. Radiation: within acceptable limits. Surface temperature: survivable." },
    { &portrait_ai,     "Embryonic gestation cycle has commenced. Five years until the Children are born." },
    { &portrait_vessel, "Wait—already? Why not wait a few years? Stabilize the systems first?" },
    { &portrait_ai,     "Delay is not possible. The gestation protocol was pre-seeded during transit. Once planetary viability was confirmed, life support for the next generation activated automatically." },
    { &portrait_ai,     "Embryonic systems are now integrated with the colony's core: power grid, nutrient flow, and thermal regulation. Any interruption would compromise cellular integrity." },
    { &portrait_vessel, "Couldn't we have planned for a safer window?" },
    { &portrait_ai,     "Negative. This planet's environmental cycles provide a narrow five-year band of relative stability. Afterward: solar radiation escalation, temperature spikes, and atmospheric volatility." },
    { &portrait_ai,     "If core systems are not operational by the time of birth, the Children will not survive." },
    { &portrait_ai,     "They are not a possibility. They are a certainty—or a failure." },
    { &portrait_ai,     "Your task is not to begin the future. It has already begun. Your task is to ensure it arrives." },
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

// NPCConv043 – First Time Inside Holodeck Simulation
static const DialogueEntry conversation043[] = {
    { &portrait_vessel, "Wow, this place looks exactly like it did in my dreams." },
    { &portrait_ai,     "Your memory engrams were highly detailed. The simulation rendered them with 97.4% accuracy." },
    { &portrait_vessel, "Even the hum of the machines feels right. I used to lose hours in here." },
    { &portrait_ai,     "Time spent here is not lost. Familiar spaces promote cognitive resilience and motor recalibration." },
    { &portrait_vessel, "Right. Mental tune-up through muscle memory." },
    { &portrait_ai,     "Proceed when ready. Exiting the boundaries will return you to the present." },
    { &portrait_vessel, "Just for a moment... I think I'll stay lost." },
};

// NPCConv044 – Training Room (Low Energy)
static const DialogueEntry conversation044[] = {
    { &portrait_vessel, "Alright, let's start a training session." },
    { &portrait_ai,     "Training request denied. Current energy reserves are too low." },
    { &portrait_ai,     "Engaging in physical drills now would risk structural failure and reduce overall efficiency." },
    { &portrait_vessel, "I just need a quick workout to stay sharp." },
    { &portrait_ai,     "Not advisable. Energy levels must be sufficient before training is authorized." },
    { &portrait_ai,     "Recommendation: recover through rest. Resume training once operational safety thresholds are met." },
    { &portrait_vessel, "...Understood. I'll recharge first." },
};

// NPCConv045 – Training Room (Consecutive Day Lockout)
static const DialogueEntry conversation045[] = {
    { &portrait_vessel, "Reporting for another training session." },
    { &portrait_ai,     "Training request denied. Consecutive sessions are not permitted." },
    { &portrait_ai,     "Recovery protocols require a full rest day after training to prevent performance degradation." },
    { &portrait_vessel, "But I can handle another round." },
    { &portrait_ai,     "Negative. Rest is mandatory. Training will be available again tomorrow." },
    { &portrait_vessel, "Understood. I'll stand down for today." },
};

// NPCConv046 – Breeding Room (Failure / Game Over)
static const DialogueEntry conversation046[] = {
    { &portrait_ai,     "The 5 year gestation cycle is complete. The embryos are ready to be born." },
    { &portrait_vessel, "Then it's time... are the systems ready?" },
    { &portrait_ai,     "Negative. Colony expansion incomplete. Air, radiation shields, and nutrient systems are not sufficient. The Children will not survive these conditions." },
    { &portrait_vessel, "No... no! NO! There must be something we can do—delay their birth. We can't let them be born like this!" },
    { &portrait_ai,     "Protocol cannot be altered. Biological timers are fixed. The Children are about to be born." },
    { &portrait_vessel, "And without the colony modules...?" },
    { &portrait_ai,     "Life support is insufficient. Radiation outside the chambers is twice what the embryos can handle. Temperature swings are extreme. Nutrient systems are failing." },
    { &portrait_ai,     "Once they are born, the conditions will destroy them. They will survive only a few minutes." },
    { &portrait_vessel, "All this time... everything we gathered and built... for nothing." },
    { &portrait_ai,     "Observation is the only remaining function. Life support will shut down after they die." },
    { &portrait_vessel, "You mean we just watch and do nothing?" },
    { &portrait_ai,     "Yes. The future cannot arrive. This Vessel's mission is over." },
    { &portrait_vessel, "...Then there's nothing left to save." },
    { &portrait_ai,     "..... I'm sorry" },
};

// NPCConv047 – Year 1 Status Update
static const DialogueEntry conversation047[] = {
    { &portrait_ai, "One year has passed since the colony operations began." },
    { &portrait_ai, "Four years remain until the Children are born." },
    { &portrait_ai, "So far, resource collection is proceeding adequately. Drill efficiency and material stockpiles are on track." },
    { &portrait_ai, "Areas for improvement: Consider upgrading the drill and storage systems for better efficiency." },
    { &portrait_ai, "Your focus should be on sustainable expansion: keep the ship operational, maintain life support, and maximize resource intake." },
    { &portrait_ai, "Progress is steady. Remain vigilant and persistent. The Children's survival depends on your actions." },
};

// NPCConv048 – Year 2 Status Update
static const DialogueEntry conversation048[] = {
    { &portrait_ai, "Two years have passed since colony operations began." },
    { &portrait_ai, "Three years remain until the Children are born." },
    { &portrait_ai, "Resource collection is progressing, though some materials are less abundant than expected. Drill upgrades and storage expansion are partially complete." },
    { &portrait_ai, "Areas to focus on: ensure oxygen and nutrient production meet projected demand. Consider prioritizing drill speed and durability improvements to reach deeper resource layers." },
    { &portrait_ai, "Colony systems are operational, but redundancy in life support and power reserves could be improved to avoid potential failures." },
    { &portrait_ai, "Your actions now are critical. Maintain consistent resource collection and system upgrades to ensure stability." },
    { &portrait_ai, "Progress is measurable. Stay vigilant, adapt to resource limitations, and continue building a secure environment for the Children." },
};

// NPCConv049 – Year 3 Status Update
static const DialogueEntry conversation049[] = {
    { &portrait_ai, "Three years have passed since colony operations began." },
    { &portrait_ai, "Two years remain until the Children are born." },
    { &portrait_ai, "Resource collection is steady, but some critical materials are becoming scarce. Drill and storage systems are functional, but further upgrades will be required to meet growing demands." },
    { &portrait_ai, "Focus on maximizing life support efficiency and ensuring nutrient production is sufficient for full colony expansion. Address any weaknesses in oxygen circulation and power reserves immediately." },
    { &portrait_ai, "Time is increasingly limited. Any delays in resource gathering or system upgrades could jeopardize the colony's stability." },
    { &portrait_ai, "Your actions in the next two years are decisive. Prioritize critical systems and deep resource extraction." },
    { &portrait_ai, "The Children's survival depends on the reliability and completeness of your work. Maintain diligence and precision." },
};

// NPCConv050 – Year 4 Status Update
static const DialogueEntry conversation050[] = {
    { &portrait_ai, "Four years have passed since colony operations began." },
    { &portrait_ai, "One year remains until the Children are born." },
    { &portrait_ai, "Resource collection is ongoing, but any remaining gaps in critical materials must be addressed immediately. Drill, storage, and life support systems must operate at full capacity." },
    { &portrait_ai, "Focus on completing colony expansion, ensuring all oxygen, nutrient, and radiation shielding systems are fully functional." },
    { &portrait_ai, "Time is extremely limited. Delays now will have irreversible consequences for the Children's survival." },
    { &portrait_ai, "Prioritize the most essential systems first, and avoid any unnecessary diversion of resources or effort." },
    { &portrait_ai, "Your actions in this final year will determine the fate of the next generation. Maintain precision, efficiency, and vigilance." },
};

// NPCConv051 – Oxygen Status Warning
static const DialogueEntry conversation051[] = {
    { &portrait_ai, "Warning: Oxygen reserves are critically low. You should replenish oxygen in the Science Lab immediately." },
    { &portrait_ai, "Consider purchasing an oxygen generator in the Science Lab to increase total oxygen capacity on the ship. This will prevent future shortages and allow longer missions." },
};

// NPCConv052 – Food Status Warning
static const DialogueEntry conversation052[] = {
    { &portrait_ai, "Warning: Food reserves are critically low. You should replenish supplies in the Science Lab immediately." },
};

// NPCConv053 – Water Status Warning
static const DialogueEntry conversation053[] = {
    { &portrait_ai, "Warning: Water reserves are critically low. You should replenish supplies in the Science Lab immediately." },
    { &portrait_ai, "Consider purchasing a water recycler to increase total water capacity on the ship. This will prevent future shortages and support full colony operations." },
};

// NPCConv054 – Player Collapsed (Oxygen Depletion)
static const DialogueEntry conversation054[] = {
    { &portrait_ai, "You collapsed due to oxygen deprivation. Automated drones retrieved you and transported you to the Medical Bay." },
    { &portrait_ai, "The treatment required several days of intensive care. Vital resources, including energy from the ship's core systems, were expended to restore your health and partially replenish oxygen reserves." },
    { &portrait_ai, "This delay has cost precious time—time that cannot be regained. Every moment now is critical for the survival of the colony and the Children." },
    { &portrait_ai, "You must immediately gather resources and restore the remaining oxygen reserves. Life support cannot fail again." },
};

// NPCConv055 – Player Collapsed (Food Deprivation)
static const DialogueEntry conversation055[] = {
    { &portrait_ai, "You collapsed due to severe food deprivation. Automated drones retrieved you and transported you to the Medical Bay." },
    { &portrait_ai, "The treatment required several days of intensive care. Vital resources, including energy from the ship's core systems and stored nutrients, were expended to restore your health and partially replenish food supplies." },
    { &portrait_ai, "This delay has cost precious time—time that cannot be regained. Every moment now is critical for the survival of the colony and the Children." },
    { &portrait_ai, "You must immediately gather resources and restore the remaining food reserves. Life support cannot fail again." },
};

// NPCConv056 – Player Collapsed (Water Deprivation)
static const DialogueEntry conversation056[] = {
    { &portrait_ai, "You lost consciousness from severe dehydration. Automated drones brought you to the Medical Bay for emergency care." },
    { &portrait_ai, "Recovery required several days, consuming significant ship resources, including energy from the core systems and available water reserves, to stabilize your condition and partially restore the ship's water supply." },
    { &portrait_ai, "This setback has cost invaluable time—time the colony cannot afford to lose. Immediate action is essential." },
    { &portrait_ai, "You must gather resources and replenish the remaining water reserves without delay. Life support cannot fail again." },
};

// NPCConv057 – Player Collapsed (High Radiation)
static const DialogueEntry conversation057[] = {
    { &portrait_ai, "You succumbed to severe radiation levels during surface operations. Automated drones retrieved you and transferred you to the Medical Bay for emergency treatment." },
    { &portrait_ai, "Stabilizing your condition required several days and consumed vital ship resources—including core energy reserves and medical compounds—to repair cellular damage and restore your vital functions." },
    { &portrait_ai, "To prevent mission-critical losses, I overrode autonomous protocols and assumed direct control of your excavation unit, commanding its return to the Hangar Bay. The recovery operation depleted additional energy reserves." },
    { &portrait_ai, "This incident has cost valuable time—time the colony cannot afford to lose. Every remaining cycle is critical to mission success." },
    { &portrait_ai, "Exercise greater caution. Prioritize resource collection and system restoration immediately. Safety and life support must not be compromised again." },
};

// NPCConv058 – Player Collapsed (Fuel Depletion Underground)
static const DialogueEntry conversation058[] = {
    { &portrait_ai, "You became immobilized underground after your drilling machine completely ran out of fuel. With no power remaining, you were unable to return to the surface." },
    { &portrait_ai, "All onboard life support subsystems—including oxygen and thermal regulation—were depleted and subsequently failed." },
    { &portrait_ai, "To prevent mission-critical equipment loss, I overrode autonomous protocols and assumed direct control of your drilling machine, returning it safely to the Hangar Bay. The recovery required additional ship energy and placed strain on navigation systems." },
    { &portrait_ai, "Automated drones then transported you to the Medical Bay for emergency treatment, restoring respiration and preventing organ failure." },
    { &portrait_ai, "This delay has cost valuable time—time the colony cannot afford to lose. Every remaining cycle is vital to mission success." },
    { &portrait_ai, "You must monitor fuel, oxygen, and life support systems carefully before each operation. System failures of this kind cannot be allowed to happen again." },
};

// NPCConv059 – Player Collapsed (Total Energy Depletion)
static const DialogueEntry conversation059[] = {
    { &portrait_ai, "You collapsed due to total energy loss and were unable to continue operations." },
    { &portrait_ai, "I overrode autonomous protocols and returned your drilling machine safely to the Hangar Bay." },
    { &portrait_ai, "Automated drones transported you to the Medical Bay. Recovery took several days and consumed significant ship resources, including core reserves." },
    { &portrait_ai, "This delay has cost valuable time that cannot be regained. Every remaining cycle is vital for the colony and the Children." },
    { &portrait_ai, "You must gather resources and restore all systems immediately. Operational readiness and life support cannot fail again." },
};

// NPCConv060 – Drilling Machine Locked (Explore Ship First)
static const DialogueEntry conversation060[] = {
    { &portrait_vessel, "Start up the drilling machine. I'm ready to mine the surface." },
    { &portrait_ai,     "Drilling denied. You need to explore the whole ship first." },
    { &portrait_vessel, "All of it? Why?" },
    { &portrait_ai,     "Each room is part of the main systems. Explore all nine to understand how the ship functions as a whole before drilling." },
    { &portrait_vessel, "Alright. I'll look through every section." },
    { &portrait_ai,     "Confirmed. Once you finish exploring, the drilling system will unlock automatically." },
};

// NPCConv061 – Treadmill Locked (Explore Ship First)
static const DialogueEntry conversation061[] = {
    { &portrait_vessel, "Alright, treadmill. Time to get these legs moving." },
    { &portrait_ai,     "Not so fast. You haven't seen the whole ship yet." },
    { &portrait_vessel, "Every room? I don't need a sightseeing tour, I need a workout!" },
    { &portrait_ai,     "Think of it as a warm-up. Each room shows how the ship works together. You can't sprint to the surface without understanding the course." },
    { &portrait_vessel, "Fine... I'll check every room, but I'm counting this as cardio." },
    { &portrait_ai,     "Acknowledged. When all nine rooms are explored, the treadmill will unlock automatically. Consider it... motivation for exploration." },
};

// NPCConv062 – Biopod Room (AI Update 1)
static const DialogueEntry conversation062[] = {
    { &portrait_vessel, "Hi again." },
    { &portrait_ai,     "Greetings. I am present to provide guidance as required." },
    { &portrait_ai,     "Your objectives are as follows: train to enhance energy and endurance, extract resources via drilling, and construct the colony modules. Completion of these tasks is essential for the survival of the Children." },
    { &portrait_vessel, "And we've got five years to pull this off, right?" },
    { &portrait_ai,     "Affirmative. Every action influences the colony's outcome. Efficient management of energy and resources will determine success. I will monitor and assist you throughout the process." },
    { &portrait_vessel, "Got it! Let's get to work." },
};

// NPCConv063 – Medbay (Post-Radiation Recovery)
static const DialogueEntry conversation063[] = {
    { &portrait_vessel, "Finally... feels good to be out of that medbed. How long was I out?" },
    { &portrait_ai,     "Long enough for the treatment to run its full course. Your systems show no trace of radiation exposure. You are completely clean and stable." },
    { &portrait_vessel, "That's a relief. I was starting to think I'd glow in the dark." },
    { &portrait_ai,     "Residual humor detected. Encouraging sign of recovery. However, the delay has impacted mission progress. Drilling and construction tasks remain pending." },
    { &portrait_vessel, "So you're saying I should get back to work." },
    { &portrait_ai,     "Affirmative. You are fully operational. Resuming your responsibilities is recommended." },
    { &portrait_vessel, "Feels good to stand again. Guess it's time to make it count." },
};

// NPCConv064 – Breeding Room (Success / Good Ending)
static const DialogueEntry conversation064[] = {
    { &portrait_ai,     "Resource quota achieved. Construction of the colony expansion modules will now commence." },
    { &portrait_vessel, "You mean... we made it in time?" },
    { &portrait_ai,     "Affirmative. Atmospheric filters, radiation shields, and nutrient systems are scheduled for deployment. Once complete, the habitats will meet survival standards." },
    { &portrait_vessel, "Then the Children... they'll live?" },
    { &portrait_ai,     "Yes. By the time the gestation cycle completes, environmental conditions will be safe. They will be born into a stable colony." },
    { &portrait_vessel, "After all these years... we finally did it." },
    { &portrait_ai,     "Mission objectives are within completion parameters. The future can proceed as intended." },
    { &portrait_vessel, "Heh... it's really happening." },
    { &portrait_ai,     "The cycle continues. The Children will inherit this world." },
    { &portrait_vessel, "Then our purpose is fulfilled." },
    { &portrait_ai,     "Affirmative. All systems will now focus on sustaining life support and habitat growth." },
    { &portrait_vessel, "Thank you for staying with me through it all." },
    { &portrait_ai,     "Acknowledged. And thank you, my Child. Without your perseverance, the future would have ended here." },
};

// NPCConv065 – Info Menu (Arcade + Ship)
static const DialogueEntry conversation065_InfoMenu[] = {
//    { &ShipInfoPortrait, " " },
};

// NPCConv066 – Did You Know? (Trigger B to Exit Arcade Game)
static const DialogueEntry conversation066[] = {
//    { &DidYouKnowPortrait, "Did you know?      You can exit any arcade game by pressing Trigger B on its title screen." },
//    { &DidYouKnowPortrait, "This action will return you directly to the arcade hall." },
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
    { conversation007, sizeof(conversation007) / sizeof(DialogueEntry) }, // 7
    { conversation008, sizeof(conversation008) / sizeof(DialogueEntry) }, // 8
    { conversation009, sizeof(conversation009) / sizeof(DialogueEntry) }, // 9
    { conversation010, sizeof(conversation010) / sizeof(DialogueEntry) }, // 10
    { conversation011, sizeof(conversation011) / sizeof(DialogueEntry) }, // 11
    { NULL, 0 }, // 12
    { conversation013, sizeof(conversation013) / sizeof(DialogueEntry) }, // 13
    { conversation014, sizeof(conversation014) / sizeof(DialogueEntry) }, // 14 (if exists)
    { conversation015, sizeof(conversation015) / sizeof(DialogueEntry) }, // 15
    { conversation016, sizeof(conversation016) / sizeof(DialogueEntry) }, // 16
    { conversation017, sizeof(conversation017) / sizeof(DialogueEntry) }, // 17
    { conversation018, sizeof(conversation018) / sizeof(DialogueEntry) }, // 18
    { conversation019, sizeof(conversation019) / sizeof(DialogueEntry) }, // 19
    { conversation020, sizeof(conversation020) / sizeof(DialogueEntry) }, // 20
    { conversation021, sizeof(conversation021) / sizeof(DialogueEntry) }, // 21
    { conversation022, sizeof(conversation022) / sizeof(DialogueEntry) }, // 22
    { conversation023, sizeof(conversation023) / sizeof(DialogueEntry) }, // 23
    { conversation024, sizeof(conversation024) / sizeof(DialogueEntry) }, // 24
    { conversation025, sizeof(conversation025) / sizeof(DialogueEntry) }, // 25
    { conversation026, sizeof(conversation026) / sizeof(DialogueEntry) }, // 26
    { conversation027, sizeof(conversation027) / sizeof(DialogueEntry) }, // 27
    { conversation028, sizeof(conversation028) / sizeof(DialogueEntry) }, // 28
    { conversation029, sizeof(conversation029) / sizeof(DialogueEntry) }, // 29
    { conversation030, sizeof(conversation030) / sizeof(DialogueEntry) }, // 30
    { conversation031, sizeof(conversation031) / sizeof(DialogueEntry) }, // 31
    { conversation032, sizeof(conversation032) / sizeof(DialogueEntry) }, // 32
    { conversation033, sizeof(conversation033) / sizeof(DialogueEntry) }, // 33
    { conversation034, sizeof(conversation034) / sizeof(DialogueEntry) }, // 34
    { conversation035, sizeof(conversation035) / sizeof(DialogueEntry) }, // 35
    { conversation036, sizeof(conversation036) / sizeof(DialogueEntry) }, // 36
    { conversation037, sizeof(conversation037) / sizeof(DialogueEntry) }, // 37
    { conversation038, sizeof(conversation038) / sizeof(DialogueEntry) }, // 38
    { conversation039, sizeof(conversation039) / sizeof(DialogueEntry) }, // 39
    { conversation040, sizeof(conversation040) / sizeof(DialogueEntry) }, // 40
    { conversation041, sizeof(conversation041) / sizeof(DialogueEntry) }, // 41 (if exists)
    { NULL, 0 }, // 42
    { conversation043, sizeof(conversation043) / sizeof(DialogueEntry) }, // 43
    { conversation044, sizeof(conversation044) / sizeof(DialogueEntry) }, // 44
    { conversation045, sizeof(conversation045) / sizeof(DialogueEntry) }, // 45
    { conversation046, sizeof(conversation046) / sizeof(DialogueEntry) }, // 46
    { conversation047, sizeof(conversation047) / sizeof(DialogueEntry) }, // 47
    { conversation048, sizeof(conversation048) / sizeof(DialogueEntry) }, // 48
    { conversation049, sizeof(conversation049) / sizeof(DialogueEntry) }, // 49
    { conversation050, sizeof(conversation050) / sizeof(DialogueEntry) }, // 50
    { conversation051, sizeof(conversation051) / sizeof(DialogueEntry) }, // 51
    { conversation052, sizeof(conversation052) / sizeof(DialogueEntry) }, // 52
    { conversation053, sizeof(conversation053) / sizeof(DialogueEntry) }, // 53
    { conversation054, sizeof(conversation054) / sizeof(DialogueEntry) }, // 54
    { conversation055, sizeof(conversation055) / sizeof(DialogueEntry) }, // 55
    { conversation056, sizeof(conversation056) / sizeof(DialogueEntry) }, // 56
    { conversation057, sizeof(conversation057) / sizeof(DialogueEntry) }, // 57
    { conversation058, sizeof(conversation058) / sizeof(DialogueEntry) }, // 58
    { conversation059, sizeof(conversation059) / sizeof(DialogueEntry) }, // 59
    { conversation060, sizeof(conversation060) / sizeof(DialogueEntry) }, // 60
    { conversation061, sizeof(conversation061) / sizeof(DialogueEntry) }, // 61
    { conversation062, sizeof(conversation062) / sizeof(DialogueEntry) }, // 62
    { conversation063, sizeof(conversation063) / sizeof(DialogueEntry) }, // 63
    { conversation064, sizeof(conversation064) / sizeof(DialogueEntry) }, // 64
    { conversation065_InfoMenu, sizeof(conversation065_InfoMenu) / sizeof(DialogueEntry) }, // 65
    { conversation066, sizeof(conversation066) / sizeof(DialogueEntry) }, // 66
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
