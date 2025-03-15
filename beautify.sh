#!/usr/bin/env zsh

# beautify.sh - Format C and Python code in the SAGE codebase
# Created: March 2025

# Display help information
show_help() {
    echo "Usage: ./beautify.sh [options]"
    echo ""
    echo "Format C and Python code in the SAGE codebase using industry-standard tools."
    echo ""
    echo "Options:"
    echo "  --help             Display this help message and exit"
    echo "  --c-only           Only format C code (using clang-format)"
    echo "  --py-only          Only format Python code (using black and isort)"
    echo ""
    echo "Requirements:"
    echo "  - clang-format     For C code formatting (install with 'brew install clang-format')"
    echo "  - black            For Python code formatting (install with 'pip install black')"
    echo "  - isort            For Python import sorting (install with 'pip install isort')"
    echo ""
    exit 0
}

# Process arguments
FORMAT_C=true
FORMAT_PY=true

for arg in "$@"; do
    case $arg in
        --help)
            show_help
            ;;
        --c-only)
            FORMAT_C=true
            FORMAT_PY=false
            ;;
        --py-only)
            FORMAT_C=false
            FORMAT_PY=true
            ;;
        *)
            echo "Unknown option: $arg"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Check for required tools
check_tool() {
  if ! command -v $1 &> /dev/null; then
    echo "${RED}Error: $1 is not installed or not in PATH${NC}"
    echo "To install: $2"
    return 1
  fi
  return 0
}

# Don't exit on error as we want to try all formatting stages
set +e

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" && pwd)"

# ANSI color codes
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

# Print banner
echo "${YELLOW}=== SAGE Code Beautifier ===${NC}"

# Format C code
if $FORMAT_C; then
    echo -n "Formatting C code... "
    if check_tool clang-format "brew install clang-format"; then
        if cd "${ROOT_DIR}" && find . \( -name "*.c" -o -name "*.h" \) | xargs clang-format -i -style=LLVM > /dev/null 2>&1; then
            echo "${GREEN}✓${NC}"
        else
            echo "${RED}✗${NC}"
            echo "${RED}Error formatting C code. See details below:${NC}"
            cd "${ROOT_DIR}" && find . \( -name "*.c" -o -name "*.h" \) | xargs clang-format -i -style=LLVM
        fi
    else
        echo "${RED}✗ (tool not found)${NC}"
    fi
fi

# Format Python code
if $FORMAT_PY; then
    # Format with Black
    echo -n "Formatting Python code with Black... "
    if check_tool black "pip install black"; then
        if black --quiet "${ROOT_DIR}" 2> /tmp/black_errors.log; then
            echo "${GREEN}✓${NC}"
        else
            echo "${RED}✗${NC}"
            echo "${RED}Black encountered errors:${NC}"
            cat /tmp/black_errors.log
        fi
    else
        echo "${RED}✗ (tool not found)${NC}"
    fi

    # Sort imports with isort
    echo -n "Sorting Python imports with isort... "
    if check_tool isort "pip install isort"; then
        if isort --profile black --quiet "${ROOT_DIR}" 2> /tmp/isort_errors.log; then
            echo "${GREEN}✓${NC}"
        else
            echo "${RED}✗${NC}"
            echo "${RED}isort encountered errors:${NC}"
            cat /tmp/isort_errors.log
        fi
    else
        echo "${RED}✗ (tool not found)${NC}"
    fi
fi

# Check if any errors occurred
if $FORMAT_PY; then
    if command -v black &> /dev/null && command -v isort &> /dev/null; then
        if [ -s /tmp/black_errors.log ] || [ -s /tmp/isort_errors.log ]; then
            echo "${YELLOW}Some Python files could not be formatted. See errors above.${NC}"
            echo "Tip: For Python 2 files, consider converting to Python 3 with '2to3 -w filename.py'"
            echo "     or manually adding parentheses to print statements."
        fi
    elif [ "$FORMAT_C" = true ] && [ "$FORMAT_PY" = true ]; then
        echo "${YELLOW}Note: Python formatting tools were not available.${NC}"
        echo "To install: pip install black isort"
    fi
fi

if $FORMAT_C && ! command -v clang-format &> /dev/null && [ "$FORMAT_PY" = true ]; then
    echo "${YELLOW}Note: C formatting tool (clang-format) was not available.${NC}"
    echo "To install: brew install clang-format"
fi

if { $FORMAT_C && command -v clang-format &> /dev/null; } || \
   { $FORMAT_PY && command -v black &> /dev/null && command -v isort &> /dev/null && [ ! -s /tmp/black_errors.log ] && [ ! -s /tmp/isort_errors.log ]; }; then
    echo "${GREEN}Formatting completed successfully for all available tools!${NC}"
fi

# Clean up temporary files
rm -f /tmp/black_errors.log /tmp/isort_errors.log

echo "${YELLOW}=== Formatting Complete ===${NC}"
