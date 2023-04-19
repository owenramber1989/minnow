#include "reassembler.hh"
#include <cstdint>

using namespace std;

void Reassembler::insert(uint64_t first_index, string data,
                         bool is_last_substring, Writer &output) {
    if (first_index > nxt_idx + output.available_capacity())
        return;
    if (is_last_substring) {
        if (i)
    }
}

uint64_t Reassembler::bytes_pending() const { return {}; }
