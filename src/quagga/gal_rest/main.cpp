
#include <memory>
#include <iostream>
#include <thread>

#include <unistd.h>
#include <sys/types.h>

#include "activity/http/http_server.hpp"

#include "io/tcp_io.hpp"

#include "util/pugixml.hpp"
#include "util/string.hpp"

#include "real_device.hpp"
#include "log.hpp"
#include "application.hpp"

int main(int argc, char* argv[])
{
    std::cout << "main phase 1.." << std::endl;
    if (argc < 3)

    {
        std::cerr << "Usage: gal_rest <address> <port>" << std::endl;

        return -1;
    }

    auto address = argv[1];
	auto port = tnt::stoi(argv[2]);
    std::cout << "main phase 2.." << std::endl;

    if (port < 1024 && geteuid() != 0)
    {
        std::cerr << "The program requires elevated privileges to run" << std::endl;

        return -1;
    }
    std::cout << "main phase 3.." << std::endl;

	tnt::Log::level(tnt::LogLevel::All);

    std::thread server_thread([&] ()
    {
        std::cout << "inside thread server_thread routine" << std::endl;
        auto ep = std::make_shared<tnt::io::TcpIOServer>(tnt::ip::SocketAddress(address, port));
        tnt::activity::HttpServer server;
        server(ep, "");
    });
    std::cout << "main phase 4.." << std::endl;

    std::thread device_thread([]()
    {
        std::cout << "inside device_thread routine" << std::endl;
        RealDevice()();
    });

    tnt::Application::run();
        std::cout << "final phase.." << std::endl;
	tnt::Log::info(colors::blue, "Bye...");

    server_thread.join();
    device_thread.join();
}

