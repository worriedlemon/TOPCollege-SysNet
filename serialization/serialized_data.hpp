#pragma once

#ifndef SERIALIZED_DATA_HPP
#define SERIALIZED_DATA_HPP

#include <string>
#include <iostream>
#include <network_common.hpp>

class Data
{
public:
    Data(std::string line)
        : line_(std::move(line))
    {}

    void SendTo(socket_t sock)
    {
        size_t size = line_.size();
        static_assert(sizeof(size) == 8);

        // Отправка и принятие данных в сетевой инфраструктуре происходит в порядке байт от старшего к младшему (Big-Endian)
        // Для разной длины данных используются свои функции конвертации:
        // 16 байт  - htons/ntohs   (типы данных short, int16_t и их беззнаковые эквиваленты)
        // 32 байта - htonl/ntohl   (типы данных int, int32_t и их беззнаковые эквиваленты)
        // 64 байта - htonll/ntohll (типы данных long, long long, size_t, int64_t и их беззнаковые эквиваленты)
        size_t netsize = htonll(size);
        ssize_t bytes = send(sock, reinterpret_cast<char*>(&netsize), static_cast<netsize_t>(sizeof(netsize)), 0 );
        if (bytes <= 0)
        {
            throw std::exception{};
        }

        bytes = send(sock, line_.c_str(), size, 0 );
        if (bytes <= 0)
        {
            throw std::exception{};
        }
    }

    static Data ReceiveFrom(socket_t sock)
    {
        size_t netsize;
        ssize_t bytes = recv(sock, reinterpret_cast<char*>(&netsize), static_cast<netsize_t>(sizeof(netsize)), 0 );
        if (bytes <= 0)
        {
            throw std::exception{};
        }

        size_t size = ntohll(netsize);

        std::string line(size, '\0');
        bytes = recv(sock, line.data(), size, 0 );
        if (bytes <= 0)
        {
            throw std::exception{};
        }

        return Data(line);
    }

    friend std::ostream& operator<<(std::ostream& s, const Data& data)
    {
        return s << data.line_;
    }

private:
    std::string line_;
};

#endif // SERIALIZED_DATA_HPP