#!/bin/bash
# Archive a version of the UCI binary for comparison testing
# Usage: ./archive_version.sh <version_name> [description]

set -e

VERSION_NAME="$1"
DESCRIPTION="${2:-No description provided}"
ARCHIVE_DIR="versions"
BINARY="bin/uci"

if [ -z "$VERSION_NAME" ]; then
    echo "Usage: ./archive_version.sh <version_name> [description]"
    echo ""
    echo "Example: ./archive_version.sh v1.0 'Initial alpha-beta implementation'"
    exit 1
fi

# Validate version name (alphanumeric, dots, dashes, underscores only)
if [[ ! "$VERSION_NAME" =~ ^[a-zA-Z0-9._-]+$ ]]; then
    echo "Error: Version name must contain only letters, numbers, dots, dashes, and underscores"
    exit 1
fi

# Create archive directory structure if it doesn't exist
mkdir -p "$ARCHIVE_DIR"

# Check if binary exists
if [ ! -f "$BINARY" ]; then
    echo "Error: Binary not found at $BINARY"
    echo "Please run 'make uci' first"
    exit 1
fi

# Check if version already exists
if [ -f "$ARCHIVE_DIR/$VERSION_NAME" ]; then
    echo "Error: Version '$VERSION_NAME' already exists"
    echo "Use a different version name or delete the existing version"
    exit 1
fi

# Get git commit hash if in a git repository
GIT_HASH="unknown"
if git rev-parse --git-dir > /dev/null 2>&1; then
    GIT_HASH=$(git rev-parse --short HEAD)
    GIT_STATUS=$(git status --porcelain)
    if [ -n "$GIT_STATUS" ]; then
        GIT_HASH="${GIT_HASH}-dirty"
    fi
fi

# Copy binary to archive
cp "$BINARY" "$ARCHIVE_DIR/$VERSION_NAME"

# Create metadata file
TIMESTAMP=$(date +"%Y-%m-%d %H:%M:%S")
cat > "$ARCHIVE_DIR/$VERSION_NAME.meta" <<EOF
version: $VERSION_NAME
description: $DESCRIPTION
timestamp: $TIMESTAMP
git_commit: $GIT_HASH
binary_size: $(stat -f%z "$BINARY" 2>/dev/null || stat -c%s "$BINARY" 2>/dev/null || echo "unknown")
EOF

echo "✓ Archived version '$VERSION_NAME'"
echo "  Git commit: $GIT_HASH"
echo "  Timestamp: $TIMESTAMP"
echo "  Location: $ARCHIVE_DIR/$VERSION_NAME"
echo ""
echo "Metadata saved to: $ARCHIVE_DIR/$VERSION_NAME.meta"
