#include "tcp_sender.hh"
#include "buffer.hh"
#include "tcp_config.hh"
#include "tcp_sender_message.hh"
#include "wrapping_integers.hh"

#include <bits/ranges_base.h>
#include <cstddef>
#include <cstdint>
#include <random>

using namespace std;

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender(uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn)
    : isn_(fixed_isn.value_or(Wrap32{random_device()()})),
      initial_RTO_ms_(initial_RTO_ms) {}

uint64_t TCPSender::sequence_numbers_in_flight() const {
    // Your code here.
    return bytes_in_flight_;
}

uint64_t TCPSender::consecutive_retransmissions() const {
    // Your code here.
    return consecutive_retransmissions_;
}

optional<TCPSenderMessage> TCPSender::maybe_send() {
    // Your code here.
    if (_messages.empty()) {
        return {};
    }
    if (consecutive_retransmissions_ && timer_.in_run() &&
        timer_.tick_now() > 0) {
        return {};
    }
    TCPSenderMessage msg = _messages.front();
    _messages.pop();

    return msg;
}

void TCPSender::push(Reader &outbound_stream) {
    // Your code here.
    if (fin_sent_) {
        return;
    }
    uint64_t space = rcvno + (window_size_ ? window_size_ : 1) - nxt_seqno_;
    while (space > 0 && !fin_sent_) {
        TCPSenderMessage msg;
        if (!nxt_seqno_) {
            msg.SYN = true;
            syn_sent_ = true;
            --space;
        }
        msg.seqno = Wrap32::wrap(nxt_seqno_, isn_);
        Buffer &buffer = msg.payload;
        read(outbound_stream, min(space, TCPConfig::MAX_PAYLOAD_SIZE), buffer);
        space -= buffer.size();
        if (outbound_stream.is_finished() && space > 0) {
            msg.FIN = true;
            fin_sent_ = true;
            --space;
        }
        uint64_t len = msg.sequence_length();
        if (!len) {
            return;
        }
        _messages.emplace(msg);
        if (!timer_.in_run()) {
            timer_.start();
        }
        _outstanding_messages.emplace(msg);
        nxt_seqno_ += len;
        bytes_in_flight_ += len;
    }
}

TCPSenderMessage TCPSender::send_empty_message() const {
    // Your code here.
    TCPSenderMessage msg;
    msg.seqno = Wrap32::wrap(nxt_seqno_, isn_);
    return msg;
}

void TCPSender::receive(const TCPReceiverMessage &msg) {
    // Your code here.
    if (msg.ackno) {
        rcvno = msg.ackno->unwrap(isn_, nxt_seqno_);
    }
    if (rcvno > nxt_seqno_) {
        return;
    }
    window_size_ = msg.window_size;
    bool new_check_ = false;
    while (!_outstanding_messages.empty()) {
        TCPSenderMessage s_msg = _outstanding_messages.front();
        uint64_t len = s_msg.sequence_length();
        uint64_t seqno = s_msg.seqno.unwrap(isn_, nxt_seqno_);
        if (seqno + len > rcvno) {
            break;
        }
        _outstanding_messages.pop();
        bytes_in_flight_ -= len;
        new_check_ = true;
    }
    if (new_check_) {
        retransmission_timeout_ = initial_RTO_ms_;
        if (!_outstanding_messages.empty()) {
            timer_.start();
        } else {
            timer_.stop();
        }
        consecutive_retransmissions_ = 0;
    }
}

void TCPSender::tick(const size_t ms_since_last_tick) {
    // Your code here.
    if (!timer_.expired(ms_since_last_tick, retransmission_timeout_) ||
        _outstanding_messages.empty()) {
        return;
    }
    _messages.push(_outstanding_messages.front());
    if (window_size_ != 0) {
        ++consecutive_retransmissions_;
        retransmission_timeout_ <<= 1;
    }
    maybe_send();
    timer_.start();
}
