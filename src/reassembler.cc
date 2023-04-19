#include "reassembler.hh"
#include <cstdint>

using namespace std;

void Reassembler::insert(uint64_t first_index, string data,
                         bool is_last_substring, Writer &output) {
     uint64_t cap = output.available_capacity();
     uint64_t max_idx = nxt_idx+cap;
     uint64_t dst_idx = first_index+data.size();
     if(first_index>max_idx) return;
     if(is_last_substring) end_idx=dst_idx>max_idx?max_idx:dst_idx;
     if(first_index==nxt_idx){
         if(cap<data.size()) {
             output.push(data.substr(0,cap));
             output.close();//已经填充完了
         }
         else {
             output.push(data);
             nxt_idx+=data.size();
             backteam(output);
         }
     }
     else if(first_index>nxt_idx){
         if(first_index+data.size()>max_idx){
             store(first_index,data.substr(0,max_idx-first_index));
             end_idx=max_idx;
         }
         else store(first_index,data);
     }
     else{
         if(first_index+data.size()>=max_idx){
             output.push(data.substr(nxt_idx-first_index,cap));
             output.close();
         }
         else{
             output.push(data.substr(nxt_idx-first_index,data.size()-nxt_idx+first_index));
             nxt_idx+=data.size()-nxt_idx+first_index;
             backteam(output);
         }
     }
}

void Reassembler::store(uint64_t idx,std::string str){
    //需要处理以下，避免内部重叠
    for(auto it=data_map.begin();it!=data_map.end();it++){
        //此处处理掉内部的重叠问题
    }
    data_map[idx]=str;
}

void Reassembler::backteam(Writer& out) {
    for(auto it=data_map.begin();it!=data_map.end();it++){
        if(it->first==nxt_idx){

        }
    }
}
uint64_t Reassembler::bytes_pending() const { return {}; }
