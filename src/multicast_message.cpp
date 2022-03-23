// File: include/ftp_message.hpp
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
        {MulticastMessageType::ACKNOWLEDGEMENT, "ACKNOWLEDGEMENT"},
        {MulticastMessageType::PARTICIPANT_REGISTER, "REGISTER"},
        {MulticastMessageType::PARTICIPANT_DEREGISTER, "DEREGISTER"},
        {MulticastMessageType::PARTICIPANT_DISCONNECT, "DISCONNECT"},
        {MulticastMessageType::PARTICIPANT_RECONNECT, "RECONNECT"},
        {MulticastMessageType::PARTICIPANT_MSEND, "MSEND"},
    };

    std::stringstream ss;

    ss << "FTPMessage(";
    ss << "type=" << type_map[header.type];
    ss << ", pid=" << header.pid;
    ss << ", size=" << header.size;
    ss << ")";

    stream << ss.str();

    return stream;
}

MulticastMessage::MulticastMessage() {

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