#include "../include/Server.hpp"

/*
ClientInfo::ClientInfo(std::string ip, int counter, int id) : ip(ip), counter(counter), id(id) {
    
}
*/

Server::Server(int port, std::string name) : port(port) {

    // ----------------------------------------------------------------------------------------------------------------

    // * informação importante, '\n' será o divisor do protocolo entre mensagem, nome e pacote (dados)
    // * basicamente o header será: (para o caso de heartbeat)
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

bool Server::sendKeepAlive() {
    std::lock_guard<std::mutex> lock(sendMutex);  // protege alterações no socket

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

    const char* heartbeat = std::string(std::string(messageTypes::toString(messageTypes::Type::Heartbeat)) + name).c_str();

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

void Server::handleMessage(char* buffer, sockaddr_in* addr) {
    std::string ip = inet_ntoa(addr->sin_addr);
    std::string buff(buffer);
    std::string_view message = buff.substr(0, 3);  // mantém como string_view

    auto typeOpt = messageTypes::fromString(message);
    if (!typeOpt.has_value()) {
        std::cout << "Mensagem inválida\n";
        return;
    }

    auto type = typeOpt.value();

    switch (type) {
        case messageTypes::Type::Heartbeat: {
            if (buff.size() < 17) { // 3 + 14
                std::cout << "Heartbeat inválido\n";
                break;
            }
            std::string name = buff.substr(3, 14);
            clients.insert_or_assign(name, std::make_pair(ip, 0));
            break;
        }

        case messageTypes::Type::Talk: {
            if (buff.size() < 7) {
                std::cout << "Talk inválido\n";
                break;
            }

            std::string s_id = buff.substr(3, 4); // 4 caracteres do ID
            std::string data = buff.substr(7);

            std::cout << "Received TALK Data from " << ip << ": " << data << std::endl;

            std::string msg = std::string(messageTypes::toString(messageTypes::Type::Ack)) + s_id;

            addr->sin_port = htons(this->port);
            sendto(server_socket, msg.c_str(), msg.size(), 0, (sockaddr*)&addr, sizeof(*addr));
            break;
        }

        case messageTypes::Type::File: {
            // implementar
            break;
        }

        case messageTypes::Type::Chunk: {
            // implementar
            break;
        }

        case messageTypes::Type::End: {
            // implementar
            break;
        }

        case messageTypes::Type::Ack: {
            // lógica de ACK (futura)
            break;
        }

        case messageTypes::Type::Nack: {
            // lógica de NACK (futura)
            break;
        }

        default:
            std::cout << "Tipo de mensagem desconhecido\n";
            break;
    }
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

    // Se necessário lançar threads, lançar aqui

    std::atomic<bool> running = true;

    // ---------------------------------------

    std::thread keepAliveThread([this, &running]() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            std::cout << "Enviando Keep Alive\n";
            if (!this->sendKeepAlive()) {
                std::cerr << "Erro ao enviar keep alive\n";
                // você pode optar por alterar running = false aqui
            }
        }
    });


    // ---------------------------------------

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
        
        // essa lógica de keep-alive não funciona, vai precisar existir uma thread pra isso
        std::cout << "Enviando Keep Alive \n";
        if(!sendKeepAlive()) {
            closed = true;
        }
    }

    running = false;
    if (keepAliveThread.joinable()) {
        keepAliveThread.join();
    }
}