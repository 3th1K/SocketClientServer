#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <netdb.h>
#endif

#define BUFFER_SIZE 1024

int createClientSocket() {
    int clientSocket;

    #if defined(_WIN32) || defined(_WIN64)
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            printf("[ - ] Error: Windows socket subsystem could not be initialized. Error Code: %d. Exiting...\n", WSAGetLastError());
            exit(EXIT_FAILURE);
        }
    #endif

    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("[ - ] Socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket successfully created.\n");
    return clientSocket;
}

int connectToServer(int clientSocket, const char *serverIP, int port) {
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);

    #if defined(_WIN32) || defined(_WIN64)
        serverAddress.sin_addr.s_addr = inet_addr(serverIP);
    #else
        struct hostent *host = gethostbyname(serverIp);
        if (host == NULL) {
            perror("[ - ] Host not found");
            exit(EXIT_FAILURE);
        }
        serverAddress.sin_addr = *((struct in_addr *)host->h_addr);
    #endif

    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("[ - ] Connection failed");
        exit(EXIT_FAILURE);
    }
    printf("[ + ] Connected to the server.\n");
    return clientSocket;
}

void communicateWithServer(int clientSocket) {
    ////// define variables
    int valRead;
    int valSend;
    char buffer[BUFFER_SIZE] = {0};
    char message[BUFFER_SIZE] = {0};
    time_t start_time, end_time;
    double timeoutSeconds = 30.00;
    ////// end
    
    ////// start loop
    while (1) {
       
        ////// recieve initial message from server
        memset(buffer, 0, BUFFER_SIZE);
        memset(message, 0, BUFFER_SIZE);

        valRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (valRead < 0) {
            perror("[ - ] Read error");
            memset(buffer, 0, sizeof(buffer));
            break; 
        } else if (valRead == 0) {
            
            printf("[ - ] Client disconnected.\n");
            break; 
        } else {
            
            printf("[ + ] Received: %s\n", buffer);
            memset(buffer, 0, sizeof(buffer));
        }
        ////// end

        ////// handle sending user id
        start_time = time(NULL);
        printf("[ + ] User Id: ");
        fgets(message, BUFFER_SIZE, stdin);
        end_time = time(NULL);
        double time_difference = difftime(end_time, start_time);
        if(time_difference<timeoutSeconds){

            valSend = send(clientSocket, message, strlen(message), 0);
            if (valSend < 0) {
                perror("[ - ] Send error");
                memset(message, 0, sizeof(message));
                break;
            }
            else if (valSend == 0) {
                printf("[ - ] Send error server disconnected.\n");
                break; 
            }

        }
        valRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (valRead < 0) {
            perror("[ - ] Read error");
            memset(buffer, 0, sizeof(buffer));
            break;
        } 
        else if (valRead == 0) {
            printf("[ - ] Client disconnected.\n");
            break; 
        }
        else if (strcmp(buffer, "timeout_exit\n") == 0) {
            printf("[ - ] Server requested to close the connection.\n");
            break; 
        }
        else if(strcmp(buffer, "Ack") == 0){ 
            printf("[ + ] Acknowledgement Recieved\n");
            memset(buffer, 0, sizeof(buffer));
        }
        else{
            printf("[ - ] Acknowledgement Was Not Recieved %s\n", buffer);
            break;
        }
        ////// end

        ////// handle sending password
        memset(message, 0, sizeof(message));
        start_time = time(NULL);
        printf("[ + ] Password: ");
        fgets(message, BUFFER_SIZE, stdin);
        end_time = time(NULL);
        time_difference = difftime(end_time, start_time);
        if(time_difference<timeoutSeconds){
            valSend = send(clientSocket, message, strlen(message), 0);
            if (valSend < 0) {
                perror("[ - ] Send error");
                memset(message, 0, sizeof(message));
                break;
            }
            else if (valSend == 0) {
                printf("[ - ] Send error server disconnected.\n");
                break; 
            }

        }
        valRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (valRead < 0) {
            perror("[ - ] Read error");
            memset(buffer, 0, sizeof(buffer));
            break;
        } 
        else if (valRead == 0) {
            printf("[ - ] Client disconnected.\n");
            break; 
        }
        else if (strcmp(buffer, "timeout_exit\n") == 0) {
            printf("[ - ] Server requested to close the connection.\n");
            break; 
        }
        ////// end

        ////// user validation
        printf("[ + ] Server : %s\n", buffer);
        if(strcmp(buffer, "User Validation Unsuccessfull") != 0){
            memset(buffer, 0, BUFFER_SIZE);
            valRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            printf("%s", buffer);
        }
        ////// end

        ////// handle quit
        while(1){
            printf("Type `QUIT` to close the connection : ");
            fgets(message, BUFFER_SIZE, stdin);

            valSend = send(clientSocket, message, strlen(message), 0);
            memset(message, 0, BUFFER_SIZE);
            if (valSend < 0) {
                perror("[ - ] Send error");
                break;
            }
            
            memset(buffer, 0, BUFFER_SIZE);
            valRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (valRead == 0) {
                printf("[ - ] Server closed the connection.\n");
                break; 
            } else if (valRead < 0) {
                perror("[ - ] Receive error");
                break;
            }

            if (strcmp(buffer, "exit\n") == 0) {
                printf("[ - ] Server requested to close the connection.\n");
                break;
            }
        }
        ////// end
    }
}



int main(int argc, char *argv[]) {
    const char *serverIP;
    int port;
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\nUsing default ip : port [ 127.0.0.1 : 1234 ]\n", argv[0]);
        serverIP = "127.0.0.1";
        port = 1234;
        //exit(EXIT_FAILURE);
    }
    else{
        serverIP = argv[1];
        port = atoi(argv[2]);
    }
    
    int clientSocket;

    clientSocket = createClientSocket();
    clientSocket = connectToServer(clientSocket, serverIP, port);
    communicateWithServer(clientSocket);

    #if defined(_WIN32) || defined(_WIN64)
        closesocket(clientSocket);
        WSACleanup();
    #else
        close(clientSocket);
    #endif

    return 0;
}
