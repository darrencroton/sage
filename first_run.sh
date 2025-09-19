#!/bin/bash

# SAGE First Run Setup Script
# This script sets up SAGE from a fresh GitHub clone by:
# 1. Creating necessary directories
# 2. Downloading mini-Millennium simulation trees
# 3. Setting up Python plotting environment
# 4. Configuring parameter files with correct paths

set -e  # Exit on any error

echo "=========================================="
echo "SAGE First Run Setup Script"
echo "=========================================="
echo ""
echo "Welcome to the Semi-Analytic Galaxy Evolution (SAGE) model!"
echo "This script will set up SAGE from a fresh GitHub clone."
echo ""
echo "For the latest updates, visit: https://github.com/darrencroton/sage"
echo ""

# Ensure we're in the correct directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd "$SCRIPT_DIR"

if [[ ! -f "README.md" ]] || [[ ! -f "Makefile" ]] || [[ ! -d "code" ]]; then
    echo "ERROR: This script must be run from the root SAGE directory."
    echo "Please ensure you're in the correct directory and try again."
    exit 1
fi

echo "✓ Confirmed we're in the SAGE root directory: $SCRIPT_DIR"
echo ""

# Step 1: Create directory structure
echo "Step 1: Creating directory structure..."
echo "----------------------------------------"

echo "Creating ./input/data/millennium directory..."
mkdir -p input/data/millennium
echo "✓ Created input/data/millennium"

echo "Creating ./output/results/millennium directory..."
mkdir -p output/results/millennium
echo "✓ Created output/results/millennium"
echo ""

# Step 2: Download mini-Millennium simulation trees
echo "Step 2: Downloading mini-Millennium simulation trees..."
echo "--------------------------------------------------------"

cd input/data/millennium

if [[ -f "trees_063.7" ]] && [[ -f "millennium.a_list" ]]; then
    echo "✓ Mini-Millennium trees already present. Skipping download."
else
    echo "Checking for download tools (wget or curl)..."
    
    DOWNLOAD_CMD=""
    if command -v wget &> /dev/null; then
        echo "✓ Using wget for download"
        DOWNLOAD_CMD="wget"
    elif command -v curl &> /dev/null; then
        echo "✓ Using curl for download"
        DOWNLOAD_CMD="curl -L -O"
    else
        echo "ERROR: Neither wget nor curl are available."
        echo "Please install wget or curl to download the simulation trees."
        exit 1
    fi
    
    echo "Downloading mini-Millennium tree files..."
    if [[ "$DOWNLOAD_CMD" == "wget" ]]; then
        wget "https://www.dropbox.com/s/l5ukpo7ar3rgxo4/mini-millennium-treefiles.tar?dl=0" -O "mini-millennium-treefiles.tar"
    else
        curl -L -o "mini-millennium-treefiles.tar" "https://www.dropbox.com/s/l5ukpo7ar3rgxo4/mini-millennium-treefiles.tar?dl=0"
    fi
    
    if [[ $? -ne 0 ]]; then
        echo "ERROR: Failed to download tree files."
        echo "Please check your internet connection and try again."
        echo "If the problem persists, please open an issue at:"
        echo "https://github.com/darrencroton/sage/issues"
        exit 1
    fi
    
    echo "✓ Downloaded tree files successfully"
    echo "Extracting tree files..."
    
    tar -xf mini-millennium-treefiles.tar
    if [[ $? -ne 0 ]]; then
        echo "ERROR: Failed to extract tree files."
        echo "The downloaded file may be corrupted. Please try again."
        exit 1
    fi
    
    rm -f mini-millennium-treefiles.tar
    echo "✓ Extracted and cleaned up tree files"
fi

echo "✓ Mini-Millennium trees are ready in: $(pwd)"
echo ""

# Step 3: Set up Python plotting environment
echo "Step 3: Setting up Python plotting environment..."
echo "--------------------------------------------------"

cd "$SCRIPT_DIR"

# Check Python version
if command -v python3 &> /dev/null; then
    PYTHON_CMD="python3"
    PIP_CMD="pip3"
elif command -v python &> /dev/null; then
    PYTHON_VERSION=$(python --version 2>&1 | cut -d' ' -f2 | cut -d'.' -f1)
    if [[ "$PYTHON_VERSION" == "3" ]]; then
        PYTHON_CMD="python"
        PIP_CMD="pip"
    else
        echo "ERROR: Python 3.x is required but only Python 2.x was found."
        echo "Please install Python 3.x and try again."
        exit 1
    fi
else
    echo "ERROR: Python is not installed or not in PATH."
    echo "Please install Python 3.x and try again."
    exit 1
fi

echo "✓ Found Python: $($PYTHON_CMD --version)"

# Check if pip is available
if ! command -v "$PIP_CMD" &> /dev/null; then
    echo "ERROR: pip is not available."
    echo "Please install pip for Python 3 and try again."
    exit 1
fi

echo "✓ Found pip: $($PIP_CMD --version | head -1)"

# Set up Python virtual environment and install packages
echo "Setting up Python virtual environment for sage-plot..."

# Check if requirements.txt exists
if [[ ! -f "requirements.txt" ]]; then
    echo "ERROR: requirements.txt file not found."
    exit 1
fi

VENV_DIR="sage_venv"

# Check if virtual environment already exists
if [[ -d "$VENV_DIR" ]]; then
    echo "✓ Virtual environment '$VENV_DIR' already exists"
else
    echo "Creating virtual environment '$VENV_DIR'..."
    $PYTHON_CMD -m venv "$VENV_DIR"
    if [[ $? -ne 0 ]]; then
        echo "ERROR: Failed to create virtual environment."
        echo "Please ensure you have python3-venv installed:"
        echo "  Ubuntu/Debian: sudo apt install python3-venv"
        echo "  macOS: Should be included with Python"
        exit 1
    fi
    echo "✓ Created virtual environment '$VENV_DIR'"
fi

# Activate the virtual environment
echo "Activating virtual environment..."
source "$VENV_DIR/bin/activate"

# Update pip in the virtual environment
echo "Updating pip in virtual environment..."
pip install --upgrade pip &> /dev/null

# Check if packages are already installed in the virtual environment
echo "Checking required Python packages in virtual environment..."
REQUIRED_PACKAGES=("numpy" "matplotlib" "tqdm")
MISSING_PACKAGES=()

for package in "${REQUIRED_PACKAGES[@]}"; do
    if ! python -c "import $package" &> /dev/null; then
        MISSING_PACKAGES+=("$package")
    else
        echo "✓ Found $package in virtual environment"
    fi
done

if [[ ${#MISSING_PACKAGES[@]} -gt 0 ]]; then
    echo ""
    echo "Installing missing packages: ${MISSING_PACKAGES[*]}"
    echo "This may take a few minutes..."
    
    pip install -r requirements.txt
    if [[ $? -ne 0 ]]; then
        echo "ERROR: Failed to install packages in virtual environment."
        echo "Please try installing manually:"
        echo "  source $VENV_DIR/bin/activate"
        echo "  pip install -r requirements.txt"
        exit 1
    fi
    
    # Verify installation
    echo ""
    echo "Verifying package installation in virtual environment..."
    ALL_INSTALLED=true
    for package in "${REQUIRED_PACKAGES[@]}"; do
        if ! python -c "import $package" &> /dev/null; then
            echo "✗ $package still not available"
            ALL_INSTALLED=false
        else
            echo "✓ $package is now available"
        fi
    done
    
    if [[ "$ALL_INSTALLED" == true ]]; then
        echo "✓ All packages successfully installed and verified"
    else
        echo "⚠ Some packages may not be properly installed"
        exit 1
    fi
else
    echo "✓ All required packages are already installed in virtual environment"
fi

# Deactivate virtual environment for now
deactivate

echo ""

# Step 4: Configure parameter file with correct paths
echo "Step 4: Configuring parameter file..."
echo "--------------------------------------"

cd "$SCRIPT_DIR"

if [[ ! -f "input/millennium.par" ]]; then
    echo "ERROR: Parameter file input/millennium.par not found."
    exit 1
fi

echo "Updating paths in millennium.par..."

# Create backup
cp input/millennium.par input/millennium.par.backup
echo "✓ Created backup: input/millennium.par.backup"

# Update paths to absolute paths
NEW_OUTPUT_DIR="OutputDir              $SCRIPT_DIR/output/results/millennium/"
NEW_SIMULATION_DIR="SimulationDir               $SCRIPT_DIR/input/data/millennium/"
NEW_SNAP_LIST="FileWithSnapList            $SCRIPT_DIR/input/data/millennium/millennium.a_list"

# Use sed to update the paths (compatible with both macOS and Linux)
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    sed -i "" "s|^OutputDir.*|$NEW_OUTPUT_DIR|g" input/millennium.par
    sed -i "" "s|^SimulationDir.*|$NEW_SIMULATION_DIR|g" input/millennium.par
    sed -i "" "s|^FileWithSnapList.*|$NEW_SNAP_LIST|g" input/millennium.par
else
    # Linux
    sed -i "s|^OutputDir.*|$NEW_OUTPUT_DIR|g" input/millennium.par
    sed -i "s|^SimulationDir.*|$NEW_SIMULATION_DIR|g" input/millennium.par
    sed -i "s|^FileWithSnapList.*|$NEW_SNAP_LIST|g" input/millennium.par
fi

echo "✓ Updated parameter file with absolute paths"
echo ""

# Step 5: Final validation
echo "Step 5: Validating setup..."
echo "---------------------------"

VALIDATION_ERRORS=()

# Check if tree files exist
if [[ ! -f "input/data/millennium/trees_063.0" ]]; then
    VALIDATION_ERRORS+=("Tree files not found in input/data/millennium/")
fi

# Check if a_list file exists
if [[ ! -f "input/data/millennium/millennium.a_list" ]]; then
    VALIDATION_ERRORS+=("millennium.a_list file not found")
fi

# Check if output directory exists
if [[ ! -d "output/results/millennium" ]]; then
    VALIDATION_ERRORS+=("Output directory not created")
fi

# Check if SAGE binary exists (optional)
if [[ -f "sage" ]]; then
    echo "✓ SAGE binary found"
elif [[ -f "code/sage" ]]; then
    echo "✓ SAGE binary found in code/"
else
    echo "ℹ SAGE binary not found - you'll need to compile it with 'make'"
fi

if [[ ${#VALIDATION_ERRORS[@]} -eq 0 ]]; then
    echo "✓ All validation checks passed"
else
    echo "⚠ Validation warnings:"
    for error in "${VALIDATION_ERRORS[@]}"; do
        echo "  - $error"
    done
fi

echo ""
echo "=========================================="
echo "SAGE Setup Complete!"
echo "=========================================="
echo ""
echo "Next steps:"
echo "1. Compile SAGE:"
echo "   make"
echo ""
echo "2. Run SAGE:"
echo "   ./sage input/millennium.par"
echo ""
echo "3. Generate plots (using the virtual environment):"
echo "   source sage_venv/bin/activate"
echo "   cd output/sage-plot"
echo "   python sage-plot.py --param-file=../../input/millennium.par"
echo "   deactivate  # when done with plotting"
echo ""
echo "Virtual Environment Info:"
echo "- Python packages are installed in: sage_venv/"
echo "- Activate with: source sage_venv/bin/activate"
echo "- Deactivate with: deactivate"
echo "- Always activate before running plotting scripts"
echo ""
echo "For more information, see the README.md file."
echo "If you encounter any issues, please visit:"
echo "https://github.com/darrencroton/sage/issues"
echo ""
echo "Happy galaxy formation modeling!"