#!/bin/bash

outputDir="../bin/www/dist/"
mkdir -p "$outputDir"

if [ -d "$outputDir"/assets ]; then
    rm -rf "$outputDir"assets
else
    echo "assets directory does not exist, skipping deletion."
fi

if command -v npm >/dev/null 2>&1; then
    echo "Installing dependencies..."
    #npm install
    echo "Building Vue project..."
    npm run build
else
    echo "npm is not installed. Please install npm first."
    exit 1
fi

if [ -d ./dist ]; then
    echo "Copying output directory..."
    cp -R ./dist/* "$outputDir"
else
    echo "dist directory does not exist. Did the build process fail?"
    exit 1
fi