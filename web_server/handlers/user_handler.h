#ifndef USERHANDLER_H
#define USERHANDLER_H

#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTMLForm.h"
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
#include <fstream>
#include <mstch/mstch.hpp>

using Poco::DateTimeFormat;
using Poco::DateTimeFormatter;
using Poco::ThreadPool;
using Poco::Timestamp;
using Poco::Net::HTMLForm;
using Poco::Net::HTTPClientSession;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerParams;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::NameValueCollection;
using Poco::Net::ServerSocket;
using Poco::Util::Application;
using Poco::Util::HelpFormatter;
using Poco::Util::Option;
using Poco::Util::OptionCallback;
using Poco::Util::OptionSet;
using Poco::Util::ServerApplication;

#include "../../database/user.h"
#include "../../database/message.h"

class UserHandler : public HTTPRequestHandler
{
private:
    bool check_login(const std::string &login, std::string &reason)
    {
        if (login.find(' ') != std::string::npos)
        {
            reason = "Login can't contain spaces";
            return false;
        }

        if (login.find('\t') != std::string::npos)
        {
            reason = "Login can't contain spaces";
            return false;
        }

        return true;
    };

    void send_not_found(HTTPServerResponse &response) {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
        std::ostream &ostr = response.send();
        ostr << "{ \"result\": false , \"reason\": \"not found\" }";
    }

    std::string render_response(const std::vector<database::User> &users, const database::User& user) {
        std::ifstream t("./../templates/feed.html");
        std::string view((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

        std::string chat_view{"<p><a href='/messages/chat?sender=" + 
                                std::to_string(user.get_id()) + "&recipient={{recipient}}'>{{chat}}</a>"};
        mstch::array recipients;
        for (database::User u: users) {
            recipients.push_back(mstch::map{
                {"chat", u.get_login()},
                {"recipient", std::to_string(u.get_id())}
            });
        }
        mstch::map context{
            {"id", std::to_string(user.get_id())},
            {"login", user.get_login()},
            {"chats", recipients}
        };
        return mstch::render(view, context, {{"chat", chat_view}});
    }

    void make_search_response(const std::vector<database::User>& users, HTTPServerResponse &response) {
        auto chats = database::Message::search_chats(users[0].get_id());
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setChunkedTransferEncoding(true);
        response.setContentType("text/html");
        std::ostream &ostr = response.send();
        ostr << render_response(chats, users[0]);
    }

    bool check_uri(const std::string &str,const std::string & prefix) {
        return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
    }

public:
    UserHandler(const std::string &format, const std::string &host, const unsigned short port) : _format(format), _host(host), _port(port)
    {
    }

    void handleRequest(HTTPServerRequest &request,
                       HTTPServerResponse &response)
    {
        HTMLForm form(request, request.stream());
        std::string uri = request.getURI();
        
        if (check_uri(uri,"/user/get") && form.has("id") && request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET)
        {
            long id = atol(form.get("id").c_str());
            try
            {
                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();
                database::User result = database::User::read_by_id(id);
                Poco::JSON::Stringifier::stringify(result.toJSON(), ostr);
                return;
            }
            catch (...)
            {
                send_not_found(response);
                return;
            }
        }
        else if (check_uri(uri,"/user/add") && 
                 form.has("login") && form.has("password") && request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST)
        {
            database::User user;
            user.login() = form.get("login");
            user.password() = form.get("password");

            bool check_result = true;
            std::string message;
            std::string reason;

            if (!check_login(user.get_login(), reason))
            {
                check_result = false;
                message += reason;
                message += "<br>";
            }

            if (check_result)
            {
                try
                {
                    auto users = database::User::search(user.get_login(), user.get_password());
                    if (users.empty()) {
                        user.save_to_mysql();
                        make_search_response({user}, response);
                        return;
                    }
                    make_search_response(users, response);
                    return;
                }
                catch (std::exception &e)
                {
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
                    std::ostream &ostr = response.send();
                    ostr << "database error" << ' ' << e.what();
                    response.send();
                    return;
                }
            }
            else
            {
                response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
                std::ostream &ostr = response.send();
                ostr << message;
                response.send();
                return;
            }
        }
        else if (check_uri(uri,"/user/delete") && 
                 form.has("id") && request.getMethod() == Poco::Net::HTTPRequest::HTTP_DELETE)
        {
            try
            {
                long id = std::stol(form.get("id"));
                database::User::clean(id);
                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();
                ostr << id;
                return;
            }
            catch (...)
            {
                response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
                std::ostream &ostr = response.send();
                ostr << "database error";
                response.send();
                return;
            }
        }
        response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
        std::ostream &ostr = response.send();
        ostr << "request error";
        response.send();
    }

private:
    std::string _format;
    std::string _host;
    unsigned short _port;
};
#endif // !AUTHORHANDLER_H