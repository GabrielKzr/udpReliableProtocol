#include "../include/Server.hpp"

Server::Server(int port, std::string name) : port(port) {

    // ----------------------------------------------------------------------------------------------------------------

    // * informação importante, '\n' será o divisor do protocolo entre mensagem, nome e pacote (dados)
    // * basicamente o header será:
    //              [    messageType    ][     name     ][                         data                            ]
    //               <------- 3 ------->  <---- 11 ---->  <----------------- tamanho a denfinir ------------------>

    // -----------------------------------------------------------------------------------------------------------------  

    // name terá 10 de tamanho, e no final possuirá um \n 
    if (name.size() < 10) {
        name.resize(10, ' '); 
    }
    name += '\n';

    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket < 0) {
        std::cerr << "Erro ao criar socket!" << std::endl;
        exit(0);
    }
}

bool Server::serverInit() {
    // Configurando o endereço do servidor
    struct sockaddr_in endereco_servidor;
    std::memset(&endereco_servidor, 0, sizeof(endereco_servidor));
    endereco_servidor.sin_family = AF_INET;
    endereco_servidor.sin_addr.s_addr = INADDR_ANY;  // Aceita qualquer endereço
    endereco_servidor.sin_port = htons(this->port);  // Porta de escuta

    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    } 

    // Bind (associar o socket à porta)
    if (bind(this->server_socket, (struct sockaddr*)&endereco_servidor, sizeof(endereco_servidor)) < 0) {
        std::cerr << "Erro ao fazer bind!" << std::endl;
        return false;
    }

    return true;
}

void Server::handleMessage(char* buffer, sockaddr_in* addr) {

}

bool Server::sendKeepAlive() {

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);  // Porta do servidor de escuta
    addr.sin_addr.s_addr = inet_addr("255.255.255.255");

    int bc = 1;
    if (setsockopt(this->server_socket, SOL_SOCKET, SO_BROADCAST, &bc, sizeof(bc)) < 0) {
        std::cerr << "Erro ao configurar o socket para broadcast!" << std::endl;
        return false;
    }

    const char* heartbeat = std::string(std::string(messageTypes::heartbeat) + name).c_str();

    int n = sendto(this->server_socket, heartbeat, sizeof(heartbeat), 0, (struct sockaddr*)&addr, sizeof(addr));
    if (n < 0) {
        std::cerr << "Erro ao enviar pacote!" << std::endl;
        return false;
    }

    bc = 0;
    if (setsockopt(this->server_socket, SOL_SOCKET, SO_BROADCAST, &bc, sizeof(bc)) < 0) {
        std::cerr << "Erro ao configurar o socket para sair de broadcast!" << std::endl;
        return false;
    }

    return true;
}

void Server::serverStart() {

    if(!serverInit()) {
        std::cerr << "Erro ao iniciar servidor" << std::endl;
        exit(0);
    }
    
    fd_set fd;
    struct timeval timeout;
    char buffer[1024];
    struct sockaddr_in clientAddr;
    socklen_t len = sizeof(clientAddr);
    bool closed = false;

    /*
        Se necessário lançar threads, lançar aqui

        // ---------------------------------------




        // ---------------------------------------
    */

    while(!closed) {

        FD_ZERO(&fd);
        FD_SET(server_socket, &fd);  // Adiciona o socket ao conjunto

        timeout.tv_sec = 5;   // Tempo de timeout de 5 segundos
        timeout.tv_usec = 0;   // 0 microsegundos

        int ret = 0;

        while ((ret = select(server_socket + 1, &fd, nullptr, nullptr, &timeout)))
        {
            ssize_t receivedBytes = recvfrom(server_socket, buffer, sizeof(buffer) - 1, 0, (sockaddr*)&clientAddr, &len);

            if (receivedBytes < 0) {
                perror("Erro em recv");  // Veja o motivo com perror ou errno
            }

            buffer[receivedBytes] = '\0';

            this->handleMessage(buffer, &clientAddr);

        }

        if(ret < 0) {
            std::cerr << "Erro ao usar select()" << std::endl;
            exit(0);
        }

        // keep-alive == heartbeat
    
        std::cout << "Enviando Keep Alive \n";
        if(!sendKeepAlive()) {
            closed = true;
        }
    }
}