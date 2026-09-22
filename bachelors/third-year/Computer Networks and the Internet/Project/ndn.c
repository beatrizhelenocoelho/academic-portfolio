#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <sys/select.h>
#include <netdb.h>
#define MAX_NODES 100
#define BUFFER_SIZE 1024

int especial = 0;

typedef struct {
    char name[100];
    int fds[MAX_NODES];       // descritores TCP das interfaces
    char states[MAX_NODES];   
    int num_interfaces;
} InterestEntry;


InterestEntry interest_table[MAX_NODES];
int num_interests = 0;

#define MAX_OBJECTS 100

typedef struct {
    char name[100];  // Nome do objeto
    int is_local;    // verifica se o objeto está na lista de objetos ou na cache 
} StoredObject;

StoredObject object_cache[MAX_OBJECTS];  // lista de cache e objetos 
int num_objects = 0;


typedef struct {
    char ip[16];
    int tcp_port;
} NodeInfo;

NodeInfo neighbors[MAX_NODES];
int num_neighbors = 0;
NodeInfo external_neighbor;
NodeInfo safeguard_node;
int has_external = 0;


typedef struct {
    int fd;
    char ip[16];
    int port;
} ActiveNeighbor;

fd_set master_fds;
int max_fd;
ActiveNeighbor active_neighbors[MAX_NODES];
int num_active = 0;

void commands() {
    printf("\n===========================\n");
    printf("  Estes são os comandos disponíveis:\n");
    printf("===========================\n");
    printf("  join (j) net                               - Entrada de um nó na rede net \n");
    printf("  direct join (dj) connectIP connectTCP      - Ligar diretamente a um nó (direct join)\n");
    printf("  show topology (st)                         - Topologia da rede\n");
    printf("  leave (l)                                  - Saída do nó na rede\n");
    printf("  c name                                     - Cria um objeto com nome <name> no nó \n");
    printf("  retrieve (r) name                          - Pesquisa do objeto com nome <name> \n");
    printf("  exit (x)                                   - Nó sai da rede\n");
    printf("===========================\n\n");
}

void show_names() {
    printf("\n===========================\n");
    printf("  Objetos armazenados no nó:\n");
    printf("===========================\n");

    if (num_objects == 0) {
        printf("  [INFO] Nenhum objeto armazenado.\n");
    } else {
        for (int i = 0; i < num_objects; i++) {
            printf("  - %s", object_cache[i].name);
            if (object_cache[i].is_local==1) {
                printf(" (lista dos objetos)");
            } else {
                printf(" (lista da cache)");
            }
            printf("\n");
        }
    }

    printf("===========================\n\n");
}

void show_interest_table() {
    printf("\n===========================\n");
    printf("  Tabela de Interesses Pendentes:\n");
    printf("===========================\n");

    if (num_interests == 0) {
        printf("  [INFO] Nenhum interesse pendente.\n");
    } else {
        for (int i = 0; i < num_interests; i++) {
            printf("  - Interesse por: %s\n", interest_table[i].name);
            printf("    Interfaces associadas: ");
            if (interest_table[i].num_interfaces == 0) {
                printf("(nenhuma interface)\n");
            } else {
                for (int j = 0; j < interest_table[i].num_interfaces; j++) {
                    printf("%d ", interest_table[i].fds[j]);
                }
                printf("\n");
            }
        }
    }
    printf("===========================\n\n");
}

void create_object(const char *object_name) {
    if (num_objects >= MAX_OBJECTS) {
        printf("A cache de objetos está cheia!\n");
        return;
    }

    // Verifica se o objeto já existe
    for (int i = 0; i < num_objects; i++) {
        if (strcmp(object_cache[i].name, object_name) == 0) {
            printf("O objeto %s já existe neste nó.\n", object_name);
            return;
        }
    }

    // Adiciona o objeto na cache
    strncpy(object_cache[num_objects].name, object_name, sizeof(object_cache[num_objects].name));
    object_cache[num_objects].is_local = 1; // Marca como criado localmente
    num_objects++;

    printf(" Objeto %s criado e armazenado neste nó.\n", object_name);
}


int na_cache(const char *object_name) {
    for (int i = 0; i < num_objects; i++) {
        if (strcmp(object_cache[i].name, object_name) == 0) {
            return 1;  // Objeto encontrado
        }
    }
    return 0;  // Objeto não encontrado
}

void send_object(int fd, const char *object_name) {
    char message[BUFFER_SIZE];

    // Verifica se o objeto está na cache antes de enviar
    if (!na_cache(object_name)) {
        printf("Tentou enviar o objeto :  %s, mas não está na cache\n", object_name);
        return;
    }

    // Cria a mensagem no formato correto
    snprintf(message, sizeof(message), "OBJECT %s\n", object_name);

    // Envia a mensagem para o nó que solicitou
    if (send(fd, message, strlen(message), 0) < 0) {
        perror("Erro ao enviar objeto");
    } else {
        printf("Objeto %s enviado para FD %d.\n", object_name, fd);
    }
}

// cria e associa um socket UDP a uma porta UDP especifica  
int create_udp_socket(int port) {
    int sockfd;
    struct addrinfo hints, *res;
    char port_str[6];

    snprintf(port_str, sizeof(port_str), "%d", port);//converter o numero da porta para string
    //para o getaddrinfo
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;         //IPv4
    hints.ai_socktype = SOCK_DGRAM;    // UDP
    hints.ai_flags = AI_PASSIVE;       // Aceita conexões de qualquer endereço

    if (getaddrinfo(NULL, port_str, &hints, &res) != 0) { //preenche res com um endereço local onde o socket pode fazer bind.
        perror("Erro no getaddrinfo UDP");
        return -1;
    }
    
    //cria socket UDP, com parametros no res
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        perror("Erro ao criar socket UDP");
        freeaddrinfo(res);
        return -1;
    }

    //bind do socket à porta, para que comece listening nessa porta
    if (bind(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        perror("Erro ao bind UDP");
        close(sockfd);
        freeaddrinfo(res);
        return -1;
    }

    freeaddrinfo(res);
    printf("[INFO] Socket UDP criado na porta %d\n", port);
    return sockfd;//return do 
}

// Criar socket TCP em listen na porta especifica port - para aceitar coneccoes de outros nos
int create_tcp_socket(int port) {
    int sockfd;//descrito de socket TCP que vai ser criado
    struct addrinfo hints, *res;//res - resultado do getaddtinfo
    char port_str[6];

    snprintf(port_str, sizeof(port_str), "%d", port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;         // IPv4
    hints.ai_socktype = SOCK_STREAM;   // TCP
    hints.ai_flags = AI_PASSIVE;       // Aceita conexões de qualquer endereço

    if (getaddrinfo(NULL, port_str, &hints, &res) != 0) {//gera um endereço que pode ser usado no bind, usando os hints 
        perror("Erro no getaddrinfo TCP");
        return -1;
    }

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        perror("Erro ao criar socket TCP");
        freeaddrinfo(res);
        return -1;
    }

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    perror("Erro ao definir SO_REUSEADDR");
    close(sockfd);
    return -1;
    }


    if (bind(sockfd, res->ai_addr, res->ai_addrlen) < 0) {//faz o bind do socket à porta especificada, liga o socket ao endereco e port desejados
        perror("Erro ao bind TCP");
        close(sockfd);
        freeaddrinfo(res);
        return -1;
    }

    if (listen(sockfd, 5) < 0) {//modo de escuta, para o socket poder 
        perror("Erro ao escutar conexões TCP");
        close(sockfd);
        freeaddrinfo(res);
        return -1;
    }

    freeaddrinfo(res);
    printf("Socket TCP a escutar na porta %d\n", port);
    return sockfd;//return o sockfd (descritor do socket) para ser usado no select(), accept() etc.
}

void guardar_cache(const char *object_name) {
    if (num_objects >= MAX_OBJECTS) {
        printf("Cache de objetos cheia! Não foi possível armazenar %s.\n", object_name);
        return;
    }

    // Verifica se o objeto já está armazenado
    for (int i = 0; i < num_objects; i++) {
        if (strcmp(object_cache[i].name, object_name) == 0) {
            printf("Objeto %s já está na cache.\n", object_name);
            return;
        }
    }

    // Adiciona o objeto à cache
    strncpy(object_cache[num_objects].name, object_name, sizeof(object_cache[num_objects].name) - 1);
    object_cache[num_objects].name[sizeof(object_cache[num_objects].name) - 1] = '\0'; // Garante terminação correta
    object_cache[num_objects].is_local = 0;
    num_objects++;


    printf("O Objeto %s foi armazenado na cache.\n", object_name);
}

void print_interest_table_state() {
    printf("\n===========================\n");
    printf("  Tabela de Interesses Pendentes (PIT):\n");
    printf("===========================\n");

    if (num_interests == 0) {
        printf(" Nenhum interesse pendente.\n");
    } else {
        for (int i = 0; i < num_interests; i++) {
            printf("  - Interesse por: %s\n", interest_table[i].name);
            printf("    Interfaces: ");
            for (int j = 0; j < interest_table[i].num_interfaces; j++) {
                printf("[FD %d - ", interest_table[i].fds[j]);
                if (interest_table[i].states[j] == 'W') printf("WAITING] ");
                else if (interest_table[i].states[j] == 'R') printf("RESPONSE] ");
                else if (interest_table[i].states[j] == 'C') printf("CLOSED] ");
            }
            printf("\n");
        }
    }

    printf("===========================\n\n");
}

void handle_interest(int sender_fd, char *object_name) {
    //printf("Recebido INTEREST por %s do FD %d\n", object_name, sender_fd);

    // Se o objeto está na cache, envia OBJECT imediatamente
    if (na_cache(object_name)) {
        send_object(sender_fd, object_name);
        return;
    }

    // Verifica se já há entrada para este objeto
    for (int i = 0; i < num_interests; i++) {
        if (strcmp(interest_table[i].name, object_name) == 0) {
            // Já existe — adiciona sender_fd como RESPONSE se ainda não estiver lá
            for (int j = 0; j < interest_table[i].num_interfaces; j++) {
                if (interest_table[i].fds[j] == sender_fd) return; // já está
            }

            interest_table[i].fds[interest_table[i].num_interfaces] = sender_fd;
            interest_table[i].states[interest_table[i].num_interfaces] = 'R';
            interest_table[i].num_interfaces++;
            print_interest_table_state();

            // Verifica se restam estados 'W'
            int tem_waiting = 0;
            for (int j = 0; j < interest_table[i].num_interfaces; j++) {
                if (interest_table[i].states[j] == 'W') {
                    tem_waiting = 1;
                    break;
                }
            }

            if (!tem_waiting) {
                // Todas as interfaces estão em RESPONSE → responde com NOOBJECT
                for (int j = 0; j < interest_table[i].num_interfaces; j++) {
                    if (interest_table[i].states[j] == 'R') {
                        char msg[BUFFER_SIZE];
                        snprintf(msg, sizeof(msg), "NOOBJECT %s\n", object_name);
                        send(interest_table[i].fds[j], msg, strlen(msg), 0);
                    }
                }
                interest_table[i] = interest_table[num_interests - 1];
                num_interests--;
                print_interest_table_state();
            }

            return;
        }
    }

    // Nova entrada: envia INTEREST para todos exceto quem enviou
    if (num_interests < MAX_NODES) {
        InterestEntry *entry = &interest_table[num_interests];
        strcpy(entry->name, object_name);
        entry->num_interfaces = 0;

        // Adiciona interface de entrada como RESPONSE
        entry->fds[entry->num_interfaces] = sender_fd;
        entry->states[entry->num_interfaces] = 'R';
        entry->num_interfaces++;

        // Propaga INTEREST pelas restantes
        for (int i = 0; i < num_active; i++) {
            if (active_neighbors[i].fd == sender_fd) continue;

            char msg[BUFFER_SIZE];
            snprintf(msg, sizeof(msg), "INTEREST %s\n", object_name);
            send(active_neighbors[i].fd, msg, strlen(msg), 0);

            entry->fds[entry->num_interfaces] = active_neighbors[i].fd;
            entry->states[entry->num_interfaces] = 'W';
            entry->num_interfaces++;
        }

        num_interests++;
        print_interest_table_state();
    }
}

void handle_object(int sender_fd, char *object_name) {
    printf("Recebido OBJECT %s do FD %d\n", object_name, sender_fd);
    guardar_cache(object_name);

    for (int i = 0; i < num_interests; i++) {
        if (strcmp(interest_table[i].name, object_name) == 0) {
            for (int j = 0; j < interest_table[i].num_interfaces; j++) {
                if (interest_table[i].states[j] == 'R') {
                    char msg[BUFFER_SIZE];
                    snprintf(msg, sizeof(msg), "OBJECT %s\n", object_name);
                    send(interest_table[i].fds[j], msg, strlen(msg), 0);
                }
            }

            // Remove a entrada
            interest_table[i] = interest_table[num_interests - 1];
            num_interests--;
            print_interest_table_state();
            return;
        }
    }
}

void handle_noobject(int sender_fd, char *object_name) {
    printf("Recebido NOOBJECT para %s\n", object_name);

    for (int i = 0; i < num_interests; i++) {
        if (strcmp(interest_table[i].name, object_name) == 0) {
            for (int j = 0; j < interest_table[i].num_interfaces; j++) {
                if (interest_table[i].fds[j] == sender_fd) {
                    interest_table[i].states[j] = 'C';
                    print_interest_table_state();
                    break;
                }
            }

            // Verificar se restam estados WAITING
            int tem_waiting = 0;
            for (int j = 0; j < interest_table[i].num_interfaces; j++) {
                if (interest_table[i].states[j] == 'W') {
                    tem_waiting = 1;
                    break;
                }
            }

            if (!tem_waiting) {
                // Reencaminha NOOBJECT para interfaces em estado RESPONSE
                for (int j = 0; j < interest_table[i].num_interfaces; j++) {
                    if (interest_table[i].states[j] == 'R') {
                        char msg[BUFFER_SIZE];
                        snprintf(msg, sizeof(msg), "NOOBJECT %s\n", object_name);
                        send(interest_table[i].fds[j], msg, strlen(msg), 0);
                    }
                }

                interest_table[i] = interest_table[num_interests - 1];
                num_interests--;
                print_interest_table_state();
            }

            return;
        }
    }
}

// Solicita a lista de nós ao servidor e processa corretamente a resposta NODESLIST
void get_nodes_list(const char *server_ip, int server_port, const char *net, char nodes[MAX_NODES][BUFFER_SIZE], int *num_nodes) {
    int sockfd;
    struct addrinfo hints, *res;
    char buffer[BUFFER_SIZE], port_str[6];

    snprintf(port_str, sizeof(port_str), "%d", server_port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo(server_ip, port_str, &hints, &res) != 0) {//para obter um struct sockaddr correspondente ao IP e porta do servidor
        perror("Erro no getaddrinfo UDP");
        return;
    }

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);//cria um socket udp 
    if (sockfd < 0) {
        perror("Erro ao criar socket UDP");
        freeaddrinfo(res);
        return;
    }

    // Enviar pedido NODES net para o servidor de nós
    snprintf(buffer, sizeof(buffer), "NODES %s\n", net);
    sendto(sockfd, buffer, strlen(buffer), 0, res->ai_addr, res->ai_addrlen);

    // Receber resposta do servidor
    int len = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, NULL, NULL);
   
    if (len <= 0) { //Verifica se foi recebida alguma coisa. Se não, erro.
        printf(" Não recebeu resposta do servidor.\n");
    } else {
        buffer[len] = '\0';//Fecha a string recebida e imprime para debug.

        //printf("\n Resposta do servidor NODESLIST:\n%s\n", buffer);

        *num_nodes = 0;//contador de nós 
        char *line = strtok(buffer, "\n");//dividir a resposta por linhas   

        // Verifica se a resposta começa com NODESLIST
        if (line != NULL && strncmp(line, "NODESLIST", 9) == 0) {//avança para a primira linha com um nó 
            printf(" Lista de nós recebida!\n");
            line = strtok(NULL, "\n"); // salta a linha NODESLIST net

            while (line != NULL && *num_nodes < MAX_NODES) {
                strcpy(nodes[*num_nodes], line);
                //printf(" Nó disponível: %s\n", nodes[*num_nodes]);
                (*num_nodes)++;
                line = strtok(NULL, "\n");
            }
        } else {
            printf("Resposta do servidor inesperada: %s\n", buffer);
        }
    }

    freeaddrinfo(res);
    close(sockfd);
}

// Registra o nó no servidor
void register_node(const char *server_ip, int server_port, const char *net, const char *ip, int tcp_port) {
    int sockfd;
    struct addrinfo hints, *res;
    char buffer[BUFFER_SIZE], port_str[6];

    snprintf(port_str, sizeof(port_str), "%d", server_port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;         // IPv4
    hints.ai_socktype = SOCK_DGRAM;    // UDP

    if (getaddrinfo(server_ip, port_str, &hints, &res) != 0) {
        perror(" Erro no getaddrinfo UDP");
        return;
    }

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        perror("Erro ao criar socket UDP");
        freeaddrinfo(res);
        return;
    }

    snprintf(buffer, sizeof(buffer), "REG %s %s %d\n", net, ip, tcp_port);
    sendto(sockfd, buffer, strlen(buffer), 0, res->ai_addr, res->ai_addrlen);

    int len = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, NULL, NULL);
    if (len > 0) {
        buffer[len] = '\0';
        if (strncmp(buffer, "OKREG", 5) == 0) {
            printf(" Registro confirmado pelo servidor: %s\n", buffer);
        } else {
            printf(" Registro falhou. Resposta inesperada: %s\n", buffer);
        }
    } else {
        printf(" Não recebeu confirmação do servidor.\n");
    }

    freeaddrinfo(res);
    close(sockfd);
}

void unregister_node(const char *server_ip, int server_port, const char *net, const char *ip, int tcp_port) {
    int sockfd;
    struct addrinfo hints, *res;
    char buffer[BUFFER_SIZE], port_str[6];

    snprintf(port_str, sizeof(port_str), "%d", server_port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo(server_ip, port_str, &hints, &res) != 0) {
        perror("Erro no getaddrinfo UDP");
        return;
    }

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        perror("Erro ao criar socket UDP");
        freeaddrinfo(res);
        return;
    }

    snprintf(buffer, sizeof(buffer), "UNREG %s %s %d\n", net, ip, tcp_port);
    sendto(sockfd, buffer, strlen(buffer), 0, res->ai_addr, res->ai_addrlen);

    int len = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);
    if (len > 0 && strncmp(buffer, "OKUNREG", 7) == 0) {
        printf(" Nó removido do servidor: %s\n", buffer);
    } else {
        printf(" Não recebeu confirmação do servidor.\n");
    }

    freeaddrinfo(res);
    close(sockfd);
}

void handle_tcp_connections(int tcp_sock, const char *my_ip, int my_port) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    int new_sock = accept(tcp_sock, (struct sockaddr *)&client_addr, &addr_len);
    if (new_sock < 0) {
        perror("Falha ao aceitar conexão");
        return;
    }

    int len = recv(new_sock, buffer, sizeof(buffer) - 1, 0);
    if (len <= 0) {
        close(new_sock);
        return;
    }

    buffer[len] = '\0';

    
    //  1 . Mensagem ENTRY (ligação nova de um nó a entrar) 
    if (strncmp(buffer, "ENTRY", 5) == 0) {
        char entry_ip[16];
        int entry_port;

        if (sscanf(buffer, "ENTRY %15s %d", entry_ip, &entry_port) == 2) {
            char previous_external_ip[16];
            int previous_external_port;
            char externo_novo_ip[16];
            int externo_novo_port;

            
            //Se estou no caso especial, ou seja, este nó nao tem vizinhos externos 
            printf("Quero saber se este nó tem externos!!\n");
        
           
            //Atualizar o interno 
            int ja_existe = 0;
            for (int i = 0; i < num_neighbors; i++) {
                if (strcmp(neighbors[i].ip, entry_ip) == 0 && neighbors[i].tcp_port == entry_port) {
                    ja_existe = 1;
                    break;
                }
            }

            if (!ja_existe && num_neighbors < MAX_NODES) {
                strncpy(neighbors[num_neighbors].ip, entry_ip, sizeof(neighbors[num_neighbors].ip));
                neighbors[num_neighbors].tcp_port = entry_port;
                num_neighbors++;
                printf("Adicionei vizinho interno %s:%d\n", entry_ip, entry_port);
            }

            strncpy(externo_novo_ip, external_neighbor.ip, sizeof(previous_external_ip));//guardar o externo 
            externo_novo_port = external_neighbor.tcp_port;

            //printf(" external_neighbor.ip, external_neighbor.tcp_port sao %s %d\n", externo_novo_ip, externo_novo_port );

            if (externo_novo_port==0){
                //nao tenho vizinhos externos-situação especial
                //Segundo Entry
                
                char self_entry[BUFFER_SIZE];

                snprintf(self_entry, sizeof(self_entry), "ENTRY %s %d\n", my_ip, my_port);
                send(new_sock, self_entry, strlen(self_entry), 0);
                printf("Mandei este ENTRY  %s %d \n", my_ip, my_port);
                especial = 1;
                //Atualizar o vizinho externo 
                strncpy(external_neighbor.ip, entry_ip, sizeof(external_neighbor.ip));
                external_neighbor.tcp_port = entry_port;
                has_external = 1;
                //fim do primeiro ENTRY - Acontece nos dois casos JÁ ESTAVA A ATUALIZAR OS EXTERNOS 

                //printf("SEGUNDO ENTRY novos vizinhos externos %s:%d após enviar ENTRY\n", entry_ip, entry_port);


            }
 
            //3º PASSO Sempre - Mandar SAFE 
            strncpy(previous_external_ip, external_neighbor.ip, sizeof(previous_external_ip));//guardar o externo 
            previous_external_port = external_neighbor.tcp_port;
            char response[BUFFER_SIZE];
            snprintf(response, sizeof(response), "SAFE %s %d\n", previous_external_ip, previous_external_port);
            printf("---Mandamos  este SAFE %s %d\n", previous_external_ip, previous_external_port); 
            printf("Handle esta com este como my port %s:%d\n", my_ip,my_port);
            send(new_sock, response, strlen(response), 0);

            // Guardar ligação como ativa
            FD_SET(new_sock, &master_fds);
            if (new_sock > max_fd) max_fd = new_sock;
            active_neighbors[num_active].fd = new_sock;
            strncpy(active_neighbors[num_active].ip, entry_ip, sizeof(active_neighbors[num_active].ip));
            active_neighbors[num_active].port = entry_port;
            num_active++;


        } else {
            close(new_sock);
        }
    }

    //  2. Mensagem SAFE (recebida numa nova ligação) 
    else if (strncmp(buffer, "SAFE", 4) == 0) {
        char new_sg_ip[16];
        int new_sg_port;

        if (sscanf(buffer, "SAFE %15s %d", new_sg_ip, &new_sg_port) == 2) {
            strncpy(safeguard_node.ip, new_sg_ip, sizeof(safeguard_node.ip));
            safeguard_node.tcp_port = new_sg_port;
            printf(" SAFE recebido (via nova ligação): novo salvaguarda é %s %d\n", new_sg_ip, new_sg_port);

            // Guardar esta ligação como ativa também!
            FD_SET(new_sock, &master_fds);
            if (new_sock > max_fd) max_fd = new_sock;
            active_neighbors[num_active].fd = new_sock;
            strncpy(active_neighbors[num_active].ip, inet_ntoa(client_addr.sin_addr), sizeof(active_neighbors[num_active].ip));
            active_neighbors[num_active].port = ntohs(client_addr.sin_port);
            num_active++;
        } else {
            close(new_sock);
        }
    }

    // 3. Mensagem inválida ---
    else {
        close(new_sock);
    }
}


void retrieve_object(const char *object_name) {
    printf("Tenta recuperar o objeto %s\n", object_name);

    // Se o objeto já estiver na cache, retorna imediatamente
    if (na_cache(object_name)) {
        printf("Objeto %s já está na cache local.\n", object_name);
        return;
    }

    // Verifica se já temos entrada na PIT
    for (int i = 0; i < num_interests; i++) {
        if (strcmp(interest_table[i].name, object_name) == 0) {
            printf(" %s já foi iniciado.\n", object_name);
            return;
        }
    }

    if (num_interests >= MAX_NODES) {
        printf("Tabela de interesses cheia!\n");
        return;
    }

    // Adiciona nova entrada na PIT com interfaces em estado 'W'
    InterestEntry *entry = &interest_table[num_interests];
    strcpy(entry->name, object_name);
    entry->num_interfaces = 0;

    int interest_sent = 0;

    for (int i = 0; i < num_active; i++) {
        char message[BUFFER_SIZE];
        snprintf(message, sizeof(message), "INTEREST %s\n", object_name);
        if (send(active_neighbors[i].fd, message, strlen(message), 0) > 0) {
            entry->fds[entry->num_interfaces] = active_neighbors[i].fd;
            entry->states[entry->num_interfaces] = 'W';
            entry->num_interfaces++;
            interest_sent = 1;
        }
    }

    if (!interest_sent) {
        printf("Nenhum vizinho disponível envia NOOBJECT (local).\n");
        return;
    }

    num_interests++;
    print_interest_table_state();
}

void direct_join(const char *join_ip, int join_port, const char *my_ip, int my_tcp_port) {
    int sockfd;
    struct addrinfo hints, *res;
    char message[BUFFER_SIZE], port_str[6];

    snprintf(port_str, sizeof(port_str), "%d", join_port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(join_ip, port_str, &hints, &res) != 0) {
        perror("getaddrinfo");
        return;
    }

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        perror("socket()");
        freeaddrinfo(res);
        return;
    }

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        perror("connect()");
        freeaddrinfo(res);
        close(sockfd);
        return;
    }
    
    //Primeiro Entry
    snprintf(message, sizeof(message), "ENTRY %s %d\n", my_ip, my_tcp_port);
    //printf(" ENTRY deste %s %d para este nó %s %d\n", my_ip, my_tcp_port, join_ip,join_port);
    send(sockfd, message, strlen(message), 0);
    strncpy(external_neighbor.ip, join_ip, sizeof(external_neighbor.ip));
    external_neighbor.tcp_port = join_port;
    has_external = 1;
    //fim do primeiro ENTRY - Acontece nos dois casos JÁ ESTAVA A ATUALIZAR OS EXTERNOS 

    printf("Atualizei o meu vizinho externo para %s:%d após enviar ENTRY\n", join_ip, join_port);
    

    int len = recv(sockfd, message, sizeof(message) - 1, 0);
    if (len > 0) {
        message[len] = '\0';
        char safeguard_ip[16];
        int safeguard_port;
        
        char entry_ip[16];
        int entry_port;
       //Receber o ENTRY Ddo handle 
        if (sscanf(message, "ENTRY %15s %d", entry_ip, &entry_port) == 2) { //AQUI NÃO DEVIAMOS ALTERAR OS INTERNOS
            // Se recebermos um ENTRY, atualizamos nosso vizinho externo
           especial =1;
            int ja_existe = 0;
            for (int i = 0; i < num_neighbors; i++) {
                if (strcmp(neighbors[i].ip, entry_ip) == 0 && neighbors[i].tcp_port == entry_port) {
                    ja_existe = 1;
                    break;
                }
            }

            if (!ja_existe && num_neighbors < MAX_NODES) {
                strncpy(neighbors[num_neighbors].ip, entry_ip, sizeof(neighbors[num_neighbors].ip));
                neighbors[num_neighbors].tcp_port = entry_port;
                num_neighbors++;
                printf("Adicionei um vizinho interno %s:%d\n", entry_ip, entry_port);
            }


        }
        //LER O SAFE 3º PASSO não está a ler aqui, podemos remover isto 
        if (sscanf(message, "SAFE %15s %d", safeguard_ip, &safeguard_port) == 2) {
            // Atualizar salvaguarda
            //printf("----LI O SAFE------\n");
            strncpy(safeguard_node.ip, safeguard_ip, sizeof(safeguard_node.ip));
            safeguard_node.tcp_port = safeguard_port;
            printf("Recebi SAFE %s %d \n", safeguard_node.ip, safeguard_node.tcp_port);

        }
        //printf("-----------myport_ e safeguard_port caso 4  %d %d \n", my_tcp_port, safeguard_node.tcp_port);
        
        //Por condicao
            //printf("\n\n especial ---------------- %d\n\n", especial);
            //strncpy(previous_external_ip, external_neighbor.ip, sizeof(previous_external_ip));//guardar o externo 
            //previous_external_port = external_neighbor.tcp_port;
            if(especial==1){
                //printf("Entrei no especial\n");
            //especial=0;
            char response[BUFFER_SIZE];
            snprintf(response, sizeof(response), "SAFE %s %d\n", external_neighbor.ip, external_neighbor.tcp_port);
            send(sockfd, response, strlen(response), 0);

            }


       
    }

    // Guardar como ligação ativa
    FD_SET(sockfd, &master_fds);
    if (sockfd > max_fd) max_fd = sockfd;
    active_neighbors[num_active].fd = sockfd;
    strncpy(active_neighbors[num_active].ip, join_ip, sizeof(active_neighbors[num_active].ip));
    active_neighbors[num_active].port = join_port;
    num_active++;

    freeaddrinfo(res);
}

#include <sys/time.h>  // Adicionar para controlar o timeout

void join_network(const char *server_ip, int server_port, const char *net, const char *my_ip, int my_tcp_port) {
    char nodes[MAX_NODES][BUFFER_SIZE];
    int num_nodes = 0;

    get_nodes_list(server_ip, server_port, net, nodes, &num_nodes);

    if (num_nodes == 0) {
        printf("Nenhum nó encontrado. A criar nova rede...\n");
        register_node(server_ip, server_port, net, my_ip, my_tcp_port);
        return;
    }

    int success = 0;
    for (int attempt = 0; attempt < num_nodes; attempt++) {
        int idx = rand() % num_nodes;
        char join_ip[16];
        int join_port;

        if (sscanf(nodes[idx], "%15s %d", join_ip, &join_port) != 2) continue;
        if (strcmp(join_ip, my_ip) == 0 && join_port == my_tcp_port) continue;

        //printf("Tentar conectar ao nó: %s:%d\n", join_ip, join_port);

        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) continue;

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(join_port);
        inet_pton(AF_INET, join_ip, &addr.sin_addr);

        struct timeval timeout = {.tv_sec = 3, .tv_usec = 0};
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

        if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
            close(sockfd);
            printf("Conexão bem-sucedida com %s:%d\n", join_ip, join_port);
            direct_join(join_ip, join_port, my_ip, my_tcp_port);  // já faz tudo o resto
            success = 1;
            break;
        }

        close(sockfd);
    }

    if (!success) {
        printf("Nenhum nó acessível encontrado. A criar nova rede...\n");
    }

    register_node(server_ip, server_port, net, my_ip, my_tcp_port);
}

void show_topology() {
    printf("\n============================\n");
    printf("[TOPOLOGIA ATUAL]\n");

    if (has_external) {
        printf("Vizinho Externo: %s %d\n", external_neighbor.ip, external_neighbor.tcp_port);
    } else {
        printf("Nenhum vizinho externo definido.\n");
    }

    if (num_neighbors > 0) {
        printf("Vizinhos Internos (%d):\n", num_neighbors);
        for (int i = 0; i < num_neighbors; i++) {
            printf("  - %s %d\n", neighbors[i].ip, neighbors[i].tcp_port);
        }
    } else {
        printf("Nenhum vizinho interno registrado.\n");
    }

    if (strlen(safeguard_node.ip) > 0 && safeguard_node.tcp_port > 0) {
        printf("Nó de Salvaguarda: %s %d\n", safeguard_node.ip, safeguard_node.tcp_port);
    } else {
        printf("Nenhum nó de salvaguarda definido.\n");
    }

    printf("============================\n");
}

void leave_network(const char *server_ip, int server_port, const char *net, const char *ip, int tcp_port, int tcp_sock, int udp_sock) {
    // sai do servidor de nós
    unregister_node(server_ip, server_port, net, ip, tcp_port);

    // fecha todas as ligações TCP ativas (sockets com vizinhos)
    for (int i = 0; i < num_active; i++) {
        int fd = active_neighbors[i].fd;
        FD_CLR(fd, &master_fds);
        close(fd);
    }
    num_active = 0;

    // limpa toda a topologia local
    has_external = 0;
    num_neighbors = 0;
    memset(&external_neighbor, 0, sizeof(external_neighbor));
    memset(&safeguard_node, 0, sizeof(safeguard_node));

    // reconfigura o master_fds com stdin, tcp_sock e udp_sock
    FD_ZERO(&master_fds);
    FD_SET(STDIN_FILENO, &master_fds);
    FD_SET(tcp_sock, &master_fds);
    FD_SET(udp_sock, &master_fds);

    max_fd = tcp_sock;
    if (udp_sock > max_fd) max_fd = udp_sock;
    if (STDIN_FILENO > max_fd) max_fd = STDIN_FILENO;

    printf("Nó saiu da rede com sucesso.\n");
}

void delete_object(const char *object_name) {
    for (int i = 0; i < num_objects; i++) {
        if (strcmp(object_cache[i].name, object_name) == 0) {
            printf("a tirar o objeto %s\n", object_name);
            for (int j = i; j < num_objects - 1; j++) {
                object_cache[j] = object_cache[j + 1];
            }
            num_objects--;
            return;
        }
    }
    printf("Objeto %s não encontrado.\n", object_name);
}


void process_tcp_activity(int fd, const char *my_ip, int my_port) {
    char buffer[BUFFER_SIZE];
    int len = recv(fd, buffer, sizeof(buffer) - 1, 0);

    if (len <= 0) {
        printf("Ligação com FD %d terminou. A reagir...\n", fd);

        char lost_ip[16] = "";
        int lost_port = 0;

        for (int i = 0; i < num_active; i++) {
            if (active_neighbors[i].fd == fd) {
                strncpy(lost_ip, active_neighbors[i].ip, sizeof(lost_ip));
                lost_port = active_neighbors[i].port;
                for (int j = i; j < num_active - 1; j++) {
                    active_neighbors[j] = active_neighbors[j + 1];
                }
                num_active--;
                break;
            }
        }

        close(fd);
        FD_CLR(fd, &master_fds);

        for (int i = 0; i < num_neighbors; i++) {
            if (strcmp(neighbors[i].ip, lost_ip) == 0 && neighbors[i].tcp_port == lost_port) {
                for (int j = i; j < num_neighbors - 1; j++) {
                    neighbors[j] = neighbors[j + 1];
                }
                num_neighbors--;
                break;
            }
        }

        if (strcmp(lost_ip, external_neighbor.ip) == 0 && lost_port == external_neighbor.tcp_port) {
            printf("vizinho externo perdido: %s:%d\n", lost_ip, lost_port);
            has_external = 0;
            memset(&external_neighbor, 0, sizeof(NodeInfo));

            // CASO 1: Este nó é salvaguarda de si próprio
            if (strcmp(my_ip, safeguard_node.ip) == 0 && my_port == safeguard_node.tcp_port) {//o no que saio é salvaguarda de si mesmo
                printf("sou o meu próprio salvaguarda. A promover vizinho interno.\n");

                if (num_neighbors == 0) {
                    printf("não há vizinhos internos para promover!\n");
                    return;
                }

                int chosen = 0;
                for (int i = 1; i < num_neighbors; i++) {
                    if (neighbors[i].tcp_port > neighbors[chosen].tcp_port) {
                        chosen = i;
                    }
                }

                strncpy(external_neighbor.ip, neighbors[chosen].ip, sizeof(external_neighbor.ip));
                external_neighbor.tcp_port = neighbors[chosen].tcp_port;
                has_external = 1;
                //printf("Vim  ao que NAO era susposto -----\n");

                //strncpy(safeguard_node.ip, my_ip, sizeof(safeguard_node.ip));
                //safeguard_node.tcp_port = my_port;

                int sockfd = socket(AF_INET, SOCK_STREAM, 0);
                if (sockfd < 0) return;

                struct sockaddr_in addr;
                memset(&addr, 0, sizeof(addr));
                addr.sin_family = AF_INET;
                addr.sin_port = htons(external_neighbor.tcp_port);
                inet_pton(AF_INET, external_neighbor.ip, &addr.sin_addr);

                if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
                    char msg[BUFFER_SIZE];
                    snprintf(msg, sizeof(msg), "ENTRY %s %d\n", my_ip, my_port);
                    send(sockfd, msg, strlen(msg), 0);

                    int len2 = recv(sockfd, msg, sizeof(msg) - 1, 0);
                    if (len2 > 0) {
                        msg[len2] = '\0';
                        printf(" ENTRY enviado e resposta recebida: %s\n", msg);

                        snprintf(msg, sizeof(msg), "SAFE %s %d\n", external_neighbor.ip, external_neighbor.tcp_port);
                        //printf("-------Saber SAFE enviado foi %s %d\n", external_neighbor.ip, external_neighbor.tcp_port);
                        send(sockfd, msg, strlen(msg), 0);

                    }
                    else {
                        close(sockfd);
                        FD_CLR(sockfd, &master_fds);  // Precaução adicional, mesmo que ainda não esteja
                    }
                    
                }

                for (int i = 0; i < num_neighbors; i++) {
                    if (strcmp(neighbors[i].ip, external_neighbor.ip) == 0 &&
                        neighbors[i].tcp_port == external_neighbor.tcp_port)
                        continue;

                    int socki = socket(AF_INET, SOCK_STREAM, 0);
                    if (socki < 0) continue;

                    struct sockaddr_in addr2;
                    memset(&addr2, 0, sizeof(addr2));
                    addr2.sin_family = AF_INET;
                    addr2.sin_port = htons(neighbors[i].tcp_port);
                    inet_pton(AF_INET, neighbors[i].ip, &addr2.sin_addr);

                    if (connect(socki, (struct sockaddr *)&addr2, sizeof(addr2)) == 0) {
                        char msg[BUFFER_SIZE];
                        snprintf(msg, sizeof(msg), "SAFE %s %d\n", external_neighbor.ip, external_neighbor.tcp_port);
                        send(socki, msg, strlen(msg), 0);
                        printf("SAFE enviado ao interno %s:%d com SG=%s:%d\n",
                               neighbors[i].ip, neighbors[i].tcp_port,
                               external_neighbor.ip, external_neighbor.tcp_port);
                        close(socki);
                    }
                }
                return;
            }

            // CASO 2: Tenho outro salvaguarda 
            printf("[DEBUG] A contactar salvaguarda: %s %d\n", safeguard_node.ip, safeguard_node.tcp_port);

            // agora como vou dar entry o vizinho externo é a salvaguarda 
            strncpy(external_neighbor.ip, safeguard_node.ip, sizeof(external_neighbor.ip));
            external_neighbor.tcp_port = safeguard_node.tcp_port ;

            // chamar dj 
            direct_join(safeguard_node.ip, safeguard_node.tcp_port, my_ip, my_port);

            // enviar safe
            for (int i = 0; i < num_neighbors; i++) {
                // Ignora o próprio salvaguarda para evitar redundância
                if (strcmp(neighbors[i].ip, external_neighbor.ip) == 0 &&
                    neighbors[i].tcp_port == external_neighbor.tcp_port)
                    continue;
            
                int sock = socket(AF_INET, SOCK_STREAM, 0);
                if (sock < 0) continue;
            
                struct sockaddr_in addr;
                memset(&addr, 0, sizeof(addr));
                addr.sin_family = AF_INET;
                addr.sin_port = htons(neighbors[i].tcp_port);
                inet_pton(AF_INET, neighbors[i].ip, &addr.sin_addr);
            
                if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
                    char msg[BUFFER_SIZE];
                    snprintf(msg, sizeof(msg), "SAFE %s %d\n", external_neighbor.ip, external_neighbor.tcp_port);
                    send(sock, msg, strlen(msg), 0);
                    //printf("SAFE enviado espero que bemmmmmmmmmmmmmm para %s:%d\n", neighbors[i].ip, neighbors[i].tcp_port);
                    close(sock);
                }
            }

        }

    } else {  
        // Processamento de mensagens recebidas
        buffer[len] = '\0';

        if(strncmp(buffer, "ENTRY", 5) == 0){
            char new_ip_2[16];
            int new_port_2;
            if (sscanf(buffer, "ENTRY %15s %d", new_ip_2, &new_port_2) == 2) {
                //adicionar internos 
                int ja_existe = 0;
                for (int i = 0; i < num_neighbors; i++) {
                    if (strcmp(neighbors[i].ip, new_ip_2) == 0 && neighbors[i].tcp_port == new_port_2) {
                        ja_existe = 1;
                        break;
                    }
                }
    
                if (!ja_existe && num_neighbors < MAX_NODES) {
                    strncpy(neighbors[num_neighbors].ip, new_ip_2, sizeof(neighbors[num_neighbors].ip));
                    neighbors[num_neighbors].tcp_port = new_port_2;
                    num_neighbors++;
                    printf("recebi um interno novo %s:%d\n", new_ip_2, new_port_2);
                }
            }

        }

        else if (strncmp(buffer, "SAFE", 4) == 0){
            char new_ip[16];
            int new_port;
            if (sscanf(buffer, "SAFE %15s %d", new_ip, &new_port) == 2) {
                strncpy(safeguard_node.ip, new_ip, sizeof(safeguard_node.ip));
                safeguard_node.tcp_port = new_port;
                printf("SAFE recebido: novo salvaguarda é %s %d\n", new_ip, new_port);
            }
        } else if (strncmp(buffer, "INTEREST", 8) == 0) {
            char object_name[100];
            sscanf(buffer, "INTEREST %99s", object_name);
            handle_interest(fd, object_name);
        } else if (strncmp(buffer, "NOOBJECT", 8) == 0) {
            char object_name[100];
            sscanf(buffer, "NOOBJECT %99s", object_name);
            handle_noobject(fd, object_name);
        } else if (strncmp(buffer, "OBJECT", 6) == 0) {
            char object_name[100];
            sscanf(buffer, "OBJECT %99s", object_name);
            handle_object(fd, object_name);
        }
      }
}

int main(int argc, char *argv[]) {
    if (argc < 6) {
        printf("Uso: %s cache IP TCP regIP regUDP\n", argv[0]);
        return EXIT_FAILURE;
    }

    char net[4] = "";
    char *ip = argv[2];
    int tcp_port = atoi(argv[3]);
    char *reg_ip = argv[4];
    int reg_udp_port = atoi(argv[5]);

    // Criar sockets TCP e UDP
    int tcp_sock = create_tcp_socket(tcp_port);
    int udp_sock = create_udp_socket(0);

    fd_set read_fds;
    FD_ZERO(&master_fds);
    FD_SET(STDIN_FILENO, &master_fds);
    FD_SET(tcp_sock, &master_fds);
    FD_SET(udp_sock, &master_fds);

    max_fd = tcp_sock;
    if (udp_sock > max_fd) max_fd = udp_sock;
    if (STDIN_FILENO > max_fd) max_fd = STDIN_FILENO;

    commands();

    while (1) {
        read_fds = master_fds;

        printf(" espera de comandos\n");

        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("Erro no select()");
            break;
        }

        // Comandos do utilizador
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            char command[BUFFER_SIZE];
            if (fgets(command, sizeof(command), stdin) == NULL) continue;

            if (strncmp(command, "dj", 2) == 0) {
                char join_ip[16];
                int join_port;
                if (sscanf(command, "dj %15s %d", join_ip, &join_port) == 2) {
                    direct_join(join_ip, join_port, ip, tcp_port);
                } else {
                    printf("comando inválido. Use: dj <IP> <PORT>\n");
                }
            } else if (strncmp(command, "j", 1) == 0) {
                char network[4];
                if (sscanf(command, "j %3s", network) == 1) {
                    strncpy(net, network, sizeof(net) - 1);
                    net[sizeof(net) - 1] = '\0';
                    join_network(reg_ip, reg_udp_port, net, ip, tcp_port);
                } else {
                    printf("comando inválido. Use: j <rede>\n");
                }
            } else if (strncmp(command, "c ", 2) == 0) {  
                char object_name[100];
                if (sscanf(command, "c %99s", object_name) == 1) {
                    create_object(object_name);
                } else {
                    printf("comando inválido. Use: c <nome_do_objeto>\n");
                }
            } else if (strncmp(command, "dl ", 3) == 0) {  
                char object_name[100];
                if (sscanf(command, "dl %99s", object_name) == 1) {
                    delete_object(object_name);
                } else {
                    printf("comando inválido. Use: dl <nome_do_objeto>\n");
                }
            } else if (strncmp(command, "r ", 2) == 0) {  
                char object_name[100];
                if (sscanf(command, "r %99s", object_name) == 1) {
                    retrieve_object(object_name);
                } else {
                    printf("comando inválido. Use: r <nome_do_objeto>\n");
                }
            } else if (strncmp(command, "sn", 2) == 0) {  
                show_names();
            } else if (strncmp(command, "si", 2) == 0) {  // 🔹 Novo comando: show interest table
                show_interest_table();
            } else if (strncmp(command, "l", 1) == 0) {
                leave_network(reg_ip, reg_udp_port, net, ip, tcp_port, tcp_sock, udp_sock);
            } else if (strncmp(command, "st", 2) == 0) {
                show_topology();
            } else if (strncmp(command, "x", 1) == 0) {
                unregister_node(reg_ip, reg_udp_port, net, ip, tcp_port);
                printf(" fechar a aplicação...\n");
                break;
            } else {
                printf("Comando inválido.\n");
            }
        }

        // Percorrer todos os descritores até max_fd
        for (int fd = 0; fd <= max_fd; fd++) {
            if (FD_ISSET(fd, &read_fds)) {
                if (fd == tcp_sock) {
                    handle_tcp_connections(tcp_sock, ip, tcp_port);
                } else if (fd == udp_sock) {
                    char buffer[BUFFER_SIZE];
                    struct sockaddr_in sender_addr;
                    socklen_t sender_len = sizeof(sender_addr);
                    int len = recvfrom(udp_sock, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&sender_addr, &sender_len);
                    if (len > 0) {
                        buffer[len] = '\0';
                        printf("Mensagem UDP recebida: %s\n", buffer);
                    }
                } else if (fd != STDIN_FILENO) {
                    // Atividade em um vizinho ativo
                    printf("Atividade no vizinho FD %d\n", fd);
                    process_tcp_activity(fd, ip, tcp_port);
                }
            }
        }
    }

    close(tcp_sock);
    close(udp_sock);
    return 0;
}
