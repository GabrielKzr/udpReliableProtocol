#include "include/Server.hpp"

#define PORT 44444
#define NAME "task1"

int main() {

    Server server(PORT, NAME);

    server.serverStart();

    return 0;
}
