IMAGE arcade1 "graphics/backgrounds/arcade1.bmp" BEST ALL
IMAGE arcade2 "graphics/backgrounds/arcade2.bmp" BEST ALL
IMAGE armoryvault "graphics/backgrounds/armoryvault.bmp" BEST ALL
IMAGE biopod "graphics/backgrounds/biopod.bmp" BEST ALL
IMAGE hangarbay "graphics/backgrounds/hangarbay.bmp" BEST ALL
IMAGE holodeck "graphics/backgrounds/holodeck.bmp" BEST ALL
IMAGE hydroponicsbay "graphics/backgrounds/hydroponicsbay.bmp" BEST ALL
IMAGE hydroponicsbay_fg "graphics/backgrounds/hydroponicsbayforeground.bmp" BEST ALL
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
SPRITE reactorSpriteDef "sprites/roomobjects/reactor.bmp" 16 18 FAST 10
SPRITE armoryVaultPanelSpriteDef "sprites/roomobjects/armoryvaultpanel.bmp" 2 2 FAST 10
SPRITE medicalBayChairSpriteDef "sprites/roomobjects/medicalbaychair.bmp" 16 11 FAST NONE
SPRITE medicalBayLightsSpriteDef "sprites/roomobjects/medicalbaylights.bmp" 18 2 FAST 20
SPRITE sleepingQuartersLightsSpriteDef "sprites/roomobjects/sleepingquarterslights.bmp" 2 4 FAST NONE
SPRITE trainingDeckTreadmillSpriteDef "sprites/roomobjects/trainingdecktreadmill.bmp" 12 10 FAST NONE
SPRITE trainingDeckWallLeftSpriteDef "sprites/roomobjects/trainingdeckwallleft.bmp" 2 14 FAST NONE
SPRITE trainingDeckWallRightSpriteDef "sprites/roomobjects/trainingdeckwallright.bmp" 2 14 FAST NONE
SPRITE hangarBayDrillingMachineSpriteDef "sprites/roomobjects/hangarbaydrillingmachine.bmp" 12 12 FAST NONE
SPRITE scienceLabHelixSpriteDef "sprites/roomobjects/sciencelabhelix.bmp" 4 12 FAST 20
SPRITE biopodRightPodSpriteDef "sprites/roomobjects/biopodrightpod.bmp" 4 10 FAST NONE
SPRITE biopodLeftPodSpriteDef "sprites/roomobjects/biopodleftpod.bmp" 4 6 FAST NONE
SPRITE biopodLightSpriteDef "sprites/roomobjects/biopodlight.bmp" 2 2 FAST 10
SPRITE hydroponicsBayFoodLeftSpriteDef "sprites/roomobjects/hydroponicsbayfoodleft.bmp" 6 14 FAST NONE
SPRITE hydroponicsBayFoodRightSpriteDef "sprites/roomobjects/hydroponicsbayfoodright.bmp" 12 10 FAST NONE
SPRITE holodeckDoorSpriteDef "sprites/roomobjects/holodeckdoor.bmp" 6 20 FAST NONE
SPRITE holodeckFloorSpriteDef "sprites/roomobjects/holodeckfloor.bmp" 30 10 FAST 6
SPRITE arcade1WallSpriteDef "sprites/roomobjects/arcade1Wall.bmp" 8 10 FAST NONE
SPRITE arcade1OpenDoorSpriteDef "sprites/roomobjects/arcade1opendoor.bmp" 6 14 FAST NONE
SPRITE titlescreenpressbuttonSpriteDef "sprites/titlescreen/titlescreenpressbuttonsprite.bmp" 10 2 FAST 26
   
#option 1: export audio in audacity: 15000 Hz, RAW, Signed 8-bit PCM and play back HALF RATE=FALSE: BIN sfx_startgame "sfx/StartGame_ActionMusical_AP1.104_15khz.raw"
#option 2: export audio in audacity: 7500 Hz, RAW, Signed 8-bit PCM and play back HALF RATE=TRUE: BIN sfx_startgame "sfx/StartGame_ActionMusical_AP1.104_7.5khz.raw"
#the lower the khz the higher the pitch
   
WAV startgame "sfx/startgame.wav" XGM2
WAV playerfootstep "sfx/player_footstep.wav" XGM2
WAV crowdapplause "sfx/Large Crowd Applause 04.wav" XGM2
