#! /bin/bash

####################################SCRIPT OPTIONS#####################################
# List of target compilers
TARGET_COMPILERS=(
    aarch64-linux-gnu-gcc
    alpha-linux-gnu-gcc
    arm-linux-gnueabi-gcc
    hppa-linux-gnu-gcc
    mips-linux-gnu-gcc
    mipsel-linux-gnu-gcc
    mips64-linux-gnuabi64-gcc
    mips64el-linux-gnuabi64-gcc
    m68k-linux-gnu-gcc
    powerpc-linux-gnu-gcc
    powerpc64-linux-gnu-gcc
    powerpc64le-linux-gnu-gcc
    riscv64-linux-gnu-gcc
    sh4-linux-gnu-gcc
    sparc64-linux-gnu-gcc
    s390x-linux-gnu-gcc
    gcc
)
# List of benchmark absolute paths
BENCHMARK_PATHS=(
    ./benchmarks/obscure_string/obscure_string.c
    ./benchmarks/reorder_string/reorder_string.c
    ./benchmarks/reverse_string/reverse_string.c
    ./benchmarks/shuffle_string/shuffle_string.c
    ./benchmarks/strfind_string/strfind_string.c
    ./benchmarks/toupper_string/toupper_string.c
)
# List of build-gcc populated QEMU absolute paths
QEMU_PATHS=(
    /build/qemu-master
    /build/qemu-5.0.0
    /build/qemu-4.2.0
    /build/qemu-4.1.0
    /build/qemu-4.0.0
    /build/qemu-3.1.0
    /build/qemu-3.0.0
)
#######################################################################################

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

# Get user machine name
MACHINE_NAME=$(uname -a | awk -F'[ ]' '{print $2}')

# Get start time
START=$(date +%s.%N)
echo $(date) - "Start of Execution"

# For each benchmark, do
for BENCHMARK_PATH in ${BENCHMARK_PATHS[@]}; do

    # Get benchmark file name from path
    # Pattern: /path/to/benchmark.c
    BENCHMARK_NAME=$(echo $BENCHMARK_PATH | awk -F/ '{print $NF}' | awk -F. '{print $1}')

    # For each target compiler, do
    for j in ${!TARGET_COMPILERS[@]}; do

        TARGET_NAME=${TARGET_NAMES[$j]}
        TARGET_COMPILER=${TARGET_COMPILERS[$j]}

        echo "-------------------------------------------------"
        echo $(date) - "Running the '$BENCHMARK_NAME' benchmark under the '$TARGET_NAME' target"
        echo "-------------------------------------------------"

        # Compile the benchmark using the target compiler
        $TARGET_COMPILER -O2 -static $BENCHMARK_PATH -o executable

        # For each QEMU
        for QEMU in ${QEMU_PATHS[@]}; do

            # Get QEMU version from path
            # Pattern: /path/to/qemu-verison
            QEMU_VERSION=$(echo $QEMU | awk -F/ '{print $NF}' | awk -F- '{print $2}')

            # Create the callgrind output directory for the benchmark and qemu version
            mkdir -p callgrind_output/$BENCHMARK_NAME/$QEMU_VERSION/$MACHINE_NAME

            # Run callgrind
            valgrind --tool=callgrind --dump-instr=yes --collect-jumps=yes \
                --callgrind-out-file=callgrind_output/$BENCHMARK_NAME/$QEMU_VERSION/$MACHINE_NAME/callgrind.out.$BENCHMARK_NAME-$TARGET_NAME.%p \
                $QEMU/build-gcc/$TARGET_NAME-linux-user/qemu-$TARGET_NAME executable

            if [ "$QEMU_VERSION" == "5.0.0" ]; then

                #If QEMU version is 5.0.0, save the control results to confirm that the emulation of all targets lead to the same control results.
                mkdir -p control_results_output/$BENCHMARK_NAME
                $QEMU/build-gcc/$TARGET_NAME-linux-user/qemu-$TARGET_NAME executable \
                    >control_results_output/$BENCHMARK_NAME/$BENCHMARK_NAME-$TARGET_NAME.txt

                #If QEMU version is 5.0.0, do further runs with the benchmark for different values of input
                #Try n = 10000, 20000, 30000, 40000, 50000, 60000
                for n in {10000..60000..10000}; do
                    mkdir -p callgrind_output/$BENCHMARK_NAME/$QEMU_VERSION-$n/$MACHINE_NAME
                    valgrind --tool=callgrind --dump-instr=yes --collect-jumps=yes \
                        --callgrind-out-file=callgrind_output/$BENCHMARK_NAME/$QEMU_VERSION-$n/$MACHINE_NAME/callgrind.out.$BENCHMARK_NAME-$TARGET_NAME.%p \
                        $QEMU/build-gcc/$TARGET_NAME-linux-user/qemu-$TARGET_NAME executable -n $n
                done
            fi
        done
    done
done

#
# Cleanup
# Remove leftover executable
#
rm executable

#Get finish time
END=$(date +%s.%N)
echo "-----------------------------------------------"
echo $(date) - "End of Execution"

# Calculate total runtime
dt=$(echo "$END - $START" | bc)
dd=$(echo "$dt/86400" | bc)
dt2=$(echo "$dt-86400*$dd" | bc)
dh=$(echo "$dt2/3600" | bc)
dt3=$(echo "$dt2-3600*$dh" | bc)
dm=$(echo "$dt3/60" | bc)
ds=$(echo "$dt3-60*$dm" | bc)

LC_NUMERIC=C printf "Total Runtime: %d:%02d:%02d:%02.4f\n" $dd $dh $dm $ds
