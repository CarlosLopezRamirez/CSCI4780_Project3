// File: multicast_message.cpp
// Author(s): Caleb Johnson-Cantrell, Carlos López Ramírez, Ojas Nadkarni

#include "include/multicast_message.hpp"

#include <cstring>

#include <sstream>
#include <unordered_map>

MulticastMessageHeader MulticastMessageHeader::from_buffer(Buffer &buffer) {
    MulticastMessageHeader result;

    std::memcpy(&result, buffer.data(), sizeof(result));

    return result;
}

std::ostream &operator<<(std::ostream &stream, const MulticastMessageHeader &header) {
    std::unordered_map<MulticastMessageType, std::string> type_map = {
        {MulticastMessageType::INVALID, "INVALID"},
        {MulticastMessageType::NEGATIVE_ACKNOWLEDGEMENT, "NACK"},
        {MulticastMessageType::ACKNOWLEDGEMENT, "ACK"},
        {MulticastMessageType::PARTICIPANT_REGISTER, "REGISTER"},
        {MulticastMessageType::PARTICIPANT_DEREGISTER, "DEREGISTER"},
        {MulticastMessageType::PARTICIPANT_DISCONNECT, "DISCONNECT"},
        {MulticastMessageType::PARTICIPANT_RECONNECT, "RECONNECT"},
        {MulticastMessageType::PARTICIPANT_MSEND, "MSEND"},
        {MulticastMessageType::PARTICIPANT_QUIT, "QUIT"},
        {MulticastMessageType::MULTI_MESSAGE, "MULTICAST MESSAGE"}
    };

    std::stringstream ss;

    ss << "MulticastMessage(";
    ss << "type=" << type_map[header.type];
    ss << ", pid=" << header.pid;
    ss << ", size=" << header.size;
    ss << ")";

    stream << ss.str();

    return stream;
}

MulticastMessage::MulticastMessage(MulticastMessageType type, uint16_t pid, time_t time_sent) {
    header_.type = type;
    header_.pid = pid;
    header_.size = 0;
    header_.coordinator_time = time_sent;
}

MulticastMessageHeader MulticastMessage::header() { return header_; }

std::string MulticastMessage::body() { return body_; }

MulticastMessage &operator<<(MulticastMessage &message, std::string data) {
    message.body_ += data;
    message.header_.size = message.body_.size();

    return message;
}

Buffer MulticastMessage::to_buffer() {
    Buffer result(sizeof(MulticastMessageHeader) + body_.size());

    std::memcpy(result.data(), &header_, sizeof(header_));

    std::memcpy((char *)result.data() + sizeof(header_), body_.data(), body_.size());

    return result;
}