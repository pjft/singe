#!/bin/bash

SCRIPT_DIR=`dirname "$0"`
if realpath / >/dev/null; then SCRIPT_DIR=$(realpath "$SCRIPT_DIR"); fi
SINGE_BIN=singe.bin
DAPHNE_SHARE=~/.daphne

echo "ActionMax Launcher : Script dir is $SCRIPT_DIR"
cd "$SCRIPT_DIR"

# point to our linked libs that user may not have
export LD_LIBRARY_PATH=$SCRIPT_DIR:$DAPHNE_SHARE:$LD_LIBRARY_PATH

if [ "$1" = "-fullscreen" ]; then
    FULLSCREEN="-fullscreen"
    shift
fi

if [ -z "$1" ] ; then
    echo "Specify a game to try: "
    echo
    echo  -e "\t$0 [-fullscreen] <gamename>"

    for game in 38ambushalley bluethunder hydrosub2021 popsghostly sonicfury; do
	if ls ~/.daphne/singe/actionmax/$game >/dev/null 2>&1; then
	    installed="$installed $game"
	else
	    uninstalled="$uninstalled $game"
	fi
    done
    if [ "$uninstalled" ]; then
	echo
	echo "Games not found in ~/.daphne/singe/actionmax: "
	echo "$uninstalled" | fold -s -w60 | sed 's/^ //; s/^/\t/'
    fi
    if [ -z "$installed" ]; then
	cat <<EOF 

Error: Please put the required files in ~/.daphne/singe/actionmax
EOF
    else   
	echo
	echo "Games available: "
	echo "$installed" | fold -s -w80 | sed 's/^ //; s/^/\t/'
    fi
    exit 1
fi

if [ ! -f $DAPHNE_SHARE/singe/actionmax/$1/$1.singe ] || [ ! -f $DAPHNE_SHARE/singe/actionmax/$1/frame_$1.txt ]; then
	echo 
	echo "Missing file: $DAPHNE_SHARE/singe/actionmax/$1/$1.singe ?"
	echo "              $DAPHNE_SHARE/singe/actionmax/$1/frame_$1.txt ?"
	echo 
	exit 1
fi

./$SINGE_BIN \
$DAPHNE_SHARE/singe/actionmax/$1/$1.singe \
$FULLSCREEN \
-framefile $DAPHNE_SHARE/singe/actionmax/$1/frame_$1.txt \
-homedir $DAPHNE_SHARE \
-datadir $DAPHNE_SHARE \
-volume_nonvldp 15 \
-volume_vldp 40 \
-x 800 \
-y 600


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
