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
        {MulticastMessageType::DATA_NORM, "DATA_NORM"},
        {MulticastMessageType::DATA_LAST, "DATA_LAST"},
        {MulticastMessageType::ACKNOWLEDGEMENT, "ACKNOWLEDGEMENT"},
        {MulticastMessageType::RESPONSE_GOOD, "RESPONSE_GOOD"},
        {MulticastMessageType::RESPONSE_BAD, "RESPONSE_BAD"},
        {MulticastMessageType::REQUEST_CD, "REQUEST_CD"},
        {MulticastMessageType::REQUEST_DELETE, "REQUEST_DELETE"},
        {MulticastMessageType::REQUEST_GET, "REQUEST_GET"},
        {MulticastMessageType::REQUEST_GET_ASYNC, "REQUEST_GET_ASYNC"},
        {MulticastMessageType::REQUEST_LS, "REQUEST_LS"},
        {MulticastMessageType::REQUEST_MKDIR, "REQUEST_MKDIR"},
        {MulticastMessageType::REQUEST_PUT, "REQUEST_PUT"},
        {MulticastMessageType::REQUEST_PUT_ASYNC, "REQUEST_PUT_ASYNC"},
        {MulticastMessageType::REQUEST_PWD, "REQUEST_PWD"},
        {MulticastMessageType::REQUEST_QUIT, "REQUEST_QUIT"},
        {MulticastMessageType::REQUEST_TERMINATE, "REQUEST_TERMINATE"},
    };

    std::stringstream ss;

    ss << "FTPMessage(";
    ss << "type=" << type_map[header.type];
    ss << ", id=" << header.id;
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