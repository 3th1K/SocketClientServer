#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
#endif

#define BUFFER_SIZE 1024

struct UserRecord {
    char userId[20];
    char password[20];
    int accountNumber;
    char balance[100];
};

struct UserRecord *authenticateUser(const char *username, const char *password, struct UserRecord users[], int length) {
    
    for (int i = 0; i < length; i++) {
        if (strcmp(users[i].userId, username) == 0 && strcmp(users[i].password, password) == 0) {
            return &users[i]; 
        }
    }
    return NULL;
}

int createServerSocket() {
    int serverSocket;
    
    #if defined(_WIN32) || defined(_WIN64)
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            printf("[ - ] Windows socket subsystem could not be initialized. Error Code: %d. Exiting...\n", WSAGetLastError());
            exit(EXIT_FAILURE);
        }
    #endif
    
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("[ - ] Socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("[ + ] Socket successfully created.\n");
    return serverSocket;
}

void setSocketOptions(int serverSocket) {
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }
}

void bindSocket(int serverSocket, int port) {
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("[ - ] Bind failed");
        exit(EXIT_FAILURE);
    }
    printf("[ + ] Socket successfully binded.\n");
}

int listenForConnections(int serverSocket) {
    if (listen(serverSocket, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    printf("[ + ] Server listening...\n");
}

int acceptConnection(int serverSocket) {
    struct sockaddr_in clientAddress;
    #if defined(_WIN32) || defined(_WIN64)
        int addressLength = sizeof(clientAddress);
    #else
        socklen_t addressLength = sizeof(clientAddress);
    #endif
    int newSocket;

    if ((newSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &addressLength)) < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }
    printf("[ + ] Server accepted the client...\n");
    return newSocket;
}

void handleClientCommunication(int newSocket) {
    ////////// define variables
    int valRead;
    char buffer[BUFFER_SIZE] = {0};
    char message[BUFFER_SIZE] = {0};
    char userId[BUFFER_SIZE] = {0};
    char password[BUFFER_SIZE] = {0};
    int timeoutSeconds = 30;
    struct timeval timeout;
    fd_set readfds;
    timeout.tv_sec = timeoutSeconds;
    timeout.tv_usec = 0;
    int result;
    struct UserRecord users[5];
    //////// end

    //////// add data in user
    strcpy(users[0].userId, "davi0027");
    strcpy(users[0].password, "Crc51RqV");
    users[0].accountNumber = 14632873;
    strcpy(users[0].balance, "70,000");

    strcpy(users[1].userId, "jack0046");
    strcpy(users[1].password, "Cfw61RqV");
    users[1].accountNumber = 14646987;
    strcpy(users[1].balance, "74,000");

    strcpy(users[2].userId, "bhat0092");
    strcpy(users[2].password, "G6M7p8az");
    users[2].accountNumber = 14666234;
    strcpy(users[2].balance, "77,000");

    strcpy(users[3].userId, "yogi0067");
    strcpy(users[3].password, "uCh781fY");
    users[3].accountNumber = 14693456;
    strcpy(users[3].balance, "80,000");

    strcpy(users[4].userId, "anni0083");
    strcpy(users[4].password, "Pd82bG57");
    users[4].accountNumber = 14677213;
    strcpy(users[4].balance, "60,000");
    //////// end
    
    //////// send initial message after client joins
    strcpy(message, "Please join the server with valid credentials to retrieve the details");
    send(newSocket, message, strlen(message), 0);

    memset(buffer, 0, BUFFER_SIZE);
    memset(message, 0, BUFFER_SIZE);
    //////// end

    //////// handle user id input
    printf("[ + ] Waiting for %d seconds to recieve User Id...\n", timeoutSeconds);

    FD_ZERO(&readfds);
    FD_SET(newSocket, &readfds);

    result = select(newSocket + 1, &readfds, NULL, NULL, &timeout);

    if (result == 0) {
        printf("[ + ] No connection occurred within %d seconds.\n", timeoutSeconds);
        send(newSocket, "timeout_exit\n", strlen(message), 0);
        return;  
    }
    if (FD_ISSET(newSocket, &readfds)){
        valRead = recv(newSocket, buffer, BUFFER_SIZE, 0);
        if (valRead < 0) {
            perror("[ - ] Read error while reading user id");
            memset(buffer, 0, sizeof(buffer));
            exit(EXIT_FAILURE);
        } else if (valRead == 0) {
            printf("[ - ] Client disconnected.\n");
            exit(EXIT_FAILURE);
        } else {
            printf("[ + ] Received User Id: %s", buffer);
            strcpy(userId, buffer);
            memset(buffer, 0, sizeof(buffer));
        }
    }
    //////// end
    
    //////// send acknowledgement after recieving user id
    strcpy(message, "Ack");
    send(newSocket, message, strlen(message), 0);
    memset(message, 0, BUFFER_SIZE);
    //////// end

    //////// handle password input
    printf("[ + ] Waiting for %d seconds to recieve Password...\n", timeoutSeconds);

    FD_ZERO(&readfds);
    FD_SET(newSocket, &readfds);

    result = select(newSocket + 1, &readfds, NULL, NULL, &timeout);

    if (result == 0) {
        printf("[ - ] No connection occurred within %d seconds.\n", timeoutSeconds);
        send(newSocket, "timeout_exit\n", strlen(message), 0);
        return;  
    }

    if (FD_ISSET(newSocket, &readfds)){
        valRead = recv(newSocket, buffer, BUFFER_SIZE, 0);
        if (valRead < 0) {
            perror("[ - ] Read error while reading password");
            memset(buffer, 0, sizeof(buffer));
            exit(EXIT_FAILURE);
        } else if (valRead == 0) {
            printf("[ - ] Client disconnected.\n");
            exit(EXIT_FAILURE);
        } else {
            printf("[ + ] Received Password: %s", buffer);
            strcpy(password, buffer);
            memset(buffer, 0, sizeof(buffer));
        }
    }
    //////// end
    
    //////// handle user auth
    userId[strcspn(userId, "\n")] = '\0';
    password[strcspn(password, "\n")] = '\0';
    
    struct UserRecord *authenticatedUser = authenticateUser(userId, password, users, 5);
    if(authenticatedUser != NULL){
        strcpy(message, "User Validation Successfull");
        send(newSocket, message, strlen(message), 0);
        memset(message, 0, BUFFER_SIZE);
        sprintf(message, "[ INFORMATION ] Account Number: %d\n[ INFORMATION ] Balance: %s\n", authenticatedUser->accountNumber, authenticatedUser->balance);
        send(newSocket, message, strlen(message), 0);
        // memset(message, 0, BUFFER_SIZE);
        // sprintf(message, "Balance: %d\n", authenticatedUser->balance);
        // send(newSocket, message, strlen(message), 0);
        memset(message, 0, BUFFER_SIZE);
    }
    else{
        printf("[ - ] Credentials mismatch \n");
        strcpy(message, "User Validation Unsuccessfull");
        int x = send(newSocket, message, strlen(message), 0);
    }
    //////// end

    //////// handle quit promt
    while(1){
        memset(buffer, 0, sizeof(buffer));
        valRead = recv(newSocket, buffer, BUFFER_SIZE, 0);
        
        if (valRead < 0) {
            perror("[ - ] Read error while reading input");
            memset(buffer, 0, sizeof(buffer));
            exit(EXIT_FAILURE);
        } else if (valRead == 0) {
            printf("[ - ] Client disconnected.\n");
            memset(buffer, 0, sizeof(buffer));
            exit(EXIT_FAILURE);
        } else if(strcmp(buffer, "QUIT\n") != 0){
            printf("[ - ] Received : %s", buffer);
            memset(message, 0, sizeof(buffer));
            memset(buffer, 0, sizeof(buffer));
            strcpy(message, "Ok");
            send(newSocket, message, BUFFER_SIZE, 0);
        }
        else if(strcmp(buffer, "QUIT\n") == 0){
            printf("[ + ] Received : %s", buffer);
            memset(buffer, 0, sizeof(buffer));
            send(newSocket, "exit\n", BUFFER_SIZE, 0);
            break;
        }
    }
    //////// end
    
}


int main(int argc, char *argv[]) {
    int port;
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\nUsing default port : 1234\n", argv[0]);
        port = 1234;
    } else {
        port = atoi(argv[1]);
    }
    int serverSocket, newSocket;

    serverSocket = createServerSocket();
    setSocketOptions(serverSocket);
    bindSocket(serverSocket, port);
    listenForConnections(serverSocket);


    while (1) {
        printf("\n***********************************\n[ + ] Waiting for a client to connect.\n");
        newSocket = acceptConnection(serverSocket);
        printf("[ + ] A client got connected.\n");
        handleClientCommunication(newSocket);
        printf("[ - ] Closing the connection to the client.\n");
        
        #if defined(_WIN32) || defined(_WIN64)
            closesocket(newSocket);
        #else
            close(newSocket);
        #endif
    }

    #if defined(_WIN32) || defined(_WIN64)
        closesocket(serverSocket);
        WSACleanup();
    #else
        close(serverSocket);
    #endif

    return 0;
}
