#include "reassembler.hh"
#include <cstdint>

using namespace std;

void Reassembler::insert(uint64_t first_index, string data,
                         bool is_last_substring, Writer &output) {
    if (!data.empty()) {
        preprocess(first_index, data, output);
    }
    while (!buf.empty() && buf.begin()->first == nxt_idx) {
        const auto &iter = buf.begin();
        output.push(iter->second);
        unassembled_bytes -= iter->second.size();
        buf.erase(iter);
    }
    // EOF
    if (is_last_substring) {
        end_checked = true;
        eof_idx = first_index + data.size();
    }
    if (end_checked && output.writer().bytes_pushed() == eof_idx) {
        output.close();
    }
}
void Reassembler::preprocess(uint64_t &first_index, std::string &data,
                             Writer &output) {
    uint64_t cap = output.available_capacity();
    nxt_idx = output.writer().bytes_pushed();
    uint64_t max_idx =
        output.reader().bytes_popped() + output.reader().bytes_buffered() + cap;
    if (nxt_idx == max_idx) {
        output.close();
        return;
    }
    if (first_index >= max_idx || first_index + data.size() - 1 < nxt_idx) {
        return;
    }

    // 需要去尾的情况
    if (first_index + data.size() > max_idx) {
        data = data.substr(0, max_idx - first_index);
    }
    // 需要去头的情况
    if (first_index < nxt_idx && first_index + data.length() > nxt_idx) {
        data = data.substr(nxt_idx - first_index);
        first_index = nxt_idx;
    }

    if (buf.empty()) {
        unassembled_bytes += data.size();
        buf.insert({first_index, data});
        return;
    }

    store(first_index, data);
}

void Reassembler::store(uint64_t idx, std::string str) {
    // 需要处理一下，避免内部重叠
    for (auto iter = buf.begin(); iter != buf.end();) {
        size_t str_end = idx + str.size() - 1;
        size_t ite_end = iter->first + iter->second.size() - 1;

        if ((idx >= iter->first && idx <= ite_end) ||
            (iter->first >= idx && iter->first <= str_end) ||
            (str_end + 1 == iter->first)) {
            merge(idx, str, iter->first,
                  iter->second); // 先消除重叠：把传入的 seg
                                 // 与已缓存并存在重叠的 segment 合并,
            unassembled_bytes -= iter->second.size();
            buf.erase(iter++); // 合并后先删除已存在的 str
        } else {
            ++iter;
        }
    }
    unassembled_bytes += str.size();
    buf.insert({idx, str}); // 把处理完重叠并合并后的 seg 插入缓冲区
}

void Reassembler::merge(uint64_t &idx, string &_str, uint64_t bdx,
                        string _buf) {
    size_t seg_tail = idx + _str.length() - 1;
    size_t cache_tail = bdx + _buf.length() - 1;
    if (seg_tail + 1 == 1) {
        _str = _str + _buf;
        return;
    }
    // 情况一：_str在前_buf在后
    if (idx < bdx && seg_tail <= cache_tail) {
        _str = _str.substr(0, bdx - idx) + _buf;
        return;
    }
    // 情况二：_buf在前_str在后
    else if (idx >= bdx && seg_tail > cache_tail) {
        _str = _buf + _str.substr(bdx + _buf.length() - idx);
        idx = bdx;
        return;
    }
    // 情况三: _str被_buf覆盖
    else if (idx >= bdx && seg_tail <= cache_tail) {
        _str = _buf;
        idx = bdx;
        return;
    }
}
uint64_t Reassembler::bytes_pending() const { return unassembled_bytes; }
