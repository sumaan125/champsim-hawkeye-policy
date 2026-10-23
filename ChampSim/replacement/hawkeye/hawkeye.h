#ifndef HAWKEYE_H
#define HAWKEYE_H
#include <vector>
#include "cache.h"
#include "modules.h"
#include "optgen.h"
#include "predictor.h"
#include "rrip.h"

struct hawkeye : public champsim::modules::replacement {
    OPTgen optgen;
    HawkeyePredictor predictor;
    std::vector<std::vector<int>> rrpv;

    hawkeye(CACHE* cache) 
        : champsim::modules::replacement(cache),
          optgen(cache->NUM_SET, cache->NUM_WAY),
          predictor(8192, 3),
          rrpv(cache->NUM_SET, std::vector<int>(cache->NUM_WAY, 0)) {}

    uint32_t find_victim(uint32_t triggering_cpu, uint64_t instr_id, long set,const CACHE::BLOCK* current_set, uint64_t ip, uint64_t full_addr, uint32_t type);
    void replacement_cache_fill(uint32_t triggering_cpu, long set, long way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type);
    void update_replacement_state(uint32_t triggering_cpu, long set, long way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type, uint8_t hit);
};
#endif