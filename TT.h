#ifndef TT_H
#define TT_H

#include <cstdint>
#include <cstring>
#include <mutex>

// Bound types for transposition table entries
enum TTFlag {
    TT_EXACT = 0,    // Exact score
    TT_ALPHA = 1,    // Upper bound (score <= alpha)
    TT_BETA = 2      // Lower bound (score >= beta)
};

struct tt_entry
{
    uint64_t key;        // Full 64-bit hash key for collision detection
    int16_t eval;        // Evaluation score
    int32_t best_move;   // Best move found (encoded as int)
    uint8_t depth;       // Search depth
    uint8_t flag;        // Bound type (TTFlag)
    uint8_t padding;     // Align to 16 bytes

    tt_entry() {
        // TODO: consider an object implementation where we can do a memset on a contiguous array of tt_entry.
        // Currently, the tt_entry and mutex are interleaved, which prevents us from doing a single memset on the whole table.
        std::memset(this, 0, sizeof(tt_entry));
    }
};

struct tt_row
{
    tt_entry e;
    std::mutex m;        // Acquire lock before reading or writing to entry. TODO enforce this with object oriented approach.
};

/**
 * @brief High-performance Transposition Table
 * Uses fixed-size array with better replacement scheme
 */
class TT
{
public:
    TT();
    ~TT();

    void init(size_t mb_size = 64);  // Initialize with size in MB

    void store(uint64_t hash, int eval, TTFlag flag, int depth, int best_move);
    bool probe(uint64_t hash, tt_entry& entry);

    // Statistics
    size_t size() const { return table_size; }
    double load_factor() const;
    void print_stats() const;

private:
    tt_row* table;

    size_t table_size;      // Number of entries (power of 2)
    size_t mask;            // table_size - 1 for fast modulo

    // Statistics
    mutable uint64_t probes;
    mutable uint64_t hits;
    mutable uint64_t collisions;

    size_t hash_index(uint64_t hash) const { return hash & mask; }
};

#endif