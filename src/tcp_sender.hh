#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"
#include <cstdint>

class Timer {
  private:
    uint64_t ticks_ = 0;
    bool running = false;

  public:
    bool expired(uint64_t ms_since_last_tick, uint64_t time_out_) {
        ticks_ += ms_since_last_tick;
        return running && (ticks_ >= time_out_);
    }
    uint64_t tick_now() { return ticks_; }
    bool in_run() { return running; }
    void start() {
        ticks_ = 0;
        running = true;
    }
    void stop() { running = false; }
};
class TCPSender {
    Wrap32 isn_;
    uint64_t initial_RTO_ms_;
    uint64_t consecutive_retransmissions_ = 0;
    uint64_t rcvno = 0;
    uint64_t nxt_seqno_ = 0;
    uint64_t window_size_ = 1;
    uint64_t retransmission_timeout_ = initial_RTO_ms_;
    uint64_t bytes_in_flight_ = 0;
    bool syn_sent_ = false;
    bool fin_sent_ = false;
    Timer timer_{};
    std::queue<TCPSenderMessage> _messages{};
    std::queue<TCPSenderMessage> _outstanding_messages{};

  public:
    /* Construct TCP sender with given default Retransmission Timeout and
     * possible ISN */
    TCPSender(uint64_t initial_RTO_ms, std::optional<Wrap32> fixed_isn);

    /* Push bytes from the outbound stream */
    void push(Reader &outbound_stream);

    /* Send a TCPSenderMessage if needed (or empty optional otherwise) */
    std::optional<TCPSenderMessage> maybe_send();

    /* Generate an empty TCPSenderMessage */
    TCPSenderMessage send_empty_message() const;

    /* Receive an act on a TCPReceiverMessage from the peer's receiver */
    void receive(const TCPReceiverMessage &msg);

    /* Time has passed by the given # of milliseconds since the last time the
     * tick() method was called. */
    void tick(uint64_t ms_since_last_tick);

    /* Accessors for use in testing */
    uint64_t sequence_numbers_in_flight()
        const; // How many sequence numbers are outstanding?
    uint64_t consecutive_retransmissions()
        const; // How many consecutive *re*transmissions have happened?
};
