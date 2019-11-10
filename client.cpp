#include <iostream>
#include <vector>
#include <memory>
#include <array>
#include <string>
#include <boost/asio.hpp> // подключаем кроссплатформенные библиотеки для сетевой подсистемы

using namespace boost::asio;

void client(std::string_view host, const int port) //функция получает хостнаме и порт и запускает клиента
{
    try
    {
        io_context context;                  //создаем объект для ввода вывода
        ip::tcp::socket tcp_socket(context); // создаем сокет протокола TCP
        ip::tcp::resolver resolver(context); //объект отменяет все асинхронные операции, ожидающие разрешения.
        connect(tcp_socket, resolver.resolve({host.data(), std::to_string(port)})); //Конектимся по хостнейму и порту
        while (true)
        {
            std::cout << "biometric number or string [1-9]: "; //предлогаем ввести номера или строку не более 9 байт.
            std::string line;
            std::cin >> line;                               //считываем
            if(std::cin.fail() || line.length() < 1 || line.length() > 9) break; //проверяем на условия задачи

            // auto request = line;
            tcp_socket.write_some(buffer(line, line.length()));  //записываем в буфер и отправляем

            std::array<char, 9> reply;
            auto reply_length = tcp_socket.read_some(buffer(reply, reply.size())); //получаем ответ от сервера

            std::cout << "biometric scaner reply is: ";
            std::cout.write(reply.data(), reply_length); //распечатываем ответ из сервера
            std::cout << std::endl;
        }
    }
    catch(const std::exception& e) //если что ловим исключение
    {
        std::cerr << e.what() << '\n';
    }
    
}

int main(int argc, char const *argv[])
{
    std::string_view host = argv[1];
    int port = std::stoi( argv[2] );
    client(host, port);
    return 0;
}
