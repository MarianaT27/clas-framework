#!/bin/bash
# Build documentation for CLAS12 Analysis Framework
# Usage: ./scripts/build_docs.sh [--serve]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_ROOT"

echo "=========================================="
echo "  Building CLAS12 Framework Documentation"
echo "=========================================="

# Check for required tools
check_command() {
    if ! command -v "$1" &> /dev/null; then
        echo "Error: $1 is not installed"
        echo "Install with: $2"
        exit 1
    fi
}

check_command doxygen "brew install doxygen (macOS) or apt install doxygen (Linux)"
check_command mkdocs "pip install mkdocs mkdocs-material mkdocstrings"

# Step 1: Generate Doxygen API documentation
echo ""
echo "Step 1: Generating Doxygen API documentation..."
echo "------------------------------------------------"

mkdir -p docs/api
doxygen Doxyfile

echo "Doxygen documentation generated in docs/api/"

# Step 2: Copy MkDocs source to docs root
echo ""
echo "Step 2: Preparing MkDocs source..."
echo "-----------------------------------"

# Create docs structure for MkDocs
mkdir -p docs/site

# Copy mkdocs content to expected location
if [ -d "docs/mkdocs" ]; then
    cp -r docs/mkdocs/* docs/
fi

# Step 3: Build MkDocs
echo ""
echo "Step 3: Building MkDocs documentation..."
echo "-----------------------------------------"

if [ "$1" == "--serve" ]; then
    echo "Starting MkDocs development server..."
    echo "Open http://127.0.0.1:8000 in your browser"
    mkdocs serve
else
    mkdocs build -d docs/site
    echo "MkDocs documentation built in docs/site/"
fi

echo ""
echo "=========================================="
echo "  Documentation build complete!"
echo "=========================================="
echo ""
echo "Output locations:"
echo "  - Doxygen API docs: docs/api/html/index.html"
echo "  - MkDocs site:      docs/site/index.html"
echo ""
echo "To serve locally: ./scripts/build_docs.sh --serve"
