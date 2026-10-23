#include "hawkeye.h"
#include <algorithm>

uint32_t hawkeye::find_victim(uint32_t triggering_cpu, uint64_t instr_id,
                              long set, const CACHE::BLOCK* current_set,
                              uint64_t ip, uint64_t full_addr, uint32_t type) {
    return static_cast<uint32_t>(::find_victim(rrpv[set]));
}

void hawkeye::replacement_cache_fill(uint32_t triggering_cpu, long set, long way,
                                     uint64_t full_addr, uint64_t ip,
                                     uint64_t victim_addr, uint32_t type) {
    bool is_friendly = predictor.predict(ip);
    Classification cls = is_friendly ? Classification::CACHE_FRIENDLY
                                     : Classification::CACHE_AVERSE;
    ::update_rrpv(rrpv[set], way, cls, false);
}

void hawkeye::update_replacement_state(uint32_t triggering_cpu, long set, long way,
                                       uint64_t full_addr, uint64_t ip,
                                       uint64_t victim_addr, uint32_t type,
                                       uint8_t hit) {
    bool opt_hit = optgen.access(set, full_addr);
    predictor.train(ip, opt_hit);
    bool is_friendly = predictor.predict(ip);
    Classification cls = is_friendly ? Classification::CACHE_FRIENDLY
                                     : Classification::CACHE_AVERSE;
    if (hit) {
        ::update_rrpv(rrpv[set], way, cls, hit);
    }
}