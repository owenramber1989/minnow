#pragma once

#include "reassembler.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"
#include "wrapping_integers.hh"

class TCPReceiver {
  public:
    /*
     * The TCPReceiver receives TCPSenderMessages, inserting their payload into
     * the Reassembler at the correct stream index.
     */
    void receive(TCPSenderMessage message, Reassembler &reassembler,
                 Writer &inbound_stream);

    /* The TCPReceiver sends TCPReceiverMessages back to the TCPSender. */
    TCPReceiverMessage send(const Writer &inbound_stream) const;

  private:
    std::uint64_t isn = 0;
    bool fin = false;
    Wrap32 zp{};
    bool is_syn_ = false;
};
