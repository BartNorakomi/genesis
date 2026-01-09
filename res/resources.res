IMAGE arcade1 "graphics/backgrounds/arcade1.bmp" BEST ALL
IMAGE arcade2 "graphics/backgrounds/arcade2.bmp" BEST ALL
IMAGE armoryvault "graphics/backgrounds/armoryvault.bmp" BEST ALL
IMAGE biopod "graphics/backgrounds/biopod.bmp" BEST ALL
IMAGE hangarbay "graphics/backgrounds/hangarbay.bmp" BEST ALL
IMAGE holodeck "graphics/backgrounds/holodeck.bmp" BEST ALL
IMAGE hydroponicsbay "graphics/backgrounds/hydroponicsbay.bmp" BEST ALL
IMAGE medicalbay "graphics/backgrounds/medicalbay.bmp" BEST ALL
IMAGE reactorchamber "graphics/backgrounds/reactorchamber.bmp" BEST ALL
IMAGE sciencelab "graphics/backgrounds/sciencelab.bmp" BEST ALL
IMAGE sleepingquarters "graphics/backgrounds/sleepingquarters.bmp" BEST ALL
IMAGE trainingdeck "graphics/backgrounds/trainingdeck.bmp" BEST ALL

IMAGE titlescreen "graphics/titlescreen/titlescreen.bmp" BEST ALL

BIN level_col_arcade1 "collisionmaps/tilemapArcade1.bin"
BIN level_col_arcade2 "collisionmaps/tilemapArcade2.bin"
BIN level_col_armoryvault "collisionmaps/tilemaparmoryvault.bin"
BIN level_col_biopod "collisionmaps/tilemapBiopod.bin"
BIN level_col_hangarbay "collisionmaps/tilemaphangarbay.bin"
BIN level_col_holodeck "collisionmaps/tilemapholodeck.bin"
BIN level_col_hydroponicsbay "collisionmaps/tilemaphydroponicsbay.bin"
BIN level_col_medicalbay "collisionmaps/tilemapmedicalbay.bin"
BIN level_col_reactorchamber "collisionmaps/tilemapreactorchamber.bin"
BIN level_col_sciencelab "collisionmaps/tilemapsciencelab.bin"
BIN level_col_sleepingquarters "collisionmaps/tilemapsleepingquarters.bin"
BIN level_col_trainingdeck "collisionmaps/tilemaptrainingdeck.bin"

SPRITE playerSpriteDef "sprites/player/playersprite.bmp" 10 10 FAST 3
SPRITE girlSpriteDef "sprites/npcs/girl.bmp" 10 10 FAST 30
SPRITE capgirlSpriteDef "sprites/npcs/capgirl.bmp" 10 10 FAST 30
SPRITE redheadboySpriteDef "sprites/npcs/redheadboy.bmp" 10 10 FAST 30
SPRITE titlescreenpressbuttonSpriteDef "sprites/titlescreen/titlescreenpressbuttonsprite.bmp" 10 2 FAST 26

#option 1: export audio in audacity: 15000 Hz, RAW, Signed 8-bit PCM and play back HALF RATE=FALSE: BIN sfx_startgame "sfx/StartGame_ActionMusical_AP1.104_15khz.raw"
#option 2: export audio in audacity: 7500 Hz, RAW, Signed 8-bit PCM and play back HALF RATE=TRUE: BIN sfx_startgame "sfx/StartGame_ActionMusical_AP1.104_7.5khz.raw"
#the lower the khz the higher the pitch
 
WAV startgame "sfx/startgame.wav" XGM2
WAV playerfootstep "sfx/player_footstep.wav" XGM2
WAV crowdapplause "sfx/Large Crowd Applause 04.wav" XGM2
