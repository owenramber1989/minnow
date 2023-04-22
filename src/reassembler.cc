#include "reassembler.hh"
#include <cstdint>

using namespace std;

void Reassembler::insert(uint64_t first_index, string data,
                         bool is_last_substring, Writer &output) {
    /*
      if (data.empty()) {
          if (is_last_substring) {
              output.close();
              return;
          }
          return;
      }
      */
    // EOF
    if (is_last_substring) {
        end_checked = true;
        // end_idx = first_index + data.length();
        eof_idx = first_index + data.size();
    }
    preprocess(first_index,data,output);
    while (!buf.empty() && buf.begin()->first == nxt_idx) {
        const auto &iter = buf.begin();
        output.push(iter->second);
        unassembled_bytes -= iter->second.size();
        buf.erase(iter);
    }

    if (end_checked && output.writer().bytes_pushed() == eof_idx) {
        output.close();
    }
}
void Reassembler::preprocess(uint64_t &first_index,std::string& data,Writer& output){
    uint64_t cap = output.available_capacity();
    nxt_idx = output.writer().bytes_pushed();
    uint64_t max_idx = output.reader().bytes_popped() + nxt_idx + cap;
    /*
     * cap 当前剩余容量
     * dst_idx 如果可以正常插入，对应的下标
     * max_idx
     * 不可以写入数据的最小下标，接收到最后一条子串时更新，如果当前子串超出了max_idx，截断子串并更新max_idx
     * nxt_idx 如果first_index是nxt_idx,那么就可以直接插入，每次写入Writer
     * stream的时候更新
     *         而且更新nxt_idx时都需要检验之前暂存起来的子串是否可以接入Writer
     * stream了 max_idx和nxt_idx都是尚未被数据填充的
     */
    if (nxt_idx == max_idx) {
        return;
    }
    if (first_index >= max_idx || first_index + data.size() < nxt_idx) {
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
/*
 * store最大的问题是如何高效存储，方便后续归队
 * 它需要能分辨下面这几种情况
 *      1. 插入的子串位于已经暂存起来的子串内部，则直接丢弃
 *      2.
 * 插入的子串与已经暂存起来的子串有所重叠，则截断重叠区域后插入，并且更新有效索引范围
 * 因此我们需要用一个额外的数据结构来记录有效索引范围，用一个定长数组就够了
 * 用0表示该位既不是某个子串的起点亦非终点，1表示起点，2表示终点
 * 当我们插入子串时，如果当前位置是0，则首先向前查找最近的非0位
 *      1.
 * 如果是2，说明与上一个子串没有任何重叠，将当前子串的first_index标记为1,
 *          从这个1开始向后寻找最近的1，如果到了子串末尾都还没找着，则将当前子串的末尾标记为2，将子串插入对应的vector
 *                  这是最好处理的情况，因为完全没有重叠
 *          如果在子串末尾前找着了，则将找着的1标记为0，截断当前子串，然后插入到对应的vector
 *      2.
 * 如果是1，则从first_index开始向后查找2，如果查到当前子串末尾都还没查到，直接丢掉子串
 *          如果在子串末尾之前找到了2，再向后查找最近的1
 *          如果到了当前子串的末尾都没查到，则将当前的末尾标记为2，把刚刚找到的2标记为0
 *          其实就是看当前子串的末尾所处区域的模式
 *          如果结束在一个 200(n个0)1 的范围内，更新2的位置
 *
 *
 *
 * 如果是1，则从first_index开始查找最近的2，若超过了
 */
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
