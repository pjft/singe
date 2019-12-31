     cd src/vldp2
     ./configure --disable-accel-detect
     make -f Makefile.linux_x64
     cd ../game/singe
     make -f Makefile.linux_x64
     cd ../..
     make
