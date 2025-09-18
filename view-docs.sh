#!/bin/bash

# Script to open Cinema Microservice Documentation

echo "Cinema Microservice Documentation Viewer"
echo "============================================="

# Check if documentation exists
if [ ! -f "docs/html/index.html" ]; then
    echo " Documentation not found!"
    echo "Run 'doxygen' first to generate documentation."
    exit 1
fi

echo " Opening documentation in browser..."

# Try different browsers
if command -v firefox >/dev/null 2>&1; then
    firefox docs/html/index.html &
    echo " Opened in Firefox"
elif command -v google-chrome >/dev/null 2>&1; then
    google-chrome docs/html/index.html &
    echo " Opened in Chrome"
elif command -v chromium-browser >/dev/null 2>&1; then
    chromium-browser docs/html/index.html &
    echo " Opened in Chromium"
elif command -v xdg-open >/dev/null 2>&1; then
    xdg-open docs/html/index.html &
    echo "Opened with default browser"
else
    echo " No browser found. Open this file manually:"
    echo "   file://$(pwd)/docs/html/index.html"
fi

echo ""
echo "Documentation available at: docs/html/index.html"
echo " Main features documented:"
echo "   • Cinema booking system architecture"
echo "   • WebSocket communication layer"
echo "   • User interface classes"
echo "   • Business logic services"
echo "   • Thread-safe data structures"