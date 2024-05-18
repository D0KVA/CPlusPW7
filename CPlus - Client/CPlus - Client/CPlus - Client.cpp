#define WIN32_LEAN_AND_MEAN  // Определяет макрос для исключения редко используемых заголовков Windows, ускоряя компиляцию и уменьшая размер конечного исполняемого файла.

#include <Windows.h>        // Подключение заголовочного файла для работы с API Windows.
#include <iostream>         // Подключение заголовочного файла для ввода/вывода данных.
#include <WinSock2.h>       // Подключение заголовочного файла для работы с сокетами.
#include <WS2tcpip.h>       // Подключение заголовочного файла для дополнительных функций работы с TCP/IP.

using namespace std;

int main() {
    WSADATA wsaData;         // Структура для хранения информации о реализации Windows Sockets.
    ADDRINFO hints;          // Структура для указания критериев фильтрации для функции getaddrinfo.
    ADDRINFO* addrResult;    // Указатель для хранения списка адресов, возвращенных функцией getaddrinfo.
    SOCKET ConnectSocket = INVALID_SOCKET; // Переменная для хранения сокета.
    char recvBuffer[512];    // Буфер для приема данных.

    const char* sendBuffer1 = "Привет от первого клиента";  // Сообщение для отправки.
    const char* sendBuffer2 = "Привет от второго клиента";  // Второе сообщение для отправки.

    // Инициализация библиотеки Winsock.
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cout << "WSAStartup failed with result: " << result << endl;
        return 1;
    }

    // Обнуление структуры hints и установка критериев фильтрации.
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;       // Семейство адресов IPv4.
    hints.ai_socktype = SOCK_STREAM; // Тип сокета - потоковый.
    hints.ai_protocol = IPPROTO_TCP; // Протокол - TCP.

    // Получение адреса сервера.
    result = getaddrinfo("localhost", "666", &hints, &addrResult);
    if (result != 0) {
        cout << "getaddrinfo failed with error: " << result << endl;
        freeaddrinfo(addrResult);   // Освобождение памяти, выделенной для списка адресов.
        WSACleanup();               // Завершение работы с Winsock.
        return 1;
    }

    // Создание сокета.
    ConnectSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET) {
        cout << "Socket creation failed" << endl;
        freeaddrinfo(addrResult);   // Освобождение памяти, выделенной для списка адресов.
        WSACleanup();               // Завершение работы с Winsock.
        return 1;
    }

    // Подключение к серверу.
    result = connect(ConnectSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (result == SOCKET_ERROR) {
        cout << "Unable to connect to server" << endl;
        closesocket(ConnectSocket); // Закрытие сокета.
        ConnectSocket = INVALID_SOCKET;
        freeaddrinfo(addrResult);   // Освобождение памяти, выделенной для списка адресов.
        WSACleanup();               // Завершение работы с Winsock.
        return 1;
    }

    // Отправка первого сообщения на сервер.
    result = send(ConnectSocket, sendBuffer1, (int)strlen(sendBuffer1), 0);
    if (result == SOCKET_ERROR) {
        cout << "Send failed, error: " << result << endl;
        closesocket(ConnectSocket); // Закрытие сокета.
        freeaddrinfo(addrResult);   // Освобождение памяти, выделенной для списка адресов.
        WSACleanup();               // Завершение работы с Winsock.
        return 1;
    }
    cout << "Sent: " << result << " bytes" << endl;

    // Отправка второго сообщения на сервер.
    result = send(ConnectSocket, sendBuffer2, (int)strlen(sendBuffer2), 0);
    if (result == SOCKET_ERROR) {
        cout << "Send failed, error: " << result << endl;
        closesocket(ConnectSocket); // Закрытие сокета.
        freeaddrinfo(addrResult);   // Освобождение памяти, выделенной для списка адресов.
        WSACleanup();               // Завершение работы с Winsock.
        return 1;
    }
    cout << "Sent: " << result << " bytes" << endl;

    // Завершение отправки данных.
    result = shutdown(ConnectSocket, SD_SEND);
    if (result == SOCKET_ERROR) {
        cout << "Shutdown failed, error: " << result << endl;
        closesocket(ConnectSocket); // Закрытие сокета.
        freeaddrinfo(addrResult);   // Освобождение памяти, выделенной для списка адресов.
        WSACleanup();               // Завершение работы с Winsock.
        return 1;
    }

    // Получение данных от сервера.
    do {
        ZeroMemory(recvBuffer, 512); // Обнуление буфера приема.
        result = recv(ConnectSocket, recvBuffer, 512, 0);
        if (result > 0) {
            cout << "Received " << result << " bytes" << endl;
            cout << "Received data: " << recvBuffer << endl;
        }
        else if (result == 0) {
            cout << "Connection closed" << endl;
        }
        else {
            cout << "Recv failed, error: " << WSAGetLastError() << endl;
        }
    } while (result > 0);

    // Закрытие сокета и освобождение ресурсов.
    closesocket(ConnectSocket);
    freeaddrinfo(addrResult);
    WSACleanup();
    return 0;
}