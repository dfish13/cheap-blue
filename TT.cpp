#include "TT.h"
#include <iostream>
#include <iomanip>

TT::TT() : table(nullptr), table_size(0), mask(0), current_age(0),
           probes(0), hits(0), collisions(0) {
}

TT::~TT() {
    if (table) {
        delete[] table;
    }
}

void TT::init(size_t mb_size) {
    // Calculate number of entries for the given MB size
    size_t entries = (mb_size * 1024 * 1024) / sizeof(tt_entry);

    // Round down to nearest power of 2
    table_size = 1;
    while (table_size * 2 <= entries) {
        table_size *= 2;
    }

    mask = table_size - 1;

    // Allocate and initialize table
    if (table) {
        delete[] table;
    }
    table = new tt_entry[table_size];
    clear();

    // Reset statistics
    probes = hits = collisions = 0;
    current_age = 0;
}

void TT::clear() {
    if (table) {
        std::memset(table, 0, table_size * sizeof(tt_entry));
    }
}

void TT::age() {
    current_age++;
    // Wrap around to prevent overflow
    if (current_age == 0) {
        current_age = 1;
    }
}

void TT::store(uint64_t hash, int eval, TTFlag flag, int depth, int best_move) {
    if (!table) return;

    size_t index = hash_index(hash);
    tt_entry* entry = &table[index];

    // Replacement scheme: replace if:
    // 1. Entry is empty (key == 0)
    // 2. Same position (key matches)
    // 3. Higher depth search
    // 4. Much older entry (different age and lower depth)
    bool should_replace = (entry->key == 0) ||
                         (entry->key == hash) ||
                         (depth >= entry->depth) ||
                         (entry->age != current_age && depth >= entry->depth - 2);

    if (should_replace) {
        entry->key = hash;
        entry->eval = static_cast<int16_t>(eval);
        entry->best_move = static_cast<int16_t>(best_move);
        entry->depth = static_cast<uint8_t>(depth);
        entry->flag = static_cast<uint8_t>(flag);
        entry->age = current_age;
    }
}

bool TT::probe(uint64_t hash, tt_entry& entry) {
    if (!table) return false;

    probes++;
    size_t index = hash_index(hash);
    tt_entry* stored = &table[index];

    if (stored->key == hash) {
        entry = *stored;
        hits++;
        return true;
    }

    if (stored->key != 0) {
        collisions++;
    }

    return false;
}

double TT::load_factor() const {
    if (!table || table_size == 0) return 0.0;

    size_t used_entries = 0;
    for (size_t i = 0; i < table_size; i++) {
        if (table[i].key != 0) {
            used_entries++;
        }
    }

    return static_cast<double>(used_entries) / table_size;
}

void TT::print_stats() const {
    std::cout << "=== Transposition Table Statistics ===\n";
    std::cout << "Size: " << table_size << " entries\n";
    std::cout << "Memory: " << (table_size * sizeof(tt_entry)) / (1024 * 1024) << " MB\n";
    std::cout << "Load factor: " << std::fixed << std::setprecision(2) << load_factor() * 100 << "%\n";
    std::cout << "Probes: " << probes << "\n";
    std::cout << "Hits: " << hits << "\n";
    std::cout << "Collisions: " << collisions << "\n";
    if (probes > 0) {
        std::cout << "Hit rate: " << std::fixed << std::setprecision(2)
                  << (static_cast<double>(hits) / probes) * 100 << "%\n";
    }
    std::cout << "=======================================\n";
}