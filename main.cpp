// #include <Poco/Net/SocketAddress.h>
// #include <Poco/Net/StreamSocket.h>
// #include <Poco/Net/SocketStream.h>
// #include <Poco/StreamCopier.h>
// #include <iostream>
// #include <string>

// int main() {
//    Poco::Net::SocketAddress sa("ya.ru", 80);
//    // Poco::Net::SocketAddress sa("localhost", 8080);
//    Poco::Net::StreamSocket socket(sa);
//    Poco::Net::SocketStream str(socket);
//    str << "GET / HTTP/1.1\r\n\r\n";
//    str.flush();
//    socket.shutdownSend();
//    Poco::StreamCopier::copyStream64(str, std::cout);
//    // std::cout << "ok" << std::endl;
//    return 0;
// }

#include "web_server/http_web_server.h"

int main(int argc, char*argv[]) 
{
    HTTPWebServer app;
    return app.run(argc, argv);
}