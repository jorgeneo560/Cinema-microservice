#!/bin/bash

# Script to regenerate Cinema Microservice Documentation

echo " Cinema Microservice Documentation Generator"

# Check if Doxygen is installed
if ! command -v doxygen >/dev/null 2>&1; then
    echo " Doxygen not found!"
    echo "Install it with: sudo apt install doxygen graphviz"
    exit 1
fi

echo " Regenerating documentation..."

# Remove old documentation
if [ -d "docs" ]; then
    echo "  Removing old documentation..."
    rm -rf docs
fi

# Generate new documentation
echo "Running Doxygen..."
doxygen > /dev/null 2>&1

# Check if generation was successful
if [ $? -eq 0 ] && [ -f "docs/html/index.html" ]; then
    echo " Documentation generated successfully!"
    echo ""
    echo " Documentation Statistics:"
    echo "   Classes documented: $(grep -c 'class.*{' server/lib/*.hpp client/lib/*.hpp)"
    echo "   Source files: $(find server client -name '*.cpp' -o -name '*.hpp' | wc -l)"
    echo "   HTML files generated: $(find docs/html -name '*.html' | wc -l)"
    echo ""
    echo " View documentation:"
    echo "   ./view_docs.sh"
    echo "   or open: docs/html/index.html"
else
    echo " Error generating documentation!"
    echo "Check Doxyfile configuration."
    exit 1
fi