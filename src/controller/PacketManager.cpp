#include "../include/PacketManager.hpp"
PacketManager::PacketManager(int port, std::string localName) : port(port), localName(localName) {
    actualMessage = nullptr;
    actualSystemId = 0;
    ackManagerMutex.lock(); // começa sempre bloqueado
}

Message_t PacketManager::buildTalkMessage(std::string message, std::string localIp) {
    uint8_t hash[16] = {0};     // Array de hash (16 bytes)

    uint8_t type = 0x002;  // Tipo de mensagem
    uint8_t id[4] = {0}; 
    intToLogicVectorLittleEndian(this->actualSystemId, id, 4);  // Converte ID para little endian

    const char* name = "";

    const uint8_t* payload = reinterpret_cast<const uint8_t*>(message.c_str());

    uint16_t payloadSize = message.size();

    char fileName[20] = {0}; // nome do arquivo

    Message_t msg(type, id, name, 0, 0, hash, 0, payload, localIp.c_str(), payloadSize, fileName);

    return msg; // Se necessário
}

Message_t PacketManager::buildNackMessage(uint8_t* id, uint8_t reason, std::string localIp) {
    uint8_t hash[16] = {0};     // Array de hash (16 bytes)

    uint8_t type = 0x0007;  // Tipo de mensagem

    const char* name = "";

    const uint8_t payload[1410] = {0};

    char fileName[20] = {0}; // nome do arquivo

    Message_t msg(type, id, name, 0, 0, hash, reason, payload, localIp.c_str(), 0, fileName);

    return msg; // Se necessário
}

Message_t PacketManager::buildAckMessage(uint8_t* id, std::string localIp) {
    uint8_t hash[16] = {0};     // Array de hash (16 bytes)

    uint8_t type = 0x0006;  // Tipo de mensagem

    const char* name = "";

    const uint8_t payload[1410] = {0};

    char fileName[20] = {0}; // nome do arquivo

    Message_t msg(type, id, name, 0, 0, hash, 0, payload, localIp.c_str(), 0, fileName);

    return msg; // Se necessário
}

std::vector<Message_t> PacketManager::buildFileMessage(std::string fileContent, std::string localIp, std::string fileName) {

    uint8_t length = ceil((float)fileContent.size() / (float)Message_t::MAX_PAYLOAD_SIZE);

    // std::cout << "Length: " << (int)length << std::endl;

    std::vector<Message_t> packets(length); // Vetor de pacotes

    std::vector<uint8_t> digest(MD5_DIGEST_LENGTH); // MD5_DIGEST_LENGTH = 16 bytes

    MD5(reinterpret_cast<const uint8_t*>(fileContent.c_str()), fileContent.size(), digest.data());

    
    if(length < 1) {
        std::cerr << "Tamanho inválido para mensagem do tipo File." << std::endl;
        throw std::invalid_argument("Tamanho inválido para mensagem do tipo File.");
    } else if(length == 1) {
        
        packets[0] = buildFileStartMessage(length, fileContent, localIp, &digest, fileName);

    } else if(length == 2) {

        std::cout << "Caí aqui\n";

        packets[0] = buildFileStartMessage(length, fileContent.substr(0, Message_t::MAX_PAYLOAD_SIZE), localIp, nullptr, fileName);

        std::cout << "montei pacote start\n";

        packets[1] = buildFileEndMessage(length, fileContent.substr(Message_t::MAX_PAYLOAD_SIZE), localIp, &digest);

    } else {

        packets[0] = buildFileStartMessage(length, fileContent.substr(0, Message_t::MAX_PAYLOAD_SIZE), localIp, nullptr, fileName);
        
        for(int i = 1; i < length - 1; ++i) {
            packets[i] = buildFileChunkMessage(length, i, fileContent.substr(i * Message_t::MAX_PAYLOAD_SIZE, Message_t::MAX_PAYLOAD_SIZE), localIp, nullptr);
        }

        packets[length - 1] = buildFileEndMessage(length, fileContent.substr((length - 1) * Message_t::MAX_PAYLOAD_SIZE), localIp, &digest);
    }

    return packets;
}

Message_t PacketManager::buildFileStartMessage(uint8_t length, std::string message, std::string localIp, std::vector<uint8_t>* digest, std::string fileName) {
            
    uint8_t type = 0x0003;  // Tipo de mensagem
    uint8_t id[4] = {0}; 
    intToLogicVectorLittleEndian(this->actualSystemId, id, 4);  // Converte ID para little endian
    
    const char* name = this->localName.c_str();
    const char* fileNameCStr = fileName.c_str();
    
    uint8_t seq = 0; // Sequência inicial

    uint8_t payload[Message_t::MAX_PAYLOAD_SIZE] = {0}; // zera o buffer
    
    if (message.size() < Message_t::MAX_PAYLOAD_SIZE) {
        std::memcpy(payload, message.data(), message.size());;    

        payload[message.size()] = '\0'; // OK, cabe
    } else {

        std::memcpy(payload, message.data(), Message_t::MAX_PAYLOAD_SIZE); // Copia só até o limite
        // Não adiciona '\0' porque não há espaço
    }
    
    uint8_t hash[16] = {0};     

    if((digest != nullptr) && (digest->size() == 16)) {
        for(int i = 0; i < 16; ++i) {
            hash[i] = (*digest)[i];
        }
    }

    uint16_t payloadSize = message.size();

    Message_t msg(type, id, name, length, seq, hash, 0, payload, localIp.c_str(), payloadSize, fileNameCStr);

    return msg; // Se necessário
}

Message_t PacketManager::buildFileEndMessage(uint8_t length, std::string message, std::string localIp, std::vector<uint8_t>* digest) {
            
    uint8_t type = 0x0005;  // Tipo de mensagem
    uint8_t id[4] = {0}; 
    intToLogicVectorLittleEndian(this->actualSystemId, id, 4);  // Converte ID para little endian
    
    const char* name = this->localName.c_str();
    
    uint8_t seq = length - 1;

    uint8_t payload[Message_t::MAX_PAYLOAD_SIZE] = {0}; // zera o buffer
    
    if (message.size() < Message_t::MAX_PAYLOAD_SIZE) {
        std::memcpy(payload, message.data(), message.size());;    

        payload[message.size()] = '\0'; // OK, cabe
    } else {

        std::memcpy(payload, message.data(), Message_t::MAX_PAYLOAD_SIZE); // Copia só até o limite
        // Não adiciona '\0' porque não há espaço
    }

    uint8_t hash[16] = {0};     

    if(digest->size() == 16) {
        for(int i = 0; i < 16; ++i) {
            hash[i] = (*digest)[i];
        }
    } else {
        // lançar exceção
        throw std::invalid_argument("Digest inválido.");
    }

    uint16_t payloadSize = message.size();

    char fileName[20] = {0}; // nome do arquivo

    Message_t msg(type, id, name, length, seq, hash, 0, payload, localIp.c_str(), payloadSize, fileName);

    return msg; // Se necessário
}

Message_t PacketManager::buildFileChunkMessage(uint8_t length, uint8_t seq, std::string message, std::string localIp, std::vector<uint8_t>* digest) {
            
    uint8_t type = 0x0004;  // Tipo de mensagem
    uint8_t id[4] = {0}; 
    intToLogicVectorLittleEndian(this->actualSystemId, id, 4);  // Converte ID para little endian
    
    const char* name = this->localName.c_str();
    char fileName[20] = {0}; // nome do arquivo

    uint8_t payload[Message_t::MAX_PAYLOAD_SIZE] = {0}; // zera o buffer
    
    if (message.size() < Message_t::MAX_PAYLOAD_SIZE) {
        std::memcpy(payload, message.data(), message.size() - 1);;    

        payload[message.size()] = '\0'; // OK, cabe
    } else {

        std::memcpy(payload, message.data(), Message_t::MAX_PAYLOAD_SIZE); // Copia só até o limite
        // Não adiciona '\0' porque não há espaço
    }
    uint8_t hash[16] = {0};     

    if((digest != nullptr) && (digest->size() == 16)) {
        for(int i = 0; i < 16; ++i) {
            hash[i] = (*digest)[i];
        }
    }



    uint16_t payloadSize = message.size();

    Message_t msg(type, id, name, length, seq, hash, 0, payload, localIp.c_str(), payloadSize, fileName);

    return msg; // Se necessário
}

bool PacketManager::sendMessage(Message_t packet, std::string ip, int sock, std::mutex& mtx) {

    std::cout << "Enviando mensagem...\n";

    if(actualMessage != nullptr && !actualMessage->acked) {
        std::cout << "Mensagem anterior não foi 'acked'. Não é possível enviar nova mensagem." << std::endl;
        return false; // Não pode enviar nova mensagem se a anterior não foi "acked"
    }

    if(sock == -1) {
        std::cerr << "Socket inválido." << std::endl;
        return false; // Socket inválido
    }

    Message* msg = new Message();
    msg->msg = &packet;
    msg->acked = false; // Inicializa como não "ackeado"
    this->actualMessage = msg;

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(this->port);  // Porta do servidor de escuta
    addr.sin_addr.s_addr = inet_addr(ip.c_str()); // IP do cliente

    std::cout << "IP: " << ip << std::endl;

    int n;
    {
        std::lock_guard<std::mutex> lock(mtx); // protege o acesso ao socket
        n = sendto(sock, &packet, sizeof(packet), 0, (struct sockaddr*)&addr, sizeof(addr));
    }
    if (n < 0) {
        std::cerr << "Erro ao enviar pacote!" << std::endl;
        return false;
    }    

    std::cout << "Pacote enviado!\n";

    // precisa validar um ack, senão pacote pode só ser perdido
    while (true)
    {        
        if (ackManagerMutex.try_lock_for(std::chrono::seconds(1))) {
            std::cout << "Recebi o ack!\n";            
            delete msg;
            actualMessage = nullptr; // Limpa a mensagem atual
            break; // Se receber o ack, sai do loop
        } else {
            std::cout << "Não recebi o ack!\n";
        
            this->retransmitPacket(sock, mtx, packet, addr);
        }
    }
    
    actualSystemId++; // depois que a mensagem for "acked", incrementa o ID do sistema

    return true; // Retorna true se o envio for bem-sucedido
}

void PacketManager::sendMessageWithoutAck(Message_t packet, std::string ip, int sock, std::mutex& mtx) {
    if(sock == -1) {
        std::cerr << "Socket inválido." << std::endl;
        return; // Socket inválido
    }

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(this->port);  // Porta do servidor de escuta
    addr.sin_addr.s_addr = inet_addr(ip.c_str()); // IP do cliente

    int n;
    {
        std::lock_guard<std::mutex> lock(mtx); // protege o acesso ao socket
        n = sendto(sock, &packet, sizeof(packet), 0, (struct sockaddr*)&addr, sizeof(addr));
    }
    if (n < 0) {
        std::cerr << "Erro ao enviar pacote!" << std::endl;
    } 
}

void PacketManager::retransmitPacket(int sock, std::mutex& mtx, Message_t packet, struct sockaddr_in addr) {
    std::cout << "Tentando retransmitir o pacote...\n";
    
    int n;
    {
        std::lock_guard<std::mutex> lock(mtx); // protege o acesso ao socket
        n = sendto(sock, &packet, sizeof(packet), 0, (struct sockaddr*)&addr, sizeof(addr));
    }
    if (n < 0) {
        std::cerr << "Erro ao retransmitir pacote!" << std::endl;
    } 
}
bool PacketManager::verifyAck(uint8_t* ack) {

    if(this->actualMessage == nullptr) {
        return false; // Mensagem atual não existe
    }

    // comparar 2 uint8_t
    uint8_t* id = this->actualMessage->msg->id;
    if(std::memcmp(ack, id, 4) != 0) {
        std::cout << "Ack inválido\n";
        return false; // Ack inválido
    }        
    // se já tinha sido "ackeado", então retorna false (ignore repetitive acks)
    if(isAcked()) {
        return false;
    }

    actualMessage->acked = true;
    ackManagerMutex.unlock(); 

    return true;
} 

bool PacketManager::isAcked() {
    return this->actualMessage->acked;
}

void PacketManager::handleNack(uint8_t reason) {
 
    switch (reason)
    {
    case 0x01:
        std::cout << "NACK recebido: conexão não existe.\n";
        break;
    
    case 0x02:
        std::cout << "NACK recebido: conexão não disponível ao enviar pacote de FILE\n";
        break; 

    default:
        std::cout << "NACK desconhecido\n";
    }
    
    actualMessage->acked = true; // Marca como "ackeado" para evitar retransmissão
    ackManagerMutex.unlock(); // Libera o mutex
}