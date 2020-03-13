## ActionMax file data

Use below folder structure with provided `actionmax.sh` script:

     .daphne/singe/actionmax/
     .daphne/singe/actionmax/38ambushalley
     .daphne/singe/actionmax/38ambushalley/38ambushalley.singe
     .daphne/singe/actionmax/38ambushalley/frame_38ambushalley.txt
     .daphne/singe/actionmax/38ambushalley/video_38ambushalley.m2v
     .daphne/singe/actionmax/38ambushalley/video_38ambushalley.ogg
     .daphne/singe/actionmax/bluethunder
     .daphne/singe/actionmax/bluethunder/bluethunder.singe
     .daphne/singe/actionmax/bluethunder/frame_bluethunder.txt
     .daphne/singe/actionmax/bluethunder/video_bluethunder.m2v
     .daphne/singe/actionmax/bluethunder/video_bluethunder.ogg
     .daphne/singe/actionmax/emulator.singe
     .daphne/singe/actionmax/font_bluestone.ttf
     .daphne/singe/actionmax/font_chemrea.ttf
     .daphne/singe/actionmax/font_led_real.ttf
     .daphne/singe/actionmax/framework.singe
     .daphne/singe/actionmax/hydrosub2021
     .daphne/singe/actionmax/hydrosub2021/frame_hydrosub2021.txt
     .daphne/singe/actionmax/hydrosub2021/hydrosub2021.singe
     .daphne/singe/actionmax/hydrosub2021/video_hydrosub2021.m2v
     .daphne/singe/actionmax/hydrosub2021/video_hydrosub2021.ogg
     .daphne/singe/actionmax/popsghostly
     .daphne/singe/actionmax/popsghostly/frame_popsghostly.txt
     .daphne/singe/actionmax/popsghostly/popsghostly.singe
     .daphne/singe/actionmax/popsghostly/video_popsghostly.m2v
     .daphne/singe/actionmax/popsghostly/video_popsghostly.ogg
     .daphne/singe/actionmax/sonicfury
     .daphne/singe/actionmax/sonicfury/frame_sonicfury.txt
     .daphne/singe/actionmax/sonicfury/sonicfury.singe
     .daphne/singe/actionmax/sonicfury/video_sonicfury.m2v
     .daphne/singe/actionmax/sonicfury/video_sonicfury.ogg
     .daphne/singe/actionmax/sound_actionmax.wav
     .daphne/singe/actionmax/sound_asteadyaimiscritical.wav
     .daphne/singe/actionmax/sound_badhit.wav
     .daphne/singe/actionmax/sound_gameover.wav
     .daphne/singe/actionmax/sound_getreadyforaction.wav
     .daphne/singe/actionmax/sound_goodhit.wav
     .daphne/singe/actionmax/sound_gunshot.wav
     .daphne/singe/actionmax/sprite_38ambushalley.png
     .daphne/singe/actionmax/sprite_actionmax.png
     .daphne/singe/actionmax/sprite_bluethunder.png
     .daphne/singe/actionmax/sprite_bullet.png
     .daphne/singe/actionmax/sprite_crosshair.png
     .daphne/singe/actionmax/sprite_hydrosub2021.png
     .daphne/singe/actionmax/sprite_lightoff.png
     .daphne/singe/actionmax/sprite_lighton.png
     .daphne/singe/actionmax/sprite_popsghostly.png
     .daphne/singe/actionmax/sprite_sonicfury.png
     .daphne/singe/actionmax/video_actionmaxintro.m2v
     .daphne/singe/actionmax/video_actionmaxintro.ogg
     .daphne/singe/actionmax/video_menu.m2v
     .daphne/singe/actionmax/video_menu.ogg


### All filenames have been converted to lowercase in this structure

You will need to use the following `framefiles` and `diff` to accomodate this.

Use the following framefile contents:


**38ambushalley/frame_38ambushalley.txt**

    .
    
    1       ../video_actionmaxintro.m2v
    355     video_38ambushalley.m2v
    28175   ../video_menu.m2v

**bluethunder/frame_bluethunder.txt**

    .
    
    1       ../video_actionmaxintro.m2v
    355     video_bluethunder.m2v
    27950   ../video_menu.m2v

**hydrosub2021/frame_hydrosub2021.txt**

    .
    
    1       ../video_actionmaxintro.m2v
    355     video_hydrosub2021.m2v
    28820   ../video_menu.m2v

**popsghostly/frame_popsghostly.txt**

    .
    
    1       ../video_actionmaxintro.m2v
    355     video_popsghostly.m2v
    31835   ../video_menu.m2v

**sonicfury/frame_sonicfury.txt**

    .
    
    1       ../video_actionmaxintro.m2v
    355     video_sonicfury.m2v
    26645   ../video_menu.m2v


### Patches to singe files for new structure

**Game specific singe files:**

38ambushalley/38ambushalley.singe, popsghostly/popsghostly.singe, \
bluethunder/bluethunder.singe, sonicfury/sonicfury.singe and \
hydrosub2021/hydrosub2021.singe

	-dofile("singe/actionmax/Emulator.singe")
	+dofile("singe/actionmax/emulator.singe")


**Finally apply the following patch to `emulator.singe` for the new structure:**


Apply in the same directory as `emulator.singe` via:

    patch -p0 < emulator.diff


### Diff file (emulator.diff):

    --- a/emulator.singe    2020-03-13 15:31:18.000000000 +0000
    +++ emulator.singe      2019-12-17 07:39:52.428714520 +0000
    @@ -1,4 +1,4 @@
    -dofile("singe/Singe/Framework.singe")
    +dofile("singe/actionmax/framework.singe")
    
     stateStartup    = 0
     stateSetup      = 1
    @@ -15,20 +15,20 @@
     pixelHigh    = 1
     pixelUnknown = 2
    
    -sprLightOn   = spriteLoad("singe/ActionMax/sprite_LightOn.png")
    -sprLightOff  = spriteLoad("singe/ActionMax/sprite_LightOff.png")
    -sprActionMax = spriteLoad("singe/ActionMax/sprite_ActionMax.png")
    -sprCrosshair = spriteLoad("singe/ActionMax/sprite_Crosshair.png")
    -sprBullet    = spriteLoad("singe/ActionMax/sprite_Bullet.png")
    -sprBoxArt    = spriteLoad("singe/ActionMax/sprite_" .. gameID .. ".png")
    -
    -sndActionMax = soundLoad("singe/ActionMax/sound_ActionMax.wav")
    -sndSteadyAim = soundLoad("singe/ActionMax/sound_ASteadyAimIsCritical.wav")
    -sndGetReady  = soundLoad("singe/ActionMax/sound_GetReadyForAction.wav")
    -sndGunShot   = soundLoad("singe/ActionMax/sound_Gunshot.wav")
    -sndGoodHit   = soundLoad("singe/ActionMax/sound_GoodHit.wav")
    -sndBadHit    = soundLoad("singe/ActionMax/sound_BadHit.wav")
    -sndGameOver  = soundLoad("singe/ActionMax/sound_GameOver.wav")
    +sprLightOn   = spriteLoad("singe/actionmax/sprite_lighton.png")
    +sprLightOff  = spriteLoad("singe/actionmax/sprite_lightoff.png")
    +sprActionMax = spriteLoad("singe/actionmax/sprite_actionmax.png")
    +sprCrosshair = spriteLoad("singe/actionmax/sprite_crosshair.png")
    +sprBullet    = spriteLoad("singe/actionmax/sprite_bullet.png")
    +sprBoxArt    = spriteLoad("singe/actionmax/sprite_" .. string.lower(gameID) .. ".png")
    +
    +sndActionMax = soundLoad("singe/actionmax/sound_actionmax.wav")
    +sndSteadyAim = soundLoad("singe/actionmax/sound_asteadyaimiscritical.wav")
    +sndGetReady  = soundLoad("singe/actionmax/sound_getreadyforaction.wav")
    +sndGunShot   = soundLoad("singe/actionmax/sound_gunshot.wav")
    +sndGoodHit   = soundLoad("singe/actionmax/sound_goodhit.wav")
    +sndBadHit    = soundLoad("singe/actionmax/sound_badhit.wav")
    +sndGameOver  = soundLoad("singe/actionmax/sound_gameover.wav")
    
     mouseX = 0
     mouseY = 0
    @@ -63,11 +63,11 @@
     lastSeconds = 0
     heartbeat = false
    
    -fntBlueStone20 = fontLoad("singe/ActionMax/font_BlueStone.ttf", 20)
    -fntChemRea16   = fontLoad("singe/ActionMax/font_chemrea.ttf", 16)
    -fntChemRea32   = fontLoad("singe/ActionMax/font_chemrea.ttf", 32)
    -fntChemRea48   = fontLoad("singe/ActionMax/font_chemrea.ttf", 48)
    -fntLEDReal32   = fontLoad("singe/ActionMax/font_LED_Real.ttf", 32)
    +fntBlueStone20 = fontLoad("singe/actionmax/font_bluestone.ttf", 20)
    +fntChemRea16   = fontLoad("singe/actionmax/font_chemrea.ttf", 16)
    +fntChemRea32   = fontLoad("singe/actionmax/font_chemrea.ttf", 32)
    +fntChemRea48   = fontLoad("singe/actionmax/font_chemrea.ttf", 48)
    +fntLEDReal32   = fontLoad("singe/actionmax/font_led_real.ttf", 32)
    
     colorBackground(0, 0, 0)
     fontQuality(FONT_QUALITY_BLENDED)

