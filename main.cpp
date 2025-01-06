#include "server.h"

int main()
{
    boost::asio::io_context ioc;
    auto server = std::make_shared<HttpServer>(ioc, 8088);
    server->run(), ioc.run();
    return 0;
}