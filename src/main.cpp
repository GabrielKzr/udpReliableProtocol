#include "include/Server.hpp"

#define PORT 44444
#define MAX_NAME_LENGTH 20

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <NAME (máx. 20 caracteres)>" << std::endl;
        return 1;
    }

    if (std::strlen(argv[1]) > MAX_NAME_LENGTH) {
        std::cerr << "Erro: o NAME não pode ter mais que " << MAX_NAME_LENGTH << " caracteres." << std::endl;
        return 1;
    }

    const char* name = argv[1];

    Server server(PORT, name);

    server.serverStart();

    return 0;
}