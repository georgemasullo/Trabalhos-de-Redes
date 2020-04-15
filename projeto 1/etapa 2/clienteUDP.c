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
#include <unistd.h>

const uint16_t ESTAGIO = 1;
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
    int32_t dado;
} ack;
typedef struct
{
    cabecalho c;
    u_int16_t len;
    u_int16_t num;
    u_int16_t novaPortaUdp;
} etapaA2;
typedef struct
{
    cabecalho c;
    u_int32_t tcpPorta;
    u_int32_t secret;
} etapaB2;
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
    u_int32_t secret;
} etapaD2;
void preparaCabecalhoEnviar(cabecalho *c)
{
    c->len = htonl(c->len);
    c->psecret = htonl(c->psecret);
    c->estagio = htons(ESTAGIO);
    c->matricula = htons(c->matricula);
}
void preparaCabecalhoRecebe(cabecalho *c)
{
    c->estagio = ntohs(c->estagio);
    c->matricula = ntohs(c->matricula);
    c->len = ntohl(c->len);
    c->psecret = ntohl(c->psecret);
}
int verificaCabecalho(cabecalho *c, cabecalho *c2)
{
    if (c->matricula == c2->matricula && c->len == c2->len && c->estagio == c2->estagio && c->psecret == c2->psecret)
        return 1;
    return 0;
}
void preparaAckEnviar(ack *a)
{
    a->dado = htonl(a->dado);
}
void preparaAckRecebe(ack *a)
{
    a->dado = ntohl(a->dado);
}
void preparaEtapaA2Recebe(etapaA2 *a2)
{
    a2->len = ntohs(a2->len);
    a2->novaPortaUdp = ntohs(a2->novaPortaUdp);
    a2->num = ntohs(a2->num);
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
    printf("---- Dados ----\n");
    printf("Len %d\n", c2->len);
    printf("Secret %d\n", c2->secretC);
    printf("Num %d\n", c2->num);
    printf("char %c\n", c2->c);
}
void imprimeAck(ack *a)
{
    printf("-------- ACK --------\n");
    printf("ID %d\n", a->dado);
}
void etapaCD(int porta)
{
    printf("-------- Etapa C --------\n");
    char *servIP = "127.0.0.1";
    in_port_t servPort = porta;

    int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0)
    {
        printf("socket() falhou\n");
        return;
    }
    // Constrói a struct de endereço do servidor
    struct sockaddr_in servAddr;            // Endereço servidor
    memset(&servAddr, 0, sizeof(servAddr)); // Limpa a struct
    servAddr.sin_family = AF_INET;          // Família de endereços IPv4
    // Converte o endereço
    int rtnVal = inet_pton(AF_INET, servIP, &servAddr.sin_addr.s_addr);
    if (rtnVal == 0)
    {
        printf("inet_pton() falhou string de endereço inválida\n");
        return;
    }
    else if (rtnVal < 0)
    {
        printf("inet_pton() falhou");
        return;
    }
    servAddr.sin_port = htons(servPort); // Porta Servidor
    int err=connect(sock, (struct sockaddr *)&servAddr, sizeof(servAddr));
    if ( err< 0)
    {
        printf("Err %d\n",err);
        printf("connect() falhou\n");
        return;
    }

    etapaC2 *c2 = malloc(sizeof(etapaC2));
    ssize_t numBytes = recv(sock, c2, sizeof(etapaC2), 0);
    if (numBytes < 0)
    {
        printf("recv() falhou\n");
        return;
    }
    else if (numBytes == 0)
    {
        printf("recv() conexão fechada prematuramente\n");
        return;
    }
    cabecalho c = c2->cab;
    preparaCabecalhoRecebe(&c);
    preparaEtapac2Recebe(c2);
    c2->cab = c;
    imprimeEtapaC2(c2);
    imprimirCabecalho(&c);
    printf("Numero de pacotes %d\n",c2->num);
    printf("-------- Etapa D1 --------\n");
    //sleep(1);
    printf("Antees do for NUM %d\n",c2->num);
    for (int i = 0; i < c2->num; i++)
    {
        int sizeBuf = c2->len + sizeof(cabecalho)+1;
    /*while (sizeBuf % 4 != 0)
    {
        sizeBuf++;
    }*/
    char buf[sizeBuf];
    char *psecret = malloc(4);
    char estagio[2];
    char matricula[3];
    
    c.len = c2->len ;
    c.psecret = c2->secretC;
    c.estagio = ESTAGIO;
    c.matricula = c.matricula;
    imprimirCabecalho(&c);
    int a = c.len;
    printf("LEN %d\n",a);
    char *aux = malloc(4);
    int a2 = c.matricula;
    int a3 = c.psecret;
    sprintf(aux, "%d", a);
    sprintf(psecret, "%d", a3);
    sprintf(estagio, "%d", c.estagio);
    sprintf(matricula, "%d", a2);

    int x = 0;
    for (int i = 0; i < 8; i++)
    {
        if (i == 4)
            x = 0;
        if (i < 4)
        {
            buf[i] = aux[x];
        }
        else
        {
            buf[i] = psecret[x];
        }
        x++;
    }
    x = 0;
    for (int i = 8; i < 13; i++)
    {
        if (i == 10)
            x = 0;
        if (i < 10)
        {
            buf[i] = estagio[x];
        }
        else
        {
            buf[i] = matricula[x];
        }
        x++;
    }
    printf("Dados:\n");
    for (int i = 13; i < sizeBuf; i++)
    {
        buf[i] = c2->c;
        printf("%c", buf[i]);
    }
    printf("\n");

        numBytes = send(sock, buf, sizeBuf, 0);
        if (numBytes < 0)
        {
            printf("send() falhou\n");
            return;
        }
        else if (numBytes != sizeBuf)
        {
            printf("send() enviou um número inesperado de bytes");
            return;
        }
        /*
        char len[4];
        char psecret[4];
        char estagio[2];
        char *matricula = malloc(2);
        int x = 0;
        for (int i = 0; i < 8; i++)
        {
            if (i == 4)
                x = 0;
            if (i < 4)
            {
                len[x] = buf[i];
            }
            else
            {
                psecret[x] = buf[i];
            }
            x++;
        }
        x = 0;
        for (int i = 8; i < 13; i++)
        {
            if (i == 10)
                x = 0;
            if (i < 10)
            {
                estagio[x] = buf[i];
            }
            else
            {
                matricula[x] = buf[i];
            }
            x++;
        }
        printf("%s", len);
        c.len = atoi(len);
        c.estagio = atoi(estagio);
        c.matricula = atoi(matricula);
        c.psecret = atoi(psecret);

        imprimirCabecalho(&c);
        printf("Dados:\n");
        for (int i = 13; i < sizeBuf; i++)
        {
            printf("%c", buf[i]);
        }
        printf("\n");*/
    }
    etapaD2 *d2 = malloc(sizeof(etapaD2));
    numBytes = recv(sock, d2, sizeof(etapaD2), 0);
    if (numBytes < 0)
    {
        printf("recv() falhou\n");
        return;
    }
    else if (numBytes == 0)
    {
        printf("recv() conexão fechada prematuramente\n");
        return;
    }
    c = d2->c;
    printf("-------- Etapa D2 --------\n");
    preparaCabecalhoRecebe(&c);
    imprimirCabecalho(&c);
    preparaEtapaD2Recebe(d2);
    imprimirD2(d2);
    free(d2);
    free(c2);
    close(sock);
}

void etapaB(etapaA2 *a2)
{
    char *server = "127.0.0.1";
    printf("Iniciando etapa B\n");
    char *servPort = malloc(sizeof(char *));
    int aux = a2->novaPortaUdp;
    sprintf(servPort, "%d", aux);
    printf("Iniciando etapa B %s\n",servPort);
    struct addrinfo addrCriteria; // Critério para casamento de endereços
    memset(&addrCriteria, 0, sizeof(addrCriteria));
    addrCriteria.ai_family = AF_UNSPEC;     // Qualquer família de endereços
    addrCriteria.ai_socktype = SOCK_DGRAM;  // Somentes sockets datagrama
    addrCriteria.ai_protocol = IPPROTO_UDP; // Somente o protocolo UDP

    // Pega endereços
    struct addrinfo *servAddr; // Lista de endereços do servidor
    int rtnVal = getaddrinfo(server, servPort, &addrCriteria, &servAddr);
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
    a2->len = a2->len + 4 + sizeof(cabecalho);
    while ((a2->len % 4) != 0)
    {
        a2->len++;
    }
    for (int i = 0; i < a2->num; i++)
    {
        int32_t id = i;
        if (i == (a2->num - 1))
        {
            id = -1;
        }
        printf("----------- b1 -----------\n");
        cabecalho c;
        int32_t buf[(a2->len / 4) + 4];
        //printf("TAM BUF %d",(a2->len / 4)+4);
        buf[0] = ESTAGIO;
        c.estagio = ESTAGIO;
        //printf("ESTAGIO %d\n ",buf[0]);
        buf[1] = a2->len - sizeof(cabecalho) - 4;
        c.len = a2->len;
        //printf("Len %d\n ",buf[1]);
        buf[2] = a2->c.matricula;
        c.matricula = a2->c.matricula;
        //printf("Matricula %d\n ",buf[2]);
        buf[3] = 1;
        c.psecret = buf[3];
        //printf("Psecret %d\n ",buf[3]);
        buf[4] = id;
        imprimirCabecalho(&c);
        printf("ID %d\n ", buf[4]);
        for (int x = 5; x < (a2->len / 4); x++)
        {
            buf[x] = 0;
        }
        for (int x = 0; x < a2->len / 4; x++)
        {
            buf[x] = htonl(buf[x]);
        }
        //sleep(1);
        printf("----------- Envia Pacote -----------\n");
        // Envia string ao servidor
        int tamBuf = 4 * (a2->len / 4);
        ssize_t numBytes = sendto(sock, buf, tamBuf, 0,
                                  servAddr->ai_addr, servAddr->ai_addrlen);
        if (numBytes < 0)
        {
            printf("sendto() falhou");
            goto fim;
        }
        else if (numBytes != tamBuf)
        {
            printf("sendto() error");
            goto fim;
        }
        printf("----------- Recebe ACK -----------\n");
        // Recebe uma resposta
        ack *comf = malloc(sizeof(ack));
        struct sockaddr_storage fromAddr;
        socklen_t fromAddrLen = sizeof(fromAddr);
        numBytes = recvfrom(sock, comf, sizeof(ack), 0,
                            (struct sockaddr *)&fromAddr, &fromAddrLen);
        if (numBytes < 0)
        {
            printf("recvfrom() falhou");
            goto fim;
        }
        else if (numBytes != sizeof(ack))
        {
            printf("recvfrom() error");
            goto fim;
        }
        c = comf->c;
        preparaCabecalhoRecebe(&c);
        imprimirCabecalho(&c);
        preparaAckRecebe(comf);
        imprimeAck(comf);

        free(comf);
    }
    printf("Etapa b2\n");
    etapaB2 *b2 = malloc(sizeof(etapaB2));
    struct sockaddr_storage fromAddr;
    socklen_t fromAddrLen = sizeof(fromAddr);
    ssize_t numBytes = recvfrom(sock, b2, sizeof(etapaB2), 0,
                                (struct sockaddr *)&fromAddr, &fromAddrLen);
    printf("Etapa b2adndcklc\n");
    if (numBytes < 0)
    {
        printf("recvfrom() falhou");
        return;
    }
    else if (numBytes != sizeof(etapaB2))
    {
        printf("recvfrom() error");
        return;
    }
    cabecalho c = b2->c;
    preparaCabecalhoRecebe(&c);
    imprimirCabecalho(&c);
    preparaEtapaB2Recebe(b2);
    imprimirB2(b2);
    int porta = b2->tcpPorta;
    int matricula = c.matricula;
    free(b2);
    freeaddrinfo(servAddr);
    sleep(1);
    etapaCD(porta);
fim:
    close(sock);
}

int main(int argc, char const *argv[])
{
    char *server = "127.0.0.1";
    char *servPort = "12235";
    cabecalho cab;
    cab.estagio = ESTAGIO;
    cab.matricula = 124;
    cab.psecret = 0;
    char *str = "hello world\n";
    cab.len = strlen(str);
    etapaA1 a1;
    a1.c = cab;

    memcpy(&a1.dados, str, strlen(str));

    struct addrinfo addrCriteria;                   // Critério para casamento de endereços
    memset(&addrCriteria, 0, sizeof(addrCriteria)); // Limpa a struct
    addrCriteria.ai_family = AF_UNSPEC;             // Qualquer família de endereços
    // Para os campos a seguir, zero significa que não se importa
    addrCriteria.ai_socktype = SOCK_DGRAM;  // Somentes sockets datagrama
    addrCriteria.ai_protocol = IPPROTO_UDP; // Somente o protocolo UDP
                                            // Pega endereços
    struct addrinfo *servAddr;              // Lista de endereços do servidor
    int rtnVal = getaddrinfo(server, servPort, &addrCriteria, &servAddr);
    if (rtnVal != 0)
    {
        printf("erro getaddinfo\n");
        return 1;
    }

    // Cria um socket datagram/UDP
    int sock = socket(servAddr->ai_family, servAddr->ai_socktype,
                      servAddr->ai_protocol); // Descritor de socket para o cliente
    if (sock < 0)
    {
        printf("erro socket\n");
        return 0;
    }
    //Etapa A1
    preparaCabecalhoEnviar(&a1.c);
    // Envia string ao servidor
    ssize_t numBytes = sendto(sock, (struct etapaA1 *)&a1, (sizeof(etapaA1)), 0, servAddr->ai_addr, servAddr->ai_addrlen);
    if (numBytes < 0)
    {
        printf("erro sendto\n");
        return 0;
    }
    else if (numBytes != sizeof(etapaA1))
    {
        printf("erro sendTo,numero inesperado de bytes\n");
        return 0;
    }
    printf("Enviando...");
    imprimirEtapaA1(&a1);
    struct sockaddr_storage fromAddr; // Endereço de origem do servidor

    etapaA2 *temp = malloc(sizeof(etapaA2));
    socklen_t fromAddrLen = sizeof(fromAddr);
    numBytes = recvfrom(sock, temp, sizeof(etapaA1), 0,
                        (struct sockaddr *)&fromAddr, &fromAddrLen);
    if (numBytes < 0)
    {
        printf("recvfrom() falhou\n");
        return 0;
    }
    else if (numBytes != sizeof(etapaA2))
    {
        printf("recvfrom() error\n");
        return 0;
    }
    printf("Recebeu...\n");
    cabecalho c = temp->c;
    preparaEtapaA2Recebe(temp);
    preparaCabecalhoRecebe(&c);
    printf("-------- Etapa A2 --------\n");
    imprimirCabecalho(&c);
    temp->c = c;
    impromirEtapaA2(temp);
    printf("temp %d\n", temp->c.matricula);
    freeaddrinfo(servAddr);
    etapaB(temp);
    //etapaCD();
    free(temp);
    close(sock);

    return 0;
}
