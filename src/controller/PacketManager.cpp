#include "../include/PacketManager.hpp"
PacketManager::PacketManager(int port) : port(port) {
    actualMessage = nullptr;
    actualSystemId = 0;
    ackManagerMutex.lock(); // começa sempre bloqueado
}

Message_t PacketManager::buildTalkMessage(std::string message, std::string localIp) {
    char name_s[20] = {0};  // Array de chars para 'name'
    uint8_t hash[16] = {0};     // Array de hash (16 bytes)

    uint8_t type = 0x002;  // Tipo de mensagem
    uint8_t id[4] = {0}; 
    intToLogicVectorLittleEndian(this->actualSystemId, id, 4);  // Converte ID para little endian

    const char* name = {0};

    const uint8_t* payload = reinterpret_cast<const uint8_t*>(message.c_str());

    Message_t msg(type, id, name, 0, 0, hash, 0, payload, localIp.c_str());

    return msg; // Se necessário
}

Message_t PacketManager::buildNackMessage(uint8_t* id, uint8_t reason, std::string localIp) {
    char name_s[20] = {0};  // Array de chars para 'name'
    uint8_t hash[16] = {0};     // Array de hash (16 bytes)

    uint8_t type = 0x0007;  // Tipo de mensagem

    const char* name = {0};

    const uint8_t* payload = {0};

    Message_t msg(type, id, name, 0, 0, hash, reason, payload, localIp.c_str());

    return msg; // Se necessário
}

bool PacketManager::sendMessage(Message_t packet, std::string ip, int sock, std::mutex& mtx) {

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

    int n;
    {
        std::lock_guard<std::mutex> lock(mtx); // protege o acesso ao socket
        n = sendto(sock, &packet, sizeof(packet), 0, (struct sockaddr*)&addr, sizeof(addr));
    }
    if (n < 0) {
        std::cerr << "Erro ao enviar pacote!" << std::endl;
        return false;
    }    

    // precisa validar um ack, senão pacote pode só ser perdido
    while (true)
    {        
        if (ackManagerMutex.try_lock_for(std::chrono::seconds(5))) {
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

    return true;
} 

bool PacketManager::isAcked() {
    return this->actualMessage->acked;
}

/*

return false;
}


void PacketManager::retransmitPacket() {
    
// lógica para a retransmissão de um pacote
// mesma coisa da lógica de envio, mas ainda não sei qual a melhor opção
}
*/


