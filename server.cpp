#include <iostream>
#include <vector>
#include <memory>
#include <array>
#include <string>
#include <random>
#include <set>
#include <charconv>
#include <boost/asio.hpp>
#include <boost/uuid/uuid.hpp>            
#include <boost/uuid/uuid_generators.hpp> 
#include <boost/uuid/uuid_io.hpp>   


class biometric_scanner
{
    private:
        std::set<std::pair<char, char>> set_compare;
        std::unique_ptr<char[]> array;
    public:
        biometric_scanner() {
            this->set_biometric_standard();
        }
        virtual ~biometric_scanner(){}

        void constract(const std::string& ar0)
        {
            if(ar0.length() > 9 && ar0.length() < 1) throw std::out_of_range("arrays error");
            
            for (size_t i = 0; i < ar0.length(); i++)
            {
                set_compare.insert(std::make_pair(array[i], ar0[i]));
            }
        }
        std::string get_biometric_result()
        {
            std::random_device dre;
            std::mt19937 gen(dre());
            std::uniform_int_distribution<int> di(0,1);
            return di(gen) == 0 ? "true" : "false";
        }
        void print_biometric()
        {
            for(const auto& [x, y] : set_compare) std::cout << "(scan: " << x << ", stand: " << y << "); ";
            std::cout << std::endl;
        }
        void uuid() const
        {
            boost::uuids::uuid uuid = boost::uuids::random_generator()();
            std::cout << uuid << std::endl;
        }
        
    private:
        void set_biometric_standard()
        {
            array = std::make_unique<char[]>(9);
            for (size_t i = 0; i < 9; i++)
            {
                std::string s = std::to_string(i);
                array[i] = *s.c_str();
            }
        }
};

class sensor
{
    public:
        sensor(){}
        std::string get_sensor_data()
        {
            std::random_device dre;
            std::mt19937 gen(dre());
            std::uniform_int_distribution<int> di(0,1);
            return di(gen) == 0 ? "Yes" : "No";
        }
        virtual ~sensor(){}
};

// template<typename B = biometric_scanner, typename S = sensor>
class session : public std::enable_shared_from_this<session>
{
    private:
        std::array<char, 9> array;
        boost::asio::ip::tcp::socket tcp_socket;
        
    public:
        session(boost::asio::ip::tcp::socket socket):tcp_socket(std::move(socket)) {}
        virtual ~session(){}
    
        void read() 
        {
            auto self(shared_from_this());
            tcp_socket.async_read_some(
                boost::asio::buffer(array, 9),
                [this, self](const std::error_code error, const size_t length){
                    if(!error)
                    {
                        auto number = std::string(array.data(), length);
                        biometric_scanner bio_scnr;
                        bio_scnr.constract(number);
                        sensor snsr;
                        auto result = bio_scnr.get_biometric_result();
                        bio_scnr.uuid();
                        bio_scnr.print_biometric();
                        std::cout << " Biometric scanner: " << result << ";\t sensor: " << snsr.get_sensor_data() << std::endl;
                        write(result);
                    }
                });
        }
    private:
        void write(std::string_view response) 
        {
            auto self(shared_from_this());
            tcp_socket.async_write_some(
                boost::asio::buffer(response.data(), response.length()),
                [this, self](const std::error_code ec, const size_t length){
                    if(!ec)
                        read();
                });
        }
};
// template<typename B = biometric_scanner, typename S = sensor>
class server 
{
    private:
        boost::asio::ip::tcp::acceptor tcp_acceptor;
        boost::asio::ip::tcp::socket tcp_socket;
        // B _bio_scnr;
        // S _snsr;
    public:
        server(boost::asio::io_context& context, const int port/* B& bio_scnr, S& snsr*/):tcp_acceptor(context, 
            boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
            tcp_socket(context)/*, _bio_scnr(std::move(bio_scnr)), _snsr(std::move(snsr))*/
            {
                std::cout << "Server running on port: " << port << std::endl;
                accept();
            }
        virtual ~server(){}
    private:
        void accept()
        {
            tcp_acceptor.async_accept(tcp_socket, 
            [this](const std::error_code error){
                if(!error)
                    std::make_shared<session>(std::move(tcp_socket))->read();
                accept();
            });
        }
};


int main(int argc, char const *argv[])
{
    int port = std::stoi( argv[1] );
    biometric_scanner bio_scnr;
    sensor snsr;
        try
        {
            boost::asio::io_context context;
            server srv(context, port);
            context.run();
            
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    return 0;
}
