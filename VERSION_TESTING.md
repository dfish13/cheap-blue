# Version Testing Framework

This framework allows you to archive different versions of the Cheap Blue UCI engine and play them against each other to measure improvement over time.

## Quick Start

1. **Build and archive a version:**
   ```bash
   make uci
   ./archive_version.sh v1.0 "Initial baseline with alpha-beta pruning"
   ```

2. **Make improvements to the code, then archive the new version:**
   ```bash
   make uci
   ./archive_version.sh v1.1 "Added better move ordering"
   ```

3. **Play a match between versions:**
   ```bash
   ./play_match.py v1.0 v1.1 -n 20
   ```

## Components

### 1. Version Archiving (`archive_version.sh`)

Archives a built UCI binary with metadata for future testing.

**Usage:**
```bash
./archive_version.sh <version_name> [description]
```

**Examples:**
```bash
./archive_version.sh v1.0 "Initial release"
./archive_version.sh baseline-2024-10-20 "Pre-optimization baseline"
./archive_version.sh dev-pv-sort "Testing PV sorting improvements"
```

**What it does:**
- Copies `bin/uci` to `versions/<version_name>`
- Creates a metadata file with:
  - Version name and description
  - Timestamp
  - Git commit hash (if available)
  - Binary size
- Prevents accidental overwrites of existing versions

### 2. Version Listing (`list_versions.sh`)

Lists all archived versions with their metadata.

**Usage:**
```bash
./list_versions.sh
```

**Output example:**
```
Archived Versions (3 total)
==========================================

[v1.0]
  Description: Initial baseline with alpha-beta pruning
  Timestamp:   2024-10-20 14:23:45
  Git commit:  3cb29a0
  Size:        425 KB

[v1.1]
  Description: Added better move ordering
  Timestamp:   2024-10-20 15:30:12
  Git commit:  4de37b1
  Size:        428 KB
```

### 3. Match Runner (`play_match.py`)

Plays matches between two engine versions using the UCI protocol.

**Requirements:**
```bash
pip install chess
```

**Usage:**
```bash
./play_match.py <engine1> <engine2> [options]
```

**Arguments:**
- `engine1`, `engine2`: Version names (e.g., `v1.0`) or direct paths (e.g., `bin/uci`)
- `-n, --games`: Number of games to play (default: 10)
- `-t, --time`: Time per move in milliseconds (default: 1000)
- `-o, --openings`: File with opening positions (one FEN per line)
- `-s, --save`: Save results to JSON file
- `-v, --verbose`: Show all moves during games

**Examples:**
```bash
# Quick 10-game match with default settings
./play_match.py v1.0 v1.1

# Longer match with more time
./play_match.py v1.0 v1.1 -n 50 -t 2000

# Test current development version against baseline
./play_match.py baseline current -n 20

# Use custom openings and save results
./play_match.py v1.0 v1.1 -n 100 -o openings.fen -s results.json

# Verbose output to see all moves
./play_match.py v1.0 v1.1 -n 5 -v
```

**How it works:**
- Plays games with alternating colors (game 1: engine1 is white, game 2: engine1 is black, etc.)
- Enforces time limits per move
- Detects wins, draws, stalemates, and other game endings
- Provides detailed statistics

**Output:**
```
Starting match: 20 games
Engine 1: versions/v1.0
Engine 2: versions/v1.1
Time per move: 1000ms
============================================================
Game 1/20: engine2 (45 moves) | Score: 0-1-0
Game 2/20: draw (73 moves) | Score: 0-1-1
Game 3/20: engine1 (62 moves) | Score: 1-1-1
...

============================================================
MATCH SUMMARY
============================================================
Engine 1: v1.0
Engine 2: v1.1

Results (20 games):
  Engine 1 wins: 7 (35.0%)
  Engine 2 wins: 10 (50.0%)
  Draws:         3 (15.0%)

Score: 8.5 - 11.5
Winner: Engine 2 (+3.0)
```

## Workflow Examples

### Testing a New Feature

```bash
# Archive current stable version
make uci
./archive_version.sh stable "Stable version before optimization"

# Implement your changes...
# (edit code, make improvements)

# Build and test
make uci
./play_match.py stable current -n 20

# If improvements are good, archive the new version
./archive_version.sh v1.1-optimized "Improved evaluation function"
```

### Regression Testing

```bash
# Compare multiple versions
./play_match.py v1.0 v1.1 -n 50 -s v1.0-vs-v1.1.json
./play_match.py v1.1 v1.2 -n 50 -s v1.1-vs-v1.2.json
./play_match.py v1.0 v1.2 -n 50 -s v1.0-vs-v1.2.json
```

### Using Opening Books

Create a file with opening positions (one FEN per line):

```
# openings.fen
rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1
rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2
```

Then use it in matches:
```bash
./play_match.py v1.0 v1.1 -n 30 -o openings.fen
```

## Directory Structure

```
cheap-blue/
├── bin/
│   └── uci              # Current built binary
├── versions/            # Archived versions (created automatically)
│   ├── v1.0
│   ├── v1.0.meta
│   ├── v1.1
│   └── v1.1.meta
├── archive_version.sh   # Archive a version
├── list_versions.sh     # List archived versions
├── play_match.py        # Run matches
└── VERSION_TESTING.md   # This file
```

## Tips

1. **Version Naming:**
   - Use semantic versioning: `v1.0`, `v1.1`, `v2.0`
   - Or use descriptive names: `baseline-oct20`, `dev-ttable`, `stable`
   - Include dates for time-based tracking: `2024-10-20-baseline`

2. **Match Length:**
   - Short tests (10-20 games): Quick feedback during development
   - Medium tests (50-100 games): More reliable results
   - Long tests (200+ games): High confidence for important changes

3. **Time Controls:**
   - Fast (500-1000ms): Quick testing, less accurate play
   - Medium (2000-5000ms): Good balance
   - Slow (10000ms+): More accurate play, better quality games

4. **Statistical Significance:**
   - Play enough games to see meaningful differences
   - A 55-45 split in 10 games might be noise
   - A 55-45 split in 100 games is likely significant

5. **Git Integration:**
   - Archive versions at meaningful git commits
   - The metadata includes the git hash for tracking
   - Use git tags to mark important versions

## Troubleshooting

**"Binary not found at bin/uci"**
- Run `make uci` first to build the binary

**"python-chess is not installed"**
- Install it: `pip install chess`

**Engines not responding**
- Check that the binary is executable
- Test the engine manually: `./bin/uci` and type `uci`
- Ensure engines support the UCI protocol

**Games taking too long**
- Reduce time per move: `-t 500`
- Reduce number of games: `-n 10`
