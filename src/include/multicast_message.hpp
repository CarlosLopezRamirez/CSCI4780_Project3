// File: include/multicast_message.hpp
// Author(s): Caleb Johnson-Cantrell, Carlos López Ramírez, Ojas Nadkarni

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "inet/buffer.hpp"

// Represents the different FTP message types used by this client and server
enum class MulticastMessageType : uint8_t {
    // Invalid message type
    INVALID = 0,

    // ACK OR NACK
    ACKNOWLEDGEMENT,
    NEGATIVE_ACKNOWLEDGEMENT,

    // Request types
    PARTICIPANT_REGISTER,
    PARTICIPANT_DEREGISTER,
    PARTICIPANT_DISCONNECT,
    PARTICIPANT_RECONNECT,
    PARTICIPANT_MSEND,
    PARTICIPANT_QUIT,

    // Multicasted Message
    MULTI_MESSAGE
};

struct MulticastMessageHeader {
    // The type of the message being transmitted
    MulticastMessageType type;

    // Participant ID
    uint16_t pid;

    // The total size of the message being transmitted, including the header
    uint32_t size;

    // Time message arrives at coordinator
    time_t coordinator_time;

    // Constructs a header from the given buffer
    static MulticastMessageHeader from_buffer(Buffer &buffer);

    // Inserts a nicely formatted string version of this header to the inputed `stream`
    friend std::ostream &operator<<(std::ostream &stream, const MulticastMessageHeader &message);
};

class MulticastMessage {
  public:
    // Constructs an FTPMessage
    MulticastMessage(MulticastMessageType type, uint16_t pid, time_t time_sent);

    // Returns the header of this message
    MulticastMessageHeader header();

    // Returns the body of this message
    std::string body();

    // Appends to the body of this message
    //
    // Note: This function will update the size in the header of this message
    friend MulticastMessage &operator<<(MulticastMessage &message, std::string data);

    // Returns a buffer containing a serialized representation of this message
    Buffer to_buffer();

  private:
    // The header that describes this message
    MulticastMessageHeader header_;

    // The body of this message
    std::string body_;
};