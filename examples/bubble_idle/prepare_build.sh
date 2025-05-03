#!/bin/bash

STEAM_SECRETS_FILE="/home/vittorio/steamSecrets.txt"

if [ ! -f "$STEAM_SECRETS_FILE" ]; then
    echo "Error: Steam secrets file not found at '$STEAM_SECRETS_FILE'"
    echo "Please create it with the username on the first line and the password on the second line."
    echo "Ensure it has appropriate permissions (e.g., chmod 600 $STEAM_SECRETS_FILE)."
    exit 1
fi

if [ ! -r "$STEAM_SECRETS_FILE" ]; then
    echo "Error: Cannot read Steam secrets file at '$STEAM_SECRETS_FILE'. Check permissions."
    exit 1
fi

STEAM_USER=$(head -n 1 "$STEAM_SECRETS_FILE" | tr -d '\n\r')
STEAM_PASS=$(sed -n '2p' "$STEAM_SECRETS_FILE" | tr -d '\n\r')

BBPREFIX="/c/OHWorkspace/BubbleIdleRel"

set -e

rm -Rf $BBPREFIX/WIP/
rm -Rf $BBPREFIX/WIP_TEST/
rm -Rf $BBPREFIX/WIP_DEMO_TEST/
rm -Rf $BBPREFIX/STEAM_BUILD/
rm -Rf $BBPREFIX/STEAM_BUILD_TEST/
rm -Rf $BBPREFIX/STEAM_BUILD_DEMO/
rm -Rf $BBPREFIX/STEAM_BUILD_DEMO_TEST/

mkdir -p $BBPREFIX/WIP/
mkdir -p $BBPREFIX/WIP_TEST/
mkdir -p $BBPREFIX/WIP_DEMO_TEST/
mkdir -p $BBPREFIX/STEAM_BUILD/
mkdir -p $BBPREFIX/STEAM_BUILD_TEST/
mkdir -p $BBPREFIX/STEAM_BUILD_DEMO/
mkdir -p $BBPREFIX/STEAM_BUILD_DEMO_TEST/

cp /c/OHWorkspace/SFML/build_vrdev_clang_rel/bin/bubble_idle.exe $BBPREFIX/WIP/
mv $BBPREFIX/WIP/bubble_idle.exe $BBPREFIX/WIP/BubbleByte.exe
cp -R /c/OHWorkspace/SFML/examples/bubble_idle/resources $BBPREFIX/WIP/
cp /c/OHWorkspace/SFML/examples/bubble_idle/steam_appid.txt $BBPREFIX/WIP/
cp /c/OHWorkspace/SFML/examples/bubble_idle/steam_api64.lib $BBPREFIX/WIP/
cp /c/OHWorkspace/SFML/examples/bubble_idle/steam_api64.dll $BBPREFIX/WIP/
cp /c/msys64/ucrt64/bin/libwinpthread-1.dll $BBPREFIX/WIP/
cp /c/msys64/ucrt64/bin/libstdc++-6.dll $BBPREFIX/WIP/
cp /c/msys64/ucrt64/bin/libgcc_s_seh-1.dll $BBPREFIX/WIP/
cp /c/msys64/ucrt64/bin/zlib1.dll $BBPREFIX/WIP/

rm $BBPREFIX/WIP/steam_appid.txt

cp -R $BBPREFIX/WIP/* $BBPREFIX/STEAM_BUILD/
cp -R $BBPREFIX/WIP/* $BBPREFIX/STEAM_BUILD_DEMO/

rm $BBPREFIX/STEAM_BUILD_DEMO/BubbleByte.exe
cp /c/OHWorkspace/SFML/build_vrdev_clang_rel/bin/bubble_idle_demo.exe $BBPREFIX/STEAM_BUILD_DEMO/BubbleByteDemo.exe

/c/OHWorkspace/steamworks/sdk/tools/ContentBuilder/builder/steamcmd.exe +login "$STEAM_USER" "$STEAM_PASS" +drm_wrap 3499760 "$BBPREFIX/STEAM_BUILD/BubbleByte.exe" "$BBPREFIX/STEAM_BUILD/BubbleByte_drm.exe" drmtoolp 6 +quit
mv $BBPREFIX/STEAM_BUILD/BubbleByte_drm.exe $BBPREFIX/STEAM_BUILD/BubbleByte.exe

/c/OHWorkspace/steamworks/sdk/tools/ContentBuilder/builder/steamcmd.exe +login "$STEAM_USER" "$STEAM_PASS" +drm_wrap 3692120 "$BBPREFIX/STEAM_BUILD_DEMO/BubbleByteDemo.exe" "$BBPREFIX/STEAM_BUILD_DEMO/BubbleByte_drm.exe" drmtoolp 6 +quit
mv $BBPREFIX/STEAM_BUILD_DEMO/BubbleByte_drm.exe $BBPREFIX/STEAM_BUILD_DEMO/BubbleByteDemo.exe

#
#
# ---------------------------------------------------------------
# UPX STEP
# ---------------------------------------------------------------

# # Process WIP .exe files
# for file in $BBPREFIX/WIP/*.exe; do
#     [ -f "$file" ] && upx -9 "$file" &
# done
#
# # Process WIP .dll files
# for file in $BBPREFIX/WIP/*.dll; do
#     [ -f "$file" ] && upx -9 "$file" &
# done
#
# # Process STEAM_BUILD .exe files
# for file in $BBPREFIX/STEAM_BUILD/*.exe; do
#     [ -f "$file" ] && upx -9 "$file" &
# done
#
# # Process STEAM_BUILD .dll files
# for file in $BBPREFIX/STEAM_BUILD/*.dll; do
#     [ -f "$file" ] && upx -9 "$file" &
# done

# Wait for all background tasks to finish
wait

cp -R $BBPREFIX/WIP/* $BBPREFIX/WIP_TEST/
cp -R $BBPREFIX/STEAM_BUILD/* $BBPREFIX/STEAM_BUILD_TEST/
cp -R $BBPREFIX/STEAM_BUILD_DEMO/* $BBPREFIX/STEAM_BUILD_DEMO_TEST/

cp -R $BBPREFIX/WIP/* $BBPREFIX/WIP_DEMO_TEST/
cp /c/OHWorkspace/SFML/build_vrdev_clang_rel/bin/bubble_idle_demo.exe $BBPREFIX/WIP_DEMO_TEST/BubbleByteDemo.exe
