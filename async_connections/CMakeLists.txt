# Подпроект, представляющий простое взаимодействие "один клиент - сервер"
project(async_connections)

add_executable(async_server
    server.cpp
)

add_executable(async_client
    client.cpp
)