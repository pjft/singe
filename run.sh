#!/bin/sh

SCRIPT_DIR=`dirname "$0"`
if realpath / >/dev/null; then SCRIPT_DIR=$(realpath "$SCRIPT_DIR"); fi
SINGE_BIN=singe.bin
DAPHNE_SHARE=~/.daphne

echo "Singe Launcher : Script dir is $SCRIPT_DIR"
cd "$SCRIPT_DIR"

# point to our linked libs that user may not have
export LD_LIBRARY_PATH=$SCRIPT_DIR:$DAPHNE_SHARE:$LD_LIBRARY_PATH

if [ "$1" = "-fullscreen" ]; then
    FULLSCREEN="-fullscreen"
    shift
fi

if [ -z $1 ] ; then
	echo "Specify a game to try: "
	echo
	echo "\t$0 [-fullscreen] <gamename>"
	echo

        echo "Games available: "
	for game in $(ls $DAPHNE_SHARE/singe/); do
		if [ $game != "actionmax" ]; then 
			installed="$installed $game"
		fi
        done
        echo "$installed" | fold -s -w60 | sed 's/^ //; s/^/\t/'
	echo
	exit
fi

if [ ! -f $DAPHNE_SHARE/singe/$1/$1.singe ] || [ ! -f $DAPHNE_SHARE/singe/$1/$1.txt ]; then
        echo 
        echo "Missing file: $DAPHNE_SHARE/singe/$1/$1.singe ?"
        echo "              $DAPHNE_SHARE/singe/$1/$1.txt ?"
        echo 
        exit 1
fi

./$SINGE_BIN \
$DAPHNE_SHARE/singe/$1/$1.singe \
$FULLSCREEN \
-framefile $DAPHNE_SHARE/singe/$1/$1.txt \
-homedir $DAPHNE_SHARE \
-datadir $DAPHNE_SHARE \
-sound_buffer 2048 \
-volume_nonvldp 5 \
-volume_vldp 30 \
-x 800 \
-y 600 

#-keymapfile altsinge.ini \

EXIT_CODE=$?

if [ "$EXIT_CODE" -ne "0" ] ; then
	if [ "$EXIT_CODE" -eq "127" ]; then
		echo ""
		echo "SINGE failed to start."
		echo "This is probably due to a library problem."
		echo "Run ./singe.bin directly to see which libraries are missing."
		echo ""
	else
		echo "Loader failed with an unknown exit code : $EXIT_CODE."
	fi
fi
