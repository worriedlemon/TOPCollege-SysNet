// Это реализация сервера. Для него необходимо:
// - создать файловый дескриптор сокета
// - закрепить сетевой адрес за сокетом
// - начать прослушивание сокета
// - принять соединение
// - читать/писать данные по сокету принятного соединения

#include <network_common.hpp>
#include <polling.hpp>

#include <iostream>
#include <cstring>
#include <vector>

int main()
{
    Startup();

    // Создаем файловый дескриптор сокета:
    // - AF_INET означает "Address Family - internet IPv4"
    // - SOCK_STREAM - это потоковый сокет, для взаимодействия по сети будет использоваться
    //                 протокол транспортного уровня TCP (выполняется контроль передачи)
    // - 0 - протокол выбирается автоматически, однако можно указать что-либо конкретное
    socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        std::cerr << "Error creating socket: " << std::strerror( errno ) << std::endl;
        return 1;
    }

    // Задаем адрес сервера, по которому будет закреплен сокет 
    // Например, 127.0.0.1:12345
    // По хорошему, этот адрес должен быть не петлей, но для теста сойдет
    std::string address = "127.0.0.1";
    short port = 12345;

    // Этими действиями мы преобразуем адрес к нужному внутреннему представлению
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    // Обратите внимание на функцию htons - она расшифровывается как
    // Host TO Network Short - перевести порядок следования байт в числе типа short из хостового в сетевой
    // Сетевой порядок следования байт - это Big Endian, а на хосте с архитектурой x86 - Little Endian.
    addr.sin_port = htons(port);
    inet_pton(AF_INET, address.c_str(), &addr.sin_addr);

    // Закрепляем сокет сервера за переданным адресом
    int rv = bind(sock, reinterpret_cast< sockaddr* >( &addr ), sizeof(sockaddr_in));
    if (rv == -1)
    {
        std::cerr << "Error binding socket: " << std::strerror( errno ) << std::endl;
        close(sock); // При любой ошибке и дальше нужно сначала закрыть сокет, а потом выйти
        return 1;
    }

    // Прослушиваем сокет на наличие ожидающих обработки подключений
    // Второй аргумент здесь - это максимальное количество ожидаемых подключений в очереди
    rv = listen(sock, 1);
    if (rv == -1)
    {
        std::cerr << "Error listening socket: " << std::strerror( errno ) << std::endl;
        close(sock);
        return 1;
    }

    std::cout << "Started listening connections on " << address << ":" << port << std::endl;

    std::vector<pollfd_t> pollStruct
    {
        {
            .fd = sock,
            .events = POLLIN,
            .revents = 0,
        }
    };

    char data[64];
    while (true)
    {
        int n = poll(pollStruct.data(), pollStruct.size(), -1);
        if (n > 0)
        {
            if (pollStruct[0].revents & POLLIN)
            {
                sockaddr_in clientAddress{ .sin_family = AF_INET, .sin_port = 0, .sin_addr = {}, .sin_zero = {}};
                socklen_t socklen = 0;

                // Принимаем подключение, если оно есть, а если его нет - просто блокируемся и ждем
                // Два следующих аргумента нужны для сохранения адреса подключившегося клиента, мы их не используем
                socket_t client = accept(sock, reinterpret_cast<sockaddr*>(&clientAddress), &socklen);
                if (client == INVALID_SOCKET)
                {
                    std::cerr << "Error accepting client: " << std::strerror( errno ) << std::endl;
                    close(sock);
                    return 1;
                }
                std::cout << "Connected!" << std::endl;
                pollStruct.push_back(pollfd{ .fd = client, .events = POLLIN, .revents = 0 });
            }
            else
            {
                for (size_t i = 1; i < pollStruct.size(); ++i)
                {
                    if (pollStruct[i].revents & POLLIN)
                    {
                        // Принимаем данные с клиента с помощью recv
                        // В Unix для тех же целей можно использовать read()
                        ssize_t bytes = recv(pollStruct[i].fd, data, static_cast<netsize_t>(sizeof(data)), 0);
                        if (bytes <= 0)
                        {
                            pollStruct[i].fd = -1;
                            continue;
                        }

                        std::cout.write(data, bytes);
                        std::cout << std::endl;
                    }
                    else if (pollStruct[i].revents & POLLNVAL)
                    {
                        pollStruct[i].fd = -1;
                    }
                }
            }
        }
        else
        {
            std::cerr << "poll interrupted" << std::endl;
            break;
        }
    }

    // Закрываем сокет, вместе с ним соединение
    close(sock);

    Cleanup();
    return 0;
}