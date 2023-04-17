#include <stdexcept>
#include <string_view>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream(uint64_t capacity) : capacity_(capacity) {}

void Writer::push(string data) {
    // Your code here.
    size_t size = min(data.length(), capacity_ - buf_.size());
    buf_.append(data.substr(0, size));
    tot_write_ += size;
}

void Writer::close() {
    // Your code here.
    is_end_ = true;
}

void Writer::set_error() {
    // Your code here.
    is_err_ = true;
}

bool Writer::is_closed() const {
    // Your code here.
    return is_end_;
}

uint64_t Writer::available_capacity() const {
    // Your code here.
    return capacity_ - buf_.size();
}

uint64_t Writer::bytes_pushed() const {
    // Your code here.
    return tot_write_;
}

string_view Reader::peek() const {
    // Your code here.
    return std::string_view(buf_.begin(), buf_.begin() + 1);
}

bool Reader::is_finished() const {
    // Your code here.
    return buf_.empty() && is_end_;
}

bool Reader::has_error() const {
    // Your code here.
    return is_err_;
}

void Reader::pop(uint64_t len) {
    // Your code here.
    const std::size_t size = min(len, buf_.size());
    buf_.erase(0, size);
    tot_read_ += size;
}

uint64_t Reader::bytes_buffered() const {
    // Your code here.
    return buf_.size();
}

uint64_t Reader::bytes_popped() const {
    // Your code here.
    return tot_read_;
}
