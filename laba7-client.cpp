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
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "socket failed: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    // Настройка адреса сервера
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    // Подключение
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "connect failed: " << WSAGetLastError() << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "Connected to server.\n";

    string msg;
    while (true) {
        cout << "Enter message (or 'quit' to exit): ";
        getline(cin, msg);  // используем getline, чтобы поддерживать пробелы

        // Отправка
        int bytesSent = send(clientSocket, msg.c_str(), (int)msg.size(), 0);
        if (bytesSent == SOCKET_ERROR) {
            cerr << "send failed: " << WSAGetLastError() << endl;
            break;
        }

        // Условие выхода 
        if (msg == "quit") {
            cout << "Exit command sent. Waiting for server response..." << endl;
            break;
        }

        // Получение ответа
        char buf[1024];
        ZeroMemory(buf, sizeof(buf));
        int bytesReceived = recv(clientSocket, buf, sizeof(buf) - 1, 0);
        if (bytesReceived <= 0) {
            if (bytesReceived == 0)
                cout << "Server closed connection." << endl;
            else
                cerr << "recv failed: " << WSAGetLastError() << endl;
            break;
        }

        cout << "Server reply: '" << string(buf, bytesReceived) << "'" << endl;
    }

    // Завершение
    closesocket(clientSocket);
    WSACleanup();
    cout << "Client shutdown." << endl;
    return 0;
}