#ifndef HTTPREQUESTFACTORY_H
#define HTTPREQUESTFACTORY_H

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Timestamp.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/Exception.h"
#include "Poco/ThreadPool.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include <iostream>

using Poco::Net::ServerSocket;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::HTTPServerParams;
using Poco::Timestamp;
using Poco::DateTimeFormatter;
using Poco::DateTimeFormat;
using Poco::ThreadPool;
using Poco::Util::ServerApplication;
using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::OptionCallback;
using Poco::Util::HelpFormatter;

#include "handlers/user_handler.h"
#include "handlers/message_handler.h"


static bool startsWith(const std::string& str, const std::string& prefix)
{
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

class DefaultHandler : public HTTPRequestHandler
{
public:
    DefaultHandler(const std::string &format) : _format(format)
    {
    }

    void handleRequest(HTTPServerRequest &request,
                       HTTPServerResponse &response)
    {
        if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
            response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
            response.setChunkedTransferEncoding(true);
            response.setContentType("text/html");
            std::ifstream t("./../templates/index.html");
            std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
            std::ostream &ostr = response.send();
            ostr << str;
            response.send();
        }
        else {
            response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
            std::ostream &ostr = response.send();
            ostr << "request error";
            response.send(); 
        }
    }
private:
    std::string _format;
};


class HTTPRequestFactory: public HTTPRequestHandlerFactory
{
public:
    HTTPRequestFactory(const std::string &format, const std::string &host, const unsigned short port) : _format(format), _host(host), _port(port)
    {
    }

    HTTPRequestHandler* createRequestHandler(
        const HTTPServerRequest& request)
    {

        if (startsWith(request.getURI(),"/user/")) 
            return new UserHandler(_format, _host, _port);
        if (startsWith(request.getURI(),"/messages/")) 
            return new MessageHandler(_format);
        else {
            return new DefaultHandler(_format);
        }
        return 0;
    }

private:
    std::string _format;
    std::string _host;
    unsigned short _port;
};

#endif
