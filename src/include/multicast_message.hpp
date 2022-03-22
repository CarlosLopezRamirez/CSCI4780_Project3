// File: include/ftp_message.hpp
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
    ACKNOWLEDGEMENT,
    DATA_NORM,
    DATA_LAST,

    // Response types
    RESPONSE_GOOD,  // Indicates successful execution of command
    RESPONSE_BAD,   // Indicates failed execution of command

    // Request types
    REQUEST_CD,
    REQUEST_DELETE,
    REQUEST_GET,
    REQUEST_GET_ASYNC,
    REQUEST_LS,
    REQUEST_MKDIR,
    REQUEST_PUT,
    REQUEST_PUT_ASYNC,
    REQUEST_PWD,
    REQUEST_QUIT,
    REQUEST_TERMINATE
};

struct MulticastMessageHeader {
    // The type of the message being transmitted
    MulticastMessageType type;

    // Used to identify a command
    uint16_t id;

    // The total size of the message being transmitted, including the header
    uint32_t size;

    // Constructs a header from the given buffer
    static MulticastMessageHeader from_buffer(Buffer &buffer);

    // Inserts a nicely formatted string version of this header to the inputed `stream`
    friend std::ostream &operator<<(std::ostream &stream, const MulticastMessageHeader &message);
};

class MulticastMessage {
  public:
    // Constructs an FTPMessage
    MulticastMessage();

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