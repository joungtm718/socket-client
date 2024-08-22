#include <iostream>
#include <thread>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080

void send_messages(SOCKET sock) {
    std::string message;
    while (true) {
        std::cout << "Enter message: ";
        std::getline(std::cin, message);
        send(sock, message.c_str(), message.size(), 0);
    }
}

void receive_messages(SOCKET sock) {
    char buffer[1024] = { 0 };
    while (true) {
        int bytesReceived = recv(sock, buffer, 1024, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::cout << "Message from server: " << buffer << std::endl;
        }
        else if (bytesReceived == 0) {
            std::cout << "Connection closed" << std::endl;
            break;
        }
        else {
            std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
            break;
        }
    }
}

void run_client() {
    WSADATA wsaData;
    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        return;
    }

    // Create a socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address / Address not supported" << std::endl;
        closesocket(sock);
        WSACleanup();
        return;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return;
    }

    // 메시지를 보내는 스레드와 받는 스레드 생성
    std::thread send_thread(send_messages, sock);
    std::thread receive_thread(receive_messages, sock);

    // 스레드가 종료될 때까지 대기
    send_thread.join();
    receive_thread.join();

    // Clean up and close the socket
    closesocket(sock);
    WSACleanup();
}

int main() {
    run_client();
    return 0;
}
