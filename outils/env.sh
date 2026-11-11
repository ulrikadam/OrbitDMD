# A sourcer dans Git Bash avant de compiler :  source outils/env.sh
# Ajoute le compilateur Arm (aarch64-none-elf) et GNU make au PATH.
T="$LOCALAPPDATA/rpi2dmd-outils"
export PATH="$T/arm-aarch64/bin:$PATH:$T/xpack-windows-build-tools-4.4.1-3/bin"
export MSYS_NO_PATHCONV=1
