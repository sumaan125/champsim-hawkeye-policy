#ifndef OPTGEN_H
#define OPTGEN_H
#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

class OPTgen {
private:
    std::size_t num_sets;
    std::size_t associativity;
    std::size_t history_capacity;
    struct set_state {
        std::size_t curTime = 0;
        std::unordered_map<uint64_t, std::size_t> lastAccess;
        std::vector<std::size_t> occupancy;
        set_state(std::size_t history_capacity) : occupancy(history_capacity, 0){ }
    };
    std::vector<set_state> sets;
public:
    OPTgen(std::size_t num_sets,std::size_t associativity,std::size_t history_multiplier = 8);
    bool access(std::size_t set_idx, uint64_t address);
};
#endif