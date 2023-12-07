#include "httpServer.hpp"

int main(int argc, const char* argv[]) {
    spdlog::init_thread_pool(8192, 1);
    short unsigned port = 8080;
    std::filesystem::path pathToFile = "base.json";
    if(argc == 2) {
        if(!strcmp(argv[1], "test")) {
            std::cout << "Normal test run!" << std::endl;
            return 0;
        }
    }
    if(argc == 3) {
        port = std::stoi(std::string{argv[1]});
        pathToFile = argv[2];
    }
    net::HttpServer server(port, pathToFile);
    server.run();

    spdlog::shutdown();
    return 0;
}