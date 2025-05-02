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
    uint8_t payload[1410] = {0};
    char fileName[20] = {0}; // nome do arquivo
    // Enviando o pacote de heartbeat
    Message_t heartbeat(0x0001, id, this->name.c_str(), 0, 0, hash, 0, payload, localIp.c_str(), 0, fileName);

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
            
            clientInfo client = {message->ip, 0, ntohs(addr->sin_port)}; // Converte para int}; // Inicializa o clientInfo com o IP e tempo 0

            clock->HandleNewClient(message->name, client); // Adiciona o cliente ao relógio
            // std::cout << "Heartbeat recebido de " << ip << ": " << message->payload << std::endl;
            break;
        }
        
        case 0x0002: {

            // verifica se a mensagem já foi recebida, se não, pode printar, se sim, ignora e envia um ack
            if(this->clock->HandleTalkMessage(*message)) {
                std::cout << "Recebi mensagem talk de " << message->ip << ": " << message->payload << std::endl;
                std::cout << "Id da mensagem: " << uint8_to_int(message->id) << std::endl;
            }

            /* // a princípio não precisa disso, mas se quiser pode fazer (mandar NACK se dispositivo não existe na lista de clientes) 
            if(!clock->containsClient(message->ip)) {
                
                std::cout << "Cliente não encontrado.\n";

                auto packet = packetManager->buildNackMessage(message->id, 0x01, localIp);

                packetManager->sendMessageWithoutAck(packet, message->ip, server_socket, sendMutex);

                break;
            }
            */

            auto packet = packetManager->buildAckMessage(message->id, localIp);

            packetManager->sendMessageWithoutAck(packet, message->ip, server_socket, sendMutex);            

            break;
        }
        
        case 0x0003 ... 0x0005: {

            int response = this->clock->addClientFile(*message);

            if(response == 0) {
                
                std::cout << "Pacote duplicado.\n";
                // não faz nada, só diz q já recebeu com um ack e é isso

            } else if(response == -1) {

                std::cout << "Pacote corrompido\n";

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


            do {

                auto client = this->clock->getClientInfo(response.second.first);
                
                /*
                if(client == nullptr) {
                    std::cout << "Cliente não encontrado.\n";
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    delete client; 
                    continue;
                }
                */

                constexpr size_t BLOCK_SIZE = Message_t::MAX_PAYLOAD_SIZE;
                
                FILE* file = std::fopen(response.second.second.c_str(), "rb");
                
                if (!file) {
                    std::cerr << "Erro ao abrir o arquivo." << std::endl;
                    delete client;
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    delete client;
                    continue;
                }
                
                std::string fileName = response.second.second.substr(response.second.second.find_last_of("/\\") + 1);
                std::cout << "ajustando pacotes com leitura em blocos...\n";
                
                // Inicializa contexto MD5
                MD5_CTX ctx;
                MD5_Init(&ctx);
                
                std::vector<uint8_t> digest(MD5_DIGEST_LENGTH);
                
                uint8_t totalChunks = 0;
                
                // Pré-contagem dos chunks (pode ser evitado se for tolerável ajustar o protocolo para stream indefinida)
                std::fseek(file, 0, SEEK_END);
                long fileSize = std::ftell(file);
                std::rewind(file);
                
                totalChunks = std::ceil((float)fileSize / (float)BLOCK_SIZE);
                
                if (totalChunks < 1) {
                    std::cerr << "Tamanho inválido para mensagem do tipo File." << std::endl;
                    std::fclose(file);
                    delete client;
                    continue;
                }
                
                char buffer[BLOCK_SIZE];
                size_t bytesRead;
                uint8_t index = 0;
                
                while ((bytesRead = std::fread(buffer, 1, BLOCK_SIZE, file)) > 0) {
                    MD5_Update(&ctx, buffer, bytesRead);
                    
                    std::string data(buffer, bytesRead);
                    
                    if (index == 0 && totalChunks == 1) {
                        // Apenas um pacote
                        MD5_Final(digest.data(), &ctx);
                        auto packet = packetManager->buildFileStartMessage(totalChunks, data, localIp, &digest, fileName);
                        packetManager->sendMessage(packet, client->ip, server_socket, sendMutex);
                        break;
                    } else if (index == 0) {
                        // Início
                        auto packet = packetManager->buildFileStartMessage(totalChunks, data, localIp, nullptr, fileName);
                        packetManager->sendMessage(packet, client->ip, server_socket, sendMutex);
                    } else if (index == totalChunks - 1) {
                        break;
                    } else {
                        // Chunk intermediário
                        auto packet = packetManager->buildFileChunkMessage(totalChunks, index, data, localIp, nullptr);
                        packetManager->sendMessage(packet, client->ip, server_socket, sendMutex);
                    }
                    
                    index++;
                }
                
                // Envia último pacote com digest real
                if (totalChunks > 1) {
                    // Finaliza digest
                    MD5_Final(digest.data(), &ctx);
                    // O buildFileEndMessage pode ser recriado aqui com digest:
                    std::fseek(file, (totalChunks - 1) * BLOCK_SIZE, SEEK_SET);
                    bytesRead = std::fread(buffer, 1, BLOCK_SIZE, file);
                    std::string data(buffer, bytesRead);
                    auto endPacket = packetManager->buildFileEndMessage(totalChunks, data, localIp, &digest);
                    packetManager->sendMessage(endPacket, client->ip, server_socket, sendMutex);
                }      
                
                std::fclose(file);
                
                delete client; 
            } while(packetManager->isCorrupted());

        } else if (response.first == "devices") {
            std::cout << "Dispositivos conectados:\n";
            this->clock->clientsToString();
            std::cin.clear();
            getchar();
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