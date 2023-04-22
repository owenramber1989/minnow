#include "tcp_receiver.hh"
#include "tcp_receiver_message.hh"
#include "wrapping_integers.hh"
#include <cstdint>

using namespace std;

void TCPReceiver::receive(TCPSenderMessage message, Reassembler &reassembler,
                          Writer &inbound_stream) {
    // Your code here.
    if (!is_syn_) {
        if (!message.SYN) {
            return;
        }
        is_syn_ = true;
        zp = message.seqno;
    }
    uint64_t cp = inbound_stream.writer().bytes_pushed() + 1;
    uint64_t idx = Wrap32(message.seqno).unwrap(zp, cp);
    uint64_t fdx = idx + message.SYN - 1;
    reassembler.insert(fdx, message.payload, message.FIN, inbound_stream);
}

TCPReceiverMessage TCPReceiver::send(const Writer &inbound_stream) const {
    // Your code here.
    TCPReceiverMessage msg;
    if (!is_syn_) {
        msg.ackno = nullopt;
    } else {
        uint64_t cp = inbound_stream.writer().bytes_pushed() + 1;
        if (inbound_stream.writer().is_closed()) {
            ++cp;
        }
        msg.ackno = zp + cp;
    }
    uint64_t cap = inbound_stream.available_capacity();
    msg.window_size = cap > 65535 ? 65535 : cap;
    return msg;
}
