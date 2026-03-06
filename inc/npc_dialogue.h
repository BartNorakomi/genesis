#ifndef NPC_DIALOGUE_H
#define NPC_DIALOGUE_H

void npcDialogueOpenWindow(const Image* portrait);
void npcDialogueCloseWindow(void);
void runDialogue(const Image* portrait, u8 whichText);

#endif
