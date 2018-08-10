#!/bin/bash
TIME_WARP=1

CMD_ARGS=""
NO_M200=""
NO_MOKAI=""
NO_SHORESIDE=""


#-------------------------------------------------------
#  Part 1: Check for and handle command-line arguments
#-------------------------------------------------------
for ARGI; do
    if [ "${ARGI}" = "--help" -o "${ARGI}" = "-h" ] ; then
        HELP="yes"
    elif [ "${ARGI}" = "--no_shoreside" -o "${ARGI}" = "-ns" ] ; then
        NO_SHORESIDE="true"
    elif [ "${ARGI}" = "--no_mokai" -o "${ARGI}" = "-nmo" ] ; then
        NO_MOKAI="true"
    elif [ "${ARGI}" = "--no_m200" -o "${ARGI}" = "-nm2" ] ; then
        NO_M200="true"
    elif [ "${ARGI//[^0-9]/}" = "$ARGI" -a "$TIME_WARP" = 1 ]; then
        TIME_WARP=$ARGI
        echo "Time warp set up to $TIME_WARP."
    elif [ "${ARGI}" = "--just_build" -o "${ARGI}" = "-j" ] ; then
        JUST_BUILD="-j" 
        # Pass $JUST_BUILD directly to the launch commands, this will be blank if not just building
        echo "Just building files; no vehicle launch."
    else
        CMD_ARGS=$CMD_ARGS" "$ARGI
    fi
done


if [ "${HELP}" = "yes" ]; then
  echo "$0 [SWITCHES]"
  echo "  XX                  : Time warp"
  echo "  --no_shoreside, -ns"
  echo "  --no_mokai, -nm"
  echo "  --no_m200, -nm2"
  echo "  --just_build, -j"
  echo "  --help, -h"
  exit 0;
fi

#-------------------------------------------------------
#  Part 2: Launching M200s
#-------------------------------------------------------
if [[ -z $NO_M200 ]]; then
  cd ./m200
  # Evan Blue
  ./launch_m200_defend.sh $TIME_WARP $JUST_BUILD -e -b -s > /dev/null &
  # Felix Red
  ./launch_m200_test.sh $TIME_WARP $JUST_BUILD -f -r -s > /dev/null &
  cd ..
fi

#-------------------------------------------------------
#  Part 3: Launching MOKAIs
#-------------------------------------------------------
if [[ -z $NO_MOKAI ]]; then
  cd ./mokai
  # w/ Evan Blue
 # ./launch_mokai.sh $TIME_WARP $JUST_BUILD -e -b -s >& /dev/null &
  # w/ Felix Red
 # ./launch_mokai.sh $TIME_WARP $JUST_BUILD -f -r -s >& /dev/null &
  cd ..
fi

#-------------------------------------------------------
#  Part 4: Launching shoreside
#-------------------------------------------------------
if [[ -z $NO_SHORESIDE ]]; then
  cd ./shoreside
  ./launch_shoreside_show.sh $TIME_WARP $JUST_BUILD >& /dev/null &
  cd ..
fi

sleep 3
#-------------------------------------------------------
#  Part 4: Launching uMAC
#-------------------------------------------------------
uMAC shoreside/targ_shoreside.moos

#-------------------------------------------------------
#  Part 5: Killing all processes launched from script
#-------------------------------------------------------
echo "Killing Simulation..."
kill -- -$$
# sleep is to give enough time to all processes to die
sleep 3
echo "All processes killed"
