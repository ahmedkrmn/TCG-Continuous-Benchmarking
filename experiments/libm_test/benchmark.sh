#!/bin/bash

############################SCRIPT OPTIONS############################
# Set qemu build absolute path and command line arguments for math.c
QEMU_BUILD="/home/ahmedkrmn/Desktop/GSoC2020/build/qemu-5.1.0/build-gcc"
ARGS="-i 3000"
TARGET_COMPILERS=(
    # aarch64-linux-gnu-gcc
    # alpha-linux-gnu-gcc
    # arm-linux-gnueabi-gcc
    # hppa-linux-gnu-gcc
    # mips-linux-gnu-gcc
    # mipsel-linux-gnu-gcc
    # mips64-linux-gnuabi64-gcc
    # mips64el-linux-gnuabi64-gcc
    m68k-linux-gnu-gcc
    # powerpc-linux-gnu-gcc
    # powerpc64-linux-gnu-gcc
    # powerpc64le-linux-gnu-gcc
    # riscv64-linux-gnu-gcc
    # sh4-linux-gnu-gcc
    # sparc64-linux-gnu-gcc
    s390x-linux-gnu-gcc
    # gcc
)
######################################################################

# Get TARGET_NAMES from the TARGET_COMPILERS array
TARGET_NAMES=()
for TARGET_COMPILER in ${TARGET_COMPILERS[@]}; do
    TARGET_NAME=$(echo $TARGET_COMPILER | awk -F- '{print $1}')
    if [ "$TARGET_NAME" == "powerpc" ]; then
        TARGET_NAME="ppc"
    elif [ "$TARGET_NAME" = "powerpc64" ]; then
        TARGET_NAME="ppc64"
    elif [ "$TARGET_NAME" = "powerpc64le" ]; then
        TARGET_NAME="ppc64le"
    elif [ "$TARGET_NAME" = "gcc" ]; then
        TARGET_NAME="x86_64"
    fi
    TARGET_NAMES+=($TARGET_NAME)
done

# Compile math.c for the selected targets
echo "--------------------STARTING COMPILATION------------------------"
mkdir executables
for i in ${!TARGET_COMPILERS[@]}; do
    echo "Compiling for ${TARGET_NAMES[$i]}"
    ${TARGET_COMPILERS[$i]} -O2 -static math.c -lm -o executables/math-${TARGET_NAMES[$i]}
done

# Execute all selected targets
echo "---------------------STARTING EXECUTION-------------------------"
mkdir output
for TARGET_NAME in ${TARGET_NAMES[@]}; do
    echo "Executing for $TARGET_NAME"
    $QEMU_BUILD/$TARGET_NAME-linux-user/qemu-$TARGET_NAME executables/math-$TARGET_NAME \
        $ARGS >output/math-results-$TARGET_NAME.txt
done
echo "----------------------------------------------------------------"

# Call data extraction script
python extract_data.py

# Clear intermediate files
rm -rf executables output
