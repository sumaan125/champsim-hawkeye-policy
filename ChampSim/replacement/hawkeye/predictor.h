#ifndef predictor_h
#define predictor_h
#include<vector>
#include<cstdint>
#include<cstddef>


class HawkeyePredictor {
private:
    std::size_t num_entries;
    int maxCounterVal;
    int thresoldCounterVal;
    std::vector<int> table;
public:
    HawkeyePredictor(std::size_t num_entries = 8192, int counter_bits = 3);
    void train(uint64_t pc, bool opt_hit);
    bool predict(uint64_t pc) const;
    int get_counter(uint64_t pc) const;
    std::size_t hash(uint64_t pc) const;
};
#endif