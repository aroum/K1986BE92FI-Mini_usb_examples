#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

CLEAN=false
SIZE=false
FLASH=false
PROJECT=""

show_help() {
    echo "Usage: ./build_all.sh <project> [options]"
    echo ""
    echo "Projects:"
    echo "  vcom      USB CDC echo"
    echo "  keyboard  USB HID keyboard"
    echo "  midi      USB MIDI"
    echo ""
    echo "Options:"
    echo "  -c, --clean    Remove build directory and re-run CMake"
    echo "  -s, --size     Show memory usage report"
    echo "  -f, --flash    Flash via OpenOCD (J-Link)"
    echo "  -h, --help     Show this help"
    echo ""
    echo "Example: ./build_all.sh vcom -csf"
}

resolve_project() {
    case "$1" in
        vcom)
            PROJECT_DIR="${SCRIPT_DIR}/vcom"
            TARGET_NAME="vcom_echo"
            ;;
        keyboard)
            PROJECT_DIR="${SCRIPT_DIR}/keyboard"
            TARGET_NAME="K1986BE92FI-Mini_usb_examples"
            ;;
        midi)
            PROJECT_DIR="${SCRIPT_DIR}/midi"
            TARGET_NAME="FREERTOS-Milandr-template"
            ;;
        *)
            echo -e "${RED}Unknown project: $1${NC}"
            echo "Valid projects: vcom, keyboard, midi"
            exit 1
            ;;
    esac
}

parse_short_opts() {
    local flags="${1#-}"
    local i c
    for ((i = 0; i < ${#flags}; i++)); do
        c="${flags:$i:1}"
        case "$c" in
            c) CLEAN=true ;;
            s) SIZE=true ;;
            f) FLASH=true ;;
            h) show_help; exit 0 ;;
            *)
                echo -e "${RED}Unknown option: -${c}${NC}"
                show_help
                exit 1
                ;;
        esac
    done
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --clean) CLEAN=true; shift ;;
        --size)  SIZE=true; shift ;;
        --flash) FLASH=true; shift ;;
        --help)  show_help; exit 0 ;;
        -)
            shift
            break
            ;;
        -?*)
            parse_short_opts "$1"
            shift
            ;;
        *)
            if [[ -z "$PROJECT" ]]; then
                PROJECT="$1"
                resolve_project "$PROJECT"
                shift
            else
                echo -e "${RED}Unexpected argument: $1${NC}"
                show_help
                exit 1
            fi
            ;;
    esac
done

if [[ -z "$PROJECT" ]]; then
    show_help
    exit 1
fi

SRC_DIR="${PROJECT_DIR}/src"
BUILD_DIR="${SRC_DIR}/build"
HRD_PROBE="${SCRIPT_DIR}/dep/probe/jlink4swd.cfg"

if [[ "$CLEAN" == true ]]; then
    echo -e "${YELLOW}=== Cleaning ${PROJECT} build directory ===${NC}"
    rm -rf "$BUILD_DIR"
fi

if [[ ! -d "$BUILD_DIR" ]]; then
    echo -e "${YELLOW}=== Initializing CMake (${PROJECT}) ===${NC}"
    mkdir -p "$BUILD_DIR"
    cmake -G "Unix Makefiles" -S "$SRC_DIR" -B "$BUILD_DIR"
fi

echo -e "${YELLOW}=== Building ${PROJECT} (${TARGET_NAME}) ===${NC}"
if [[ "$OSTYPE" == darwin* ]]; then
    JOBS=$(sysctl -n hw.ncpu 2>/dev/null || echo 4)
else
    JOBS=$(nproc 2>/dev/null || echo 4)
fi

cmake --build "$BUILD_DIR" -j"$JOBS"

echo -e "${GREEN}=== Build successful: ${PROJECT} ===${NC}"
echo -e "Binary: ${BUILD_DIR}/${TARGET_NAME}"

ELF_FILE="${BUILD_DIR}/${TARGET_NAME}"
[[ -f "$ELF_FILE" ]] || ELF_FILE="${BUILD_DIR}/${TARGET_NAME}.elf"

if [[ "$SIZE" == true ]]; then
    if [[ ! -f "$ELF_FILE" ]]; then
        echo -e "${RED}ELF not found: ${ELF_FILE}${NC}"
        exit 1
    fi

    echo -e "\n${GREEN}=== Memory usage (${PROJECT}) ===${NC}"
    MAP_DATA=$(arm-none-eabi-size -A "$ELF_FILE" 2>/dev/null || size -A "$ELF_FILE")

    get_size() {
        local val
        val=$(echo "$MAP_DATA" | grep -E "^\.$1" | awk '{print $2}' | head -n 1)
        echo "${val:-0}"
    }

    TEXT=$(get_size "text")
    RODATA=$(get_size "rodata")
    DATA=$(get_size "data")
    BSS=$(get_size "bss")
    TOTAL_FLASH=$((TEXT + RODATA + DATA))
    TOTAL_RAM=$((DATA + BSS))
    MAX_FLASH=$((128 * 1024))
    MAX_RAM=$((32 * 1024))

    FLASH_PCT=$(echo "scale=2; $TOTAL_FLASH * 100 / $MAX_FLASH" | bc)
    RAM_PCT=$(echo "scale=2; $TOTAL_RAM * 100 / $MAX_RAM" | bc)

    echo -e "${YELLOW}Flash:${NC} $((TOTAL_FLASH / 1024)) KB (${FLASH_PCT}% of 128 KB)"
    echo -e "${YELLOW}RAM:${NC}   $((TOTAL_RAM / 1024)) KB (${RAM_PCT}% of 32 KB)"
fi

if [[ "$FLASH" == true ]]; then
    if [[ ! -f "$ELF_FILE" ]]; then
        echo -e "${RED}ELF not found: ${ELF_FILE}${NC}"
        exit 1
    fi

    BIN_FILE="${BUILD_DIR}/${TARGET_NAME}.bin"
    arm-none-eabi-objcopy -O binary "$ELF_FILE" "$BIN_FILE"

    if [[ ! -f "$HRD_PROBE" ]]; then
        echo -e "${RED}Probe config not found: ${HRD_PROBE}${NC}"
        exit 1
    fi

    echo -e "${YELLOW}=== Flashing ${PROJECT} ===${NC}"
    openocd -f "$HRD_PROBE" -c "init" -c "halt" \
        -c "program ${BIN_FILE} verify reset 0x08000000" -c "exit"
    echo -e "${GREEN}=== Flash successful ===${NC}"
fi
