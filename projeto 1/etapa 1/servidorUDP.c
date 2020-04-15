#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

const int TAMCAB = 12; //  TAMANHO 12 BYTES
const uint16_t ESTAGIO = 2;

typedef struct
{
    uint32_t len;
    uint32_t psecret;
    uint16_t estagio;
    uint16_t matricula;
} cabecalho;
typedef struct
{
    cabecalho c;
    char dados[12];
} etapaA1;
typedef struct
{
    cabecalho c;
    u_int16_t len;
    u_int16_t num;
    u_int16_t novaPortaUdp;
} etapaA2;
typedef struct
{
    cabecalho cab;
    u_int16_t len;
    u_int16_t num;
    u_int16_t secretC;
    char c;
    char completar;
} etapaC2;
typedef struct
{
    cabecalho c;
    int32_t dado;
} ack;
typedef struct
{
    cabecalho c;
    u_int32_t tcpPorta;
    u_int32_t secret;
} etapaB2;
typedef struct
{
    cabecalho c;
    u_int32_t secret;
} etapaD2;

void preparaCabecalhoRecebe(cabecalho *c)
{
    c->estagio = ntohs(c->estagio);
    c->matricula = ntohs(c->matricula);
    c->len = ntohl(c->len);
    c->psecret = ntohl(c->psecret);
}
void preparaCabecalhoEnviar(cabecalho *c)
{
    c->len = htonl(c->len);
    c->psecret = htonl(c->psecret);
    c->estagio = htons(ESTAGIO);
    c->matricula = htons(c->matricula);
}
void preparaEtapaA2Enviar(etapaA2 *a2)
{
    a2->len = htons(a2->len);
    a2->novaPortaUdp = htons(a2->novaPortaUdp);
    a2->num = htons(a2->num);
}
void preparaAckEnviar(ack *a)
{
    a->dado = htonl(a->dado);
}
void preparaAckRecebe(ack *a)
{
    a->dado = ntohl(a->dado);
}
void preparaEtapaB2Enviar(etapaB2 *b2)
{
    b2->secret = htonl(b2->secret);
    b2->tcpPorta = htonl(b2->tcpPorta);
}
void preparaEtapaB2Recebe(etapaB2 *b2)
{
    b2->secret = ntohl(b2->secret);
    b2->tcpPorta = ntohl(b2->tcpPorta);
}
void preparaEtapaD2Enviar(etapaD2 *d2)
{
    d2->secret = htonl(d2->secret);
}
void preparaEtapaD2Recebe(etapaD2 *d2)
{
    d2->secret = ntohl(d2->secret);
}
void imprimirD2(etapaD2 *d2)
{
    printf("secret %d\n", d2->secret);
}
void imprimirB2(etapaB2 *b2)
{
    printf("secret %d\n", b2->secret);
    printf("TCP Porta %d\n", b2->tcpPorta);
}
void imprimirCabecalho(cabecalho *c)
{
    printf("---- Cabecalho ----\n");
    printf("Estagio %d\n", c->estagio);
    printf("Len %d\n", c->len);
    printf("Matricula %d\n", c->matricula);
    printf("PSecret %d\n", c->psecret);
}

void imprimirEtapaA1(etapaA1 *a1)
{
    printf("-------- Etapa A1 --------\n");
    cabecalho c = a1->c;
    preparaCabecalhoRecebe(&c);
    imprimirCabecalho(&c);
    printf("---- Dados ----\n");
    char str[12];
    memcpy(str, a1->dados, c.len);
    printf("Texto %s\n", str);
    //free(str);
}
void impromirEtapaA2(etapaA2 *e2)
{
    printf("---- Dados ----\n");
    printf("Len %d\n", e2->len);
    printf("Nova Porta Udp %d\n", e2->novaPortaUdp);
    printf("Num %d\n", e2->num);
}
void preparaEtapaA2Recebe(etapaA2 *a2)
{
    a2->len = ntohs(a2->len);
    a2->novaPortaUdp = ntohs(a2->novaPortaUdp);
    a2->num = ntohs(a2->num);
}
void preparaEtapaC2Enviar(etapaC2 *c2)
{
    c2->len = ntohs(c2->len);
    c2->num = htons(c2->num);
    c2->secretC = htons(c2->secretC);
}
void preparaEtapac2Recebe(etapaC2 *c2)
{
    c2->len = ntohs(c2->len);
    c2->num = ntohs(c2->num);
    c2->secretC = ntohs(c2->secretC);
}
void imprimeEtapaC2(etapaC2 *c2)
{
    printf("-------- Etapa C2 --------\n");
    cabecalho c = c2->cab;
    imprimirCabecalho(&c);
    printf("---- Dados ----\n");
    printf("Len %d\n", c2->len);
    printf("Secret %d\n", c2->secretC);
    printf("Num %d\n", c2->num);
    printf("char %c\n", c2->c);
}
void imprimeAck(ack *a)
{
    printf("-------- ACK Dado --------\n");
    printf("ID %d\n", a->dado);
}
int verificaCabecalho(cabecalho *c, cabecalho *c2)
{
    if (c->matricula == c2->matricula && c->len == c2->len && c->estagio == c2->estagio && c->psecret == c2->psecret)
        return 1;
    return 0;
}
void etapaCD(int porta, int mat)
{

    printf("-------- Etapa C --------\n");
    //int porta = 13456;
    in_port_t servPort = porta;

    // Cria socket para as requisições de conexões que chegarem
    int servSock;
    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        printf("socket() falhou\n");
        return;
    }
    
    // Constrói struct de endereço local
    struct sockaddr_in servAddr;            // Endereço local
    memset(&servAddr, 0, sizeof(servAddr)); // Limpa a struct
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(servPort); // Porta local

    // Associa ao endereço local
    if (bind(servSock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
    {
        printf("bind() falhou\n");
        return;
    }

    // Marca o socket que irá escutar as requisições de conexão
    if (listen(servSock, 3) < 0)
    {
        printf("listen() falhou\n");
        return;
    }

    struct sockaddr_in clntAddr; // Endereço cliente
    socklen_t clntAddrLen = sizeof(clntAddr);

    // Espera pela conexão de um cliente
    int clntSock = accept(servSock, (struct sockaddr *)&clntAddr, &clntAddrLen);
    if (clntSock < 0)
    {
        printf("accept() falhou\n");
        return;
    }

    etapaC2 c2;
    cabecalho c;
    c.estagio = ESTAGIO;
    c.len = sizeof(etapaC2) - sizeof(cabecalho);
    c.matricula = mat;
    c.psecret = 3;
    c2.num = (rand() % 10)+1;
    c2.len = (rand() % 10)+1;
    c2.secretC = 3;
    c2.completar = 'x';
    c2.c = 'a' + (char)(rand() % 26);
    //int BUFSIZE = 512;
    c2.cab = c;
    imprimeEtapaC2(&c2);

    preparaEtapaC2Enviar(&c2);
    preparaCabecalhoEnviar(&c2.cab);
    ssize_t numBytesSent = send(clntSock, &c2, sizeof(etapaC2), 0);
    if (numBytesSent < 0)
    {
        printf("send() falhou\n");
        return;
    }
    else if (numBytesSent != sizeof(etapaC2))
    {
        printf("send() enviou número inesperado de bytes\n");
        return;
    }

    printf("-------- Etapa D1 --------\n");
    preparaEtapac2Recebe(&c2);
    int sizeBuf = c2.len + sizeof(cabecalho) + 1;
    char buf[sizeBuf];
    printf("Numero de pacotes %d\n", c2.num);
    //printf("Tamanho do buf %d\n",sizeBuf);
    // Recebe mensagem do cliente

    for (int i = 0; i < c2.num; i++)
    {
        fd_set select_fds;
        struct timeval timeout;

        FD_ZERO(&select_fds);
        FD_SET(clntSock, &select_fds);

        timeout.tv_sec = 3;
        timeout.tv_usec = 0;

        if (select(clntSock + 1, &select_fds, NULL, NULL, &timeout) == 0)
        {
            printf("Tempo acabou :(\n");
            exit(0);
        }
        else
        {
            ssize_t numBytesRcvd = recv(clntSock, buf, sizeBuf, 0);
            if (numBytesRcvd < 0)
            {
                printf("recv() falhou\n");
                return;
            }

            char len[4];
            char psecret[4];
            char estagio[2];
            char matricula[3];

            int x = 0;
            for (int i = 0; i < 4; i++)
            {
                len[x] = buf[i];
                x++;
            }
            x = 0;
            for (int i = 4; i < 8; i++)
            {
                psecret[x] = buf[i];
                x++;
            }
            x = 0;
            for (int i = 8; i < 10; i++)
            {
                estagio[x] = buf[i];
                x++;
            }
            printf("Matricula ");
            for (int i = 10; i < 13; i++)
            {
                //matricula[x] = buf[i];
                //x++;
                printf("%c",buf[i]);
            }
            printf("\n");
            //printf("Matricula %s\n", matricula);
            printf("Estagio %s\n", estagio);
            printf("len %s\n", len);
            printf("Psecret %s\n", psecret);
            printf("Numero de envios %d\n", c2.num);
            printf("Dados:\n");
            for (int i = 13; i < sizeBuf; i++)
            {
                printf("%c", buf[i]);
            }
            printf("\n");
        }
    }
    printf("-------- Etapa D2 --------\n");
    etapaD2 d2;
    c.estagio = ESTAGIO;
    c.len = sizeof(etapaD2) - sizeof(cabecalho);
    imprimirCabecalho(&c);
    preparaCabecalhoEnviar(&c);
    d2.secret = 4;
    imprimirD2(&d2);
    preparaEtapaD2Enviar(&d2);
    d2.c = c;
    //sleep(1);
    numBytesSent = send(clntSock, &d2, sizeof(etapaD2), 0);
    if (numBytesSent < 0)
    {
        printf("send() falhou\n");
        return;
    }
    else if (numBytesSent != sizeof(etapaD2))
    {
        printf("send() enviou número inesperado de bytes\n");
        return;
    }

    close(clntSock); // Fecha socket cliente
    close(servSock);
}
void etapaB(etapaA2 *a2)
{
    char *portaUDP = malloc(sizeof(char *));
    int porta = a2->novaPortaUdp;
    sprintf(portaUDP, "%d", porta);
    struct addrinfo addrCriteria;
    memset(&addrCriteria, 0, sizeof(addrCriteria));
    addrCriteria.ai_family = AF_UNSPEC;
    addrCriteria.ai_flags = AI_PASSIVE;
    addrCriteria.ai_socktype = SOCK_DGRAM;
    addrCriteria.ai_protocol = IPPROTO_UDP;

    struct addrinfo *servAddr;
    int rtnVal = getaddrinfo(NULL, portaUDP, &addrCriteria, &servAddr);
    if (rtnVal != 0)
    {
        printf("getaddrinfo() falhou");
        return;
    }

    int sock = socket(servAddr->ai_family, servAddr->ai_socktype,
                      servAddr->ai_protocol);
    if (sock < 0)
    {
        printf("socket() falhou");
        return;
    }

    if (bind(sock, servAddr->ai_addr, servAddr->ai_addrlen) < 0)
    {
        printf("bind falhou");
        return;
    }

    freeaddrinfo(servAddr);
    //sleep(1);
    struct sockaddr_storage clntAddr;
    socklen_t clntAddrLen = sizeof(clntAddr);
    a2->len = a2->len + 4 + sizeof(cabecalho);
    while ((a2->len % 4) != 0)
    {
        a2->len++;
    }
    for (int i = 0; i < a2->num; i++)
    {
        printf("----------- b1 -----------\n");
        int32_t buf[(a2->len / 4) + 4];
        int tamBuf = 4 * (a2->len / 4);

        fd_set select_fds;
        struct timeval timeout;

        FD_ZERO(&select_fds);
        FD_SET(sock, &select_fds);

        timeout.tv_sec = 3;
        timeout.tv_usec = 0;

        if (select(sock + 1, &select_fds, NULL, NULL, &timeout) == 0)
        {
            printf("Tempo acabou :(\n");
            goto fim;
        }

        ssize_t numBytesRcvd = recvfrom(sock, buf, tamBuf, 0,
                                        (struct sockaddr *)&clntAddr, &clntAddrLen);
        if (numBytesRcvd < 0)
        {
            printf("recvfrom() falhou");
            goto fim;
        }
        cabecalho esperado;
        esperado.matricula = a2->c.matricula;
        esperado.len = tamBuf - sizeof(cabecalho) - 4;
        esperado.estagio = 1;
        esperado.psecret = 1;

        cabecalho c;
        c.estagio = htons(ntohl(buf[0]));
        c.len = buf[1];
        c.matricula = htons(ntohl(buf[2]));
        c.psecret = buf[3];
        preparaCabecalhoRecebe(&c);
        if (verificaCabecalho(&c, &esperado) == 0)
        {
            imprimirCabecalho(&c);
            printf("Cabecalho inesperado\n");
            imprimirCabecalho(&esperado);
            goto fim;
        }

        int32_t id = ntohl(buf[4]);
        imprimirCabecalho(&c);
        printf("ID %d\n", id);

        printf("----------- ACK -----------\n");
        ack comf;
        comf.dado = id;
        c.estagio = ESTAGIO;
        c.matricula = c.matricula;
        c.len = sizeof(ack) - sizeof(cabecalho);
        c.psecret = c.psecret;
        imprimirCabecalho(&c);
        imprimeAck(&comf);
        preparaCabecalhoEnviar(&c);
        comf.c = c;
        preparaAckEnviar(&comf);
        //sleep(1);
        ssize_t numBytesSent = sendto(sock, &comf, sizeof(ack), 0,
                                      (struct sockaddr *)&clntAddr, sizeof(clntAddr));
        if (numBytesSent < 0)
        {
            printf("sendto() falhou");
            goto fim;
        }
        else if (numBytesSent != sizeof(ack))
        {
            printf("sendto() error");
            goto fim;
        }
    }
    printf("----------- Etapa b2 -----------\n");
    etapaB2 b2;
    cabecalho c;
    c.estagio = ESTAGIO;
    c.matricula = a2->c.matricula;
    c.psecret = 1;
    c.len = sizeof(etapaB2) - sizeof(cabecalho);
    int mat = c.matricula;
    imprimirCabecalho(&c);
    preparaCabecalhoEnviar(&c);
    b2.c = c;
    b2.secret = 2;
    int tcpPorta = 0;
    srand( (unsigned)time(NULL) );
    do
    {
        tcpPorta = rand() % 50353;
    } while (tcpPorta < 49753);
    //49152-65535
    b2.tcpPorta = tcpPorta;
    imprimirB2(&b2);
    preparaEtapaB2Enviar(&b2);
    //sleep(1);
    ssize_t numBytesSent = sendto(sock, &b2, sizeof(etapaB2), 0,
                                  (struct sockaddr *)&clntAddr, sizeof(clntAddr));
    if (numBytesSent < 0)
    {
        printf("sendto() falhou");
        return;
    }
    else if (numBytesSent != sizeof(etapaB2))
    {
        printf("sendto() error");
        return;
    }
    //freeaddrinfo(servAddr);
    etapaCD(tcpPorta, mat);
fim:
    close(sock);
}

int main(int argc, char *argv[])
{
    char *portaUDP = "12235";
    srand((unsigned)time(NULL));

    struct addrinfo addrCriteria;
    memset(&addrCriteria, 0, sizeof(addrCriteria));
    addrCriteria.ai_family = AF_UNSPEC;
    addrCriteria.ai_flags = AI_PASSIVE;
    addrCriteria.ai_socktype = SOCK_DGRAM;
    addrCriteria.ai_protocol = IPPROTO_UDP;

    struct addrinfo *servAddr;
    int rtnVal = getaddrinfo(NULL, portaUDP, &addrCriteria, &servAddr);
    if (rtnVal != 0)
    {
        printf("erro no getaddrinfo");
        return 0;
    }

    int sock = socket(servAddr->ai_family, servAddr->ai_socktype,
                      servAddr->ai_protocol);
    if (sock < 0)
    {
        return 0;
        printf("erro socket");
    }
    if (bind(sock, servAddr->ai_addr, servAddr->ai_addrlen) < 0)
    {
        printf("erro bind");
        return 0;
    }

    freeaddrinfo(servAddr);
    etapaA1 *temp = malloc(sizeof(etapaA1));
    for (;;)
    {
        struct sockaddr_storage clntAddr; // Client address
        socklen_t clntAddrLen = sizeof(clntAddr);
        //Etapa A1
        ssize_t numBytesRcvd = recvfrom(sock, temp, sizeof(etapaA1), 0,
                                        (struct sockaddr *)&clntAddr, &clntAddrLen);
        if (numBytesRcvd < 0)
        {
            printf("Erro recvfrom()");
            return 0;
        }
        cabecalho c = temp->c;
        preparaCabecalhoRecebe(&c);
        printf("Recebendo...\n");
        imprimirEtapaA1(temp);

        //Etapa A2
        printf("-------- Etapa A2 --------\n");
        u_int16_t len = (rand() % 10)+1;
        u_int16_t num = (rand() % 10)+2;
        u_int16_t novaPortaUdp = 0;
        do
        {
            novaPortaUdp = rand() % 49752;
        } while (novaPortaUdp <= 49152); //tem 600 portas
        //49152-65535
        etapaA2 a2;
        cabecalho c2;
        c2.matricula = c.matricula;
        c2.estagio = ESTAGIO;
        c2.psecret = 0;
        c2.len = sizeof(etapaA2) - sizeof(cabecalho);
        imprimirCabecalho(&c2);
        a2.len = len;
        a2.novaPortaUdp = novaPortaUdp;
        a2.num = num;
        printf("NUM %d\n", num);
        printf("enviando...\n");
        impromirEtapaA2(&a2);
        a2.c = c2;
        preparaCabecalhoEnviar(&a2.c);
        preparaEtapaA2Enviar(&a2);

        ssize_t numBytesSent = sendto(sock, (struct etapaA2 *)&a2, sizeof(etapaA2), 0,
                                      (struct sockaddr *)&clntAddr, sizeof(clntAddr));
        if (numBytesSent < 0)
        {
            printf("erro sendto\n");
            return 0;
        }
        else if (numBytesSent != sizeof(etapaA2))
        {
            printf("erro sendTo,numero inesperado de bytes\n");
            return 0;
        }
        preparaCabecalhoRecebe(&a2.c);
        preparaEtapaA2Recebe(&a2);
        etapaB(&a2);
        //etapaCD();
        //return 0;
    }
}
