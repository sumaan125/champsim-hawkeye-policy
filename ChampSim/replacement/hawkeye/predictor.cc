#include "predictor.h"

HawkeyePredictor::HawkeyePredictor(std::size_t num_entries_val, int counter_bits){
    this->num_entries = num_entries_val;
    this->maxCounterVal = (1 << counter_bits) - 1;
    this->thresoldCounterVal = (1 << (counter_bits - 1));
    table.assign(num_entries, thresoldCounterVal);
}

std::size_t HawkeyePredictor::hash(uint64_t pc) const{
    return (pc ^ (pc >> 12)) % num_entries;
}

void HawkeyePredictor::train(uint64_t pc, bool opt_hit){
    std::size_t idx = hash(pc);
    if(opt_hit){
        if(table[idx] < maxCounterVal) table[idx]++;
    } else{
        if(table[idx] > 0) table[idx]--;
    }
}

bool HawkeyePredictor::predict(uint64_t pc) const{
    return table[hash(pc)] >= thresoldCounterVal;
}

int HawkeyePredictor::get_counter(uint64_t pc) const{
    return table[hash(pc)];
}