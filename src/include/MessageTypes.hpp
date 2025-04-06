#ifndef MESSAGETYPES_HPP
#define MESSAGETYPES_HPP

#include <array>
#include <optional>
#include <string>

namespace messageTypes {
    enum class Type {
        Heartbeat,
        Talk,
        File,
        Chunk,
        End,
        Ack,
        Nack,
        Invalid  // opcional, caso queira indicar erro
    };

    constexpr std::array<std::string_view, 7> typeStrings = {
        "01\n", "02\n", "03\n", "04\n", "05\n", "06\n", "07\n"
    };

    // Mapeia enum -> string
    std::string_view toString(Type type);

    // Mapeia string -> enum (versão com std::optional)
    std::optional<Type> fromString(std::string_view msg);

    // Verifica se é uma mensagem válida
    bool isValidMessageType(std::string_view msg);
}

#endif