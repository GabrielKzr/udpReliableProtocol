#include "../include/Server.hpp"

/*
ClientInfo::ClientInfo(std::string ip, int counter, int id) : ip(ip), counter(counter), id(id) {
    
}
*/

Server::Server(int port, std::string name) : port(port) {

    // ----------------------------------------------------------------------------------------------------------------

    // o header é outro, eu só n refiz por preguiça

    // * informação importante, '\n' será o divisor do protocolo entre mensagem, nome e pacote (dados)
    // * basicamente o header será: (para o caso de heartbeat)
    //              [    messageType    ][     name     ][                         data                            ]
    //               <------- 3 ------->  <---- 11 ---->  <----------------- tamanho a denfinir ------------------>

    // -----------------------------------------------------------------------------------------------------------------  

    this->name = name;
    
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket < 0) {
        std::cerr << "Erro ao criar socket!" << std::endl;
        exit(0);
    }

    packetManager = new PacketManager(this->port, this->name);

    clock = new Clock(clients);
}

std::string Server::getLocalIp() {
    struct ifaddrs *ifaddr, *ifa;
    char buf[INET_ADDRSTRLEN];
    std::string ip;

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return "";
    }

    for (ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;
        if (ifa->ifa_addr->sa_family == AF_INET) {
            void* addr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, addr, buf, INET_ADDRSTRLEN);
            // ignora loopback
            if (std::string(buf) != "127.0.0.1") {
                ip = buf;
                break;
            }
        }
    }
    freeifaddrs(ifaddr);
    return ip;
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

    localIp = getLocalIp();

    return true;
}

bool Server::sendKeepAlive() {
    std::lock_guard<std::mutex> lock(sendMutex);  // protege alterações no socket

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(this->port);  // Porta do servidor de escuta
    addr.sin_addr.s_addr = inet_addr("255.255.255.255");
    
    int bc = 1;
    if (setsockopt(this->server_socket, SOL_SOCKET, SO_BROADCAST, &bc, sizeof(bc)) < 0) {
        std::cerr << "Erro ao configurar o socket para broadcast!" << std::endl;
        return false;
    }
    
    uint8_t id[4] = {0};
    uint8_t hash[16] = {0};
    uint8_t payload[1430] = {0};
    // Enviando o pacote de heartbeat
    Message_t heartbeat(0x0001, id, this->name.c_str(), 0, 0, hash, 0, payload, localIp.c_str(), 0);

    int n = sendto(this->server_socket, &heartbeat, sizeof(heartbeat), 0, (struct sockaddr*)&addr, sizeof(addr));
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

void Server::handleMessage(Message_t* message, sockaddr_in* addr, int receivedBytes) {

    switch (message->type) {

        case 0x0001: {
            
            clientInfo client = {message->ip, 0}; // Inicializa o clientInfo com o IP e tempo 0

            clock->HandleNewClient(message->name, client); // Adiciona o cliente ao relógio
            // std::cout << "Heartbeat recebido de " << ip << ": " << message->payload << std::endl;
            break;
        }
        
        case 0x0002: {
            std::cout << "Recebi mensagem talk de " << message->ip << ": " << message->payload << std::endl;

            if(!clock->containsClient(message->ip)) {
                
                std::cout << "Cliente não encontrado.\n";

                auto packet = packetManager->buildNackMessage(message->id, 0x01, localIp);

                packetManager->sendMessageWithoutAck(packet, message->ip, server_socket, sendMutex);

                break;
            }

            // std::cout << "Verificando se o cliente está no relógio...\n";

            auto packet = packetManager->buildAckMessage(message->id, localIp);

            packetManager->sendMessageWithoutAck(packet, message->ip, server_socket, sendMutex);            

            break;
        }
        
        case 0x0003 ... 0x0005: {

            if(!this->clock->addClientFile(*message)) {
                
                std::cout << "Cliente não encontrado.\n";

                auto packet = packetManager->buildNackMessage(message->id, 0x02, localIp);

                packetManager->sendMessageWithoutAck(packet, message->ip, server_socket, sendMutex);

                break;

            } 

            auto packet = packetManager->buildAckMessage(message->id, localIp);

            packetManager->sendMessageWithoutAck(packet, message->ip, server_socket, sendMutex);   

            break;

        }

        case 0x0006: {

            std::cout << "Recebi ACK de " << message->ip << ": " << message->payload << std::endl;
            
            // o correto pra não acontecer nenhum tipo de falha de segurança é fazer a 
            // verificação do cliente que enviou o ack, então teria q ser alterado em packetManager, e a Message_t seria passada por parâmetro
            packetManager->verifyAck(message->id);

            break;
        } 
        
        case 0x0007: {
            std::cout << "Recebi NACK de " << message->ip << ": " << message->payload << std::endl;

            packetManager->handleNack(message->reason);
            // implementar
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
    

    // Se necessário lançar threads, lançar aqui

    std::atomic<bool> running = true;

    // ---------------------------------------

    std::thread keepAliveThread([this, &running]() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            // std::cout << "Enviando Keep Alive\n";
            if (!this->sendKeepAlive()) {
                std::cerr << "Erro ao enviar keep alive\n";
                running = false; // Para o loop se houver erro
            }
        }
    });

    this->clock->Start(); // Inicia o relógio

    // ---------------------------------------

    std::thread mainThread([this, &running]() {

        fd_set fd;
        struct timeval timeout;
        Message_t message;
        struct sockaddr_in clientAddr;
        socklen_t len = sizeof(clientAddr);


        while (running) {
            FD_ZERO(&fd);
            FD_SET(server_socket, &fd);  // Adiciona o socket ao conjunto

            timeout.tv_sec = 5;   // Tempo de timeout de 5 segundos
            timeout.tv_usec = 0;   // 0 microsegundos

            int ret = 0;

            while ((ret = select(server_socket + 1, &fd, nullptr, nullptr, &timeout)))
            {
                ssize_t receivedBytes = recvfrom(server_socket, &message, sizeof(message) - 1, 0, (sockaddr*)&clientAddr, &len);

                std::string srcIp = inet_ntoa(clientAddr.sin_addr);
                if (srcIp == localIp) {
                    // é o meu próprio broadcast — ignora
                    continue;
                }        

                if (receivedBytes < 0) {
                    perror("Erro em recv");  // Veja o motivo com perror ou errno
                } 
                else if (receivedBytes == 0) {
                    std::cout << "Conexão fechada pelo cliente." << std::endl;
                    running = false;
                } 
                else {
                    // Processa a mensagem recebida
                    this->handleMessage(&message, &clientAddr, receivedBytes);
                }

            
            }

            if(ret < 0) {
                std::cerr << "Erro ao usar select()" << std::endl;
                exit(0);
            }
        }
    });

    while (true)
    {       
        console.cleanConsole();
        console.printMenu();
        auto response = console.handleInput();

        
        if (response.first == "talk") {
            
            auto packet = packetManager->buildTalkMessage(response.second.second, localIp);
            
            auto client = this->clock->getClientInfo(response.second.first);

            if(client == nullptr) {
                std::cout << "Cliente não encontrado.\n";
                std::this_thread::sleep_for(std::chrono::seconds(1));
                delete client; 
                continue;
            }

            std::cout << "Enviando mensagem para " << client->ip << ": " << response.second.second << std::endl;
            packetManager->sendMessage(packet, client->ip, server_socket, sendMutex);

            std::this_thread::sleep_for(std::chrono::seconds(1));

            delete client; // Libera a memória alocada para clientInfo
        } else if (response.first == "file") {



            auto client = this->clock->getClientInfo(response.second.first);

            /*
            if(client == nullptr) {
                std::cout << "Cliente não encontrado.\n";
                std::this_thread::sleep_for(std::chrono::seconds(1));
                delete client; 
                continue;
            }
            */

            std::string fileContent = readFileToString(response.second.second);

            if (fileContent.empty()) {
                std::cerr << "Erro ao ler o arquivo ou arquivo indisponível." << std::endl;
                delete client;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }

            std::cout << "ajustando pacotes... (vai dar seg fault, certeza)\n";
            std::vector<Message_t> packets = packetManager->buildFileMessage(fileContent, localIp);

            if (packets.size() == 0) {
                std::cerr << "Erro ao construir pacotes de arquivo." << std::endl;
                delete client;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }

            if(packets.size() != ceil((float)fileContent.length() / (float)Message_t::MAX_PAYLOAD_SIZE)) {
                std::cerr << "Erro ao construir pacotes de arquivo." << std::endl;
                delete client;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }

            std::cout << "Pacotes construídos com sucesso. Enviando...\n";
            for(size_t i = 0; i < packets.size(); ++i) {

                packetManager->sendMessage(packets[i], client->ip, server_socket, sendMutex);
            }

            delete client; 

        } else if(response.first == "exit") {
            std::cout << "Saindo do programa...\n";
            break;
        } else {
            std::cout << "Comando inválido. Tente novamente.\n";
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        // Aqui você pode adicionar lógica para lidar com a entrada do usuário
        // e enviar mensagens para os clientes, se necessário.
    }

    std::cout << "Encerrando...\n";

    running = false;
    if (keepAliveThread.joinable()) {
        keepAliveThread.join();
    }
    if (mainThread.joinable()) {
        mainThread.join();
    }
    this->serverClose();
}

void Server::serverClose() {
    clock->Stop(); // Para o relógio
    close(server_socket);
    std::cout << "Servidor fechado." << std::endl;
}

Server::~Server() {
    close(server_socket);
    delete clock;
    delete packetManager;
    std::cout << "Servidor fechado." << std::endl;
}