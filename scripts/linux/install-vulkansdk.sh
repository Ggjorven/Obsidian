#!/bin/bash

# Define variables
VULKAN_SDK_URL="https://sdk.lunarg.com/sdk/download/latest/linux/vulkan-sdk-linux-x86_64.tar.xz"
TEMP_DIR=$(mktemp -d)  # Create a safe temporary directory
FINAL_DIR="/usr/local/vulkan-sdk"
TAR_FILE="${TEMP_DIR}/vulkan-sdk.tar.xz"

# Function to handle errors and cleanup
cleanup() {
    echo "Cleaning up..."
    rm -rf "$TEMP_DIR"
}
trap cleanup EXIT  # Ensure cleanup runs on exit

# Ensure wget is installed
if ! command -v wget &>/dev/null; then
    echo "Error: wget is not installed. Install it using: sudo apt install wget"
    exit 1
fi

# Download the Vulkan SDK tarball
echo "Downloading Vulkan SDK..."
if ! wget -O "$TAR_FILE" "$VULKAN_SDK_URL"; then
    echo "Error downloading Vulkan SDK. Check the URL or your network connection."
    exit 1
fi

# Verify the file type
echo "Verifying downloaded file..."
if ! file "$TAR_FILE" | grep -q "XZ compressed data"; then
    echo "The downloaded file is not a valid XZ tarball. Check the download URL or try again."
    exit 1
fi

# Quick integrity check
echo "Checking archive integrity..."
if ! tar -tf "$TAR_FILE" &>/dev/null; then
    echo "Error: Corrupted tarball. Download might have failed."
    exit 1
fi

# Extract the tarball
echo "Extracting Vulkan SDK..."
if ! tar -xJvf "$TAR_FILE" -C "$TEMP_DIR" --no-same-owner; then
    echo "Error extracting Vulkan SDK."
    exit 1
fi

# Find the extracted version directory
VERSION_DIR=$(find "$TEMP_DIR" -mindepth 1 -maxdepth 1 -type d)
if [ -z "$VERSION_DIR" ]; then
    echo "Error: Could not find the version directory."
    ls -l "$TEMP_DIR"
    exit 1
fi

# Check for setup-env.sh
SETUP_ENV_SCRIPT="$VERSION_DIR/setup-env.sh"
if [ ! -f "$SETUP_ENV_SCRIPT" ]; then
    echo "Error: setup-env.sh not found in $VERSION_DIR."
    ls -l "$VERSION_DIR"
    exit 1
fi

# Move Vulkan SDK to final location
echo "Moving Vulkan SDK to $FINAL_DIR..."
sudo mkdir -p "$FINAL_DIR" && sudo cp -r "$VERSION_DIR/"* "$FINAL_DIR" || {
    echo "Failed to move Vulkan SDK. Ensure you have sudo permissions."
    exit 1
}

# Source environment script
SETUP_ENV_SCRIPT="$FINAL_DIR/setup-env.sh"
if [ -f "$SETUP_ENV_SCRIPT" ]; then
    echo "Setting up environment variables..."
    source "$SETUP_ENV_SCRIPT"
    
    # Ensure persistent environment setup
    if ! grep -q "source $FINAL_DIR/setup-env.sh" ~/.bashrc; then
        echo "source $FINAL_DIR/setup-env.sh" >> ~/.bashrc
        echo "Added Vulkan SDK setup to ~/.bashrc."
    fi
else
    echo "Error: setup-env.sh not found in $FINAL_DIR."
    exit 1
fi

echo "Vulkan SDK installation completed successfully!"
echo "Restart your terminal or run: source $FINAL_DIR/setup-env.sh"
