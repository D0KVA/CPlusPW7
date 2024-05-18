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
    SOCKET ListenSocket = INVALID_SOCKET; // Переменная для хранения слушающего сокета.
    SOCKET ConnectSocket = INVALID_SOCKET; // Переменная для хранения сокета соединения.
    char recvBuffer[512];    // Буфер для приема данных.

    const char* sendBuffer = "Привет от сервера";  // Сообщение для отправки клиенту.

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
    hints.ai_flags = AI_PASSIVE;     // Флаг для использования адреса для прослушивающего сокета.

    // Получение адреса сервера.
    result = getaddrinfo(NULL, "666", &hints, &addrResult);
    if (result != 0) {
        cout << "getaddrinfo failed with error: " << result << endl;
        freeaddrinfo(addrResult);   // Освобождение памяти, выделенной для списка адресов.
        WSACleanup();               // Завершение работы с Winsock.
        return 1;
    }

    // Создание слушающего сокета.
    ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        cout << "Socket creation failed" << endl;
        freeaddrinfo(addrResult);   // Освобождение памяти, выделенной для списка адресов.
        WSACleanup();               // Завершение работы с Winsock.
        return 1;
    }

    // Привязка сокета к адресу.
    result = bind(ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (result == SOCKET_ERROR) {
        cout << "Bind failed, error: " << result << endl;
        closesocket(ListenSocket); // Закрытие сокета.
        freeaddrinfo(addrResult);   // Освобождение памяти, выделенной для списка адресов.
        WSACleanup();               // Завершение работы с Winsock.
        return 1;
    }

    // Начало прослушивания на сокете.
    result = listen(ListenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        cout << "Listen failed, error: " << result << endl;
        closesocket(ListenSocket); // Закрытие сокета.
        freeaddrinfo(addrResult);   // Освобождение памяти, выделенной для списка адресов.
        WSACleanup();               // Завершение работы с Winsock.
        return 1;
    }

    // Ожидание подключения клиента.
    ConnectSocket = accept(ListenSocket, NULL, NULL);
    if (ConnectSocket == INVALID_SOCKET) {
        cout << "Accept failed, error: " << WSAGetLastError() << endl;
        closesocket(ListenSocket); // Закрытие слушающего сокета.
        freeaddrinfo(addrResult);   // Освобождение памяти, выделенной для списка адресов.
        WSACleanup();               // Завершение работы с Winsock.
        return 1;
    }

    closesocket(ListenSocket); // Закрытие слушающего сокета, так как он больше не нужен.

    // Обработка соединения с клиентом.
    do {
        ZeroMemory(recvBuffer, 512); // Обнуление буфера приема.
        result = recv(ConnectSocket, recvBuffer, 512, 0);
        if (result > 0) {
            cout << "Received " << result << " bytes" << endl;
            cout << "Received data: " << recvBuffer << endl;

            // Отправка данных клиенту.
            result = send(ConnectSocket, sendBuffer, (int)strlen(sendBuffer), 0);
            if (result == SOCKET_ERROR) {
                cout << "Send failed, error: " << result << endl;
                closesocket(ConnectSocket); // Закрытие сокета соединения.
                freeaddrinfo(addrResult);   // Освобождение памяти, выделенной для списка адресов.
                WSACleanup();               // Завершение работы с Winsock.
                return 1;
            }
        }
        else if (result == 0) {
            cout << "Connection closing" << endl;
        }
        else {
            cout << "Recv failed, error: " << WSAGetLastError() << endl;
            closesocket(ConnectSocket); // Закрытие сокета соединения.
            freeaddrinfo(addrResult);   // Освобождение памяти, выделенной для списка адресов.
            WSACleanup();               // Завершение работы с Winsock.
            return 1;
        }
    } while (result > 0);

    // Завершение отправки данных и закрытие соединения.
    result = shutdown(ConnectSocket, SD_SEND);
    if (result == SOCKET_ERROR) {
        cout << "Shutdown failed, error: " << result << endl;
        closesocket(ConnectSocket); // Закрытие сокета соединения.
        freeaddrinfo(addrResult);   // Освобождение памяти, выделенной для списка адресов.
        WSACleanup();               // Завершение работы с Winsock.
        return 1;
    }

    closesocket(ConnectSocket); // Закрытие сокета соединения.
    freeaddrinfo(addrResult);   // Освобождение памяти, выделенной для списка адресов.
    WSACleanup();               // Завершение работы с Winsock.
    return 0;
}