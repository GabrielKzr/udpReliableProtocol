#include "../include/MessageTypes.hpp"

std::string_view messageTypes::toString(Type type) {
    switch (type) {
        case Type::Heartbeat: return "01\n";
        case Type::Talk:      return "02\n";
        case Type::File:      return "03\n";
        case Type::Chunk:     return "04\n";
        case Type::End:       return "05\n";
        case Type::Ack:       return "06\n";
        case Type::Nack:      return "07\n";
        default:              return "";
    }
}

std::optional<messageTypes::Type> messageTypes::fromString(std::string_view msg) {
    for (size_t i = 0; i < typeStrings.size(); ++i) {
        if (msg == typeStrings[i]) {
            return static_cast<Type>(i);
        }
    }
    return std::nullopt;
}

bool isValidMessageType(std::string_view msg) {
    return messageTypes::fromString(msg).has_value();
}