#include <iostream>
#include <WS2tcpip.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main()
{
    // Инициализация Winsock
    WSADATA wsaData;
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res != 0) {
        cerr << "WSAStartup failed: " << res << endl;
        return 1;
    }

    // Создание сокета
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        cerr << "socket failed: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    // Настройка адреса сервера
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(54000);

    // Привязка
    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "bind failed: " << WSAGetLastError() << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // Прослушивание (макс. 1 клиент в очереди)
    if (listen(listenSocket, 1) == SOCKET_ERROR) {
        cerr << "listen failed: " << WSAGetLastError() << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    cout << "TCP Server listening on port 54000...\n";

    // Принятие подключения
    sockaddr_in clientAddr{};
    int clientAddrSize = sizeof(clientAddr);
    SOCKET clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &clientAddrSize);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "accept failed: " << WSAGetLastError() << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    char clientIp[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, INET_ADDRSTRLEN);
    cout << "Client connected: " << clientIp << endl;

    // Обмен сообщениями
    char buf[1024];
    while (true) {
        ZeroMemory(buf, sizeof(buf));
        int bytesReceived = recv(clientSocket, buf, sizeof(buf) - 1, 0);

        if (bytesReceived <= 0) {
            if (bytesReceived == 0)
                cout << "Client disconnected." << endl;
            else
                cerr << "recv failed: " << WSAGetLastError() << endl;
            break;
        }

        string msg(buf);
        cout << "Received: '" << msg << "'" << endl;

        // Условие выхода
        if (msg == "quit") {
            cout << "Exit command received. Closing connection." << endl;
            send(clientSocket, "Bye", 3, 0);  
            break;
        }

        
        send(clientSocket, buf, bytesReceived, 0);
    }

    // Завершение
    closesocket(clientSocket);
    closesocket(listenSocket);
    WSACleanup();
    cout << "Server shutdown" << endl;
    return 0;
}