# Подпроект, представляющий простое взаимодействие "один клиент - сервер"
project(poll_connections)

add_executable(poll_server
    server.cpp
)

add_executable(poll_client
    client.cpp
)