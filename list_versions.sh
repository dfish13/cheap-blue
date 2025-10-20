#!/bin/bash
# List all archived versions with metadata
# Usage: ./list_versions.sh

ARCHIVE_DIR="versions"

if [ ! -d "$ARCHIVE_DIR" ]; then
    echo "No versions archived yet. Use ./archive_version.sh to create one."
    exit 0
fi

# Count versions
VERSION_COUNT=$(find "$ARCHIVE_DIR" -type f -not -name "*.meta" | wc -l | tr -d ' ')

if [ "$VERSION_COUNT" -eq 0 ]; then
    echo "No versions archived yet. Use ./archive_version.sh to create one."
    exit 0
fi

echo "Archived Versions ($VERSION_COUNT total)"
echo "=========================================="
echo ""

# List all binaries (excluding .meta files)
for binary in "$ARCHIVE_DIR"/*; do
    # Skip metadata files
    if [[ "$binary" == *.meta ]]; then
        continue
    fi

    VERSION_NAME=$(basename "$binary")
    META_FILE="$binary.meta"

    echo "[$VERSION_NAME]"

    if [ -f "$META_FILE" ]; then
        # Parse and display metadata
        while IFS=': ' read -r key value; do
            case "$key" in
                description)
                    echo "  Description: $value"
                    ;;
                timestamp)
                    echo "  Timestamp:   $value"
                    ;;
                git_commit)
                    echo "  Git commit:  $value"
                    ;;
                binary_size)
                    # Convert bytes to human readable
                    size_kb=$((value / 1024))
                    echo "  Size:        ${size_kb} KB"
                    ;;
            esac
        done < "$META_FILE"
    else
        echo "  (No metadata available)"
    fi

    echo ""
done
