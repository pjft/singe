Standalone Singe (v1.18) for Linux
==================================

*- As of version 1.15 Singe is officially a fork of Daphne.
It only runs singe games.*

This code was discovered in an archived zipfile:
* No change control files were present. 
* Some changes referenced RDG within comments.
* Underlying Daphne engine heavily stripped back.
* *CHANGELOG* describes feature additions and fixes. 
* diff file for Singe between 1.14 and 1.18 is provided below:

https://github.com/DirtBagXon/singe/blob/master/patches/singe_1.18.diff

**This repository contains source and binaries for** `x86_64` _Debian_ **&** `armv7l` _Raspbian_ **linux.**

### Changelog

https://github.com/DirtBagXon/singe/blob/master/CHANGELOG.md

### Local changes

* Patched to allow 64bit system reading of 32bit Lua bytecode supplied with *Time Gal* and *Ninja Hayate*. \
This is configurable in `lundump.h` via `LUAC_STR_SIZE_TYPE`. \
Corrects: `SINGE: error compiling script: bad header in precompiled chunk`. \
See `bytecode` directory for further information on bytecode issues.
* Changed _default_ keymap filename from `dapinput.ini` to `singeinput.ini`.
* Added extra command line argument: `-keymapfile <altinput.ini>` \
Specifies an _alternate_ `singeinput.ini` file. \
_e.g._ allows specifying a swapped _UP/DOWN_ keymap file for flight based games.
* Limited `-vertical_stretch` argument values to between `0` and `50`. \
This blocks a segfault on `g_vertical_offset` calculations when supplied with invalid values.\
Limit will be silently enforced with a *WARNING* to log.
* `*gamevid_pixels` calculations have been changed for SDL overlay values in `SDL_LockYUVOverlay`. \
This corrects an _overlay misalignment_ and checks for another _out-of-bounds segfault_.
* New console error graphic. Error sound has been re-enabled with new `grumble.wav`.

### Dependencies

     libasound2-dev libcaca-dev libglew-dev libglib2.0-dev libogg-dev libpcre3-dev libpcre32-3 libpng-dev 
     libpng-tools libpulse-dev libsdl-image1.2-dev libsdl-ttf2.0-dev libsdl1.2-dev libslang2-dev 
     libvorbis-dev libxi-dev pkg-config


### Compile and install

     cd src/vldp2
     ./configure --disable-accel-detect
     make -f Makefile.linux
     cd ../game/singe
     make -f Makefile.linux
     cd ../..
     make
     cd ..
     mkdir -p ~/.daphne/singe
     cp -a pics sound ~/.daphne/

### Install precompiled binaries

    cd binaries/$(uname -m)
    cp *.so singe.bin ../..
    cd ../..

### Game data files

**There is no game data in this repository. You will need to aquire game data files elsewhere to utilise this software.**

Game data should be copied into ~/.daphne/singe/ within a sub-directory name matching the game title.

The primary .singe and .txt framefiles should also match this naming convention within the game sub-directory.

     ~/.daphne/singe/timegal/timegal.singe
     ~/.daphne/singe/timegal/timegal.txt

### Actionmax data files

Refer to auxillary file for ActionMax folder structure:

https://github.com/DirtBagXon/singe/blob/master/ACTIONMAX.md

### Frontend scripts based on BASH scripts provided by:

https://github.com/DavidGriffith/daphne

### GUI frontend

A **very simple** GTK frontend GUI: https://github.com/DirtBagXon/ldfrontend

### Run games

     ./run.sh timegal
     ./actionmax.sh popsghostly


![timegal](https://raw.githubusercontent.com/DirtBagXon/singe/master/screenshots/timegal.png) 
![starblazers](https://raw.githubusercontent.com/DirtBagXon/singe/master/screenshots/starblazers.png) 
![popsghostly](https://raw.githubusercontent.com/DirtBagXon/singe/master/screenshots/popsghostly.png) 
![singelauncher](https://raw.githubusercontent.com/DirtBagXon/ldfrontend/master/screenshots/singelauncher.png) 

