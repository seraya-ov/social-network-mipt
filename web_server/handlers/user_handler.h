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
    bool check_email(const std::string &email, std::string &reason)
    {
        if (email.find(' ') != std::string::npos)
        {
            reason = "EMail can't contain spaces";
            return false;
        }

        if (email.find('\t') != std::string::npos)
        {
            reason = "EMail can't contain spaces";
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
                {"chat", u.get_email()},
                {"recipient", std::to_string(u.get_id())}
            });
        }
        mstch::map context{
            {"first_name", user.get_first_name()},
            {"last_name", user.get_last_name()},
            {"id", std::to_string(user.get_id())},
            {"email", user.get_email()},
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
        else if (check_uri(uri,"/user/search"))
        {
            try
            {
                std::vector<database::User> users;
                if (form.has("email")) {
                    std::string email = form.get("email");
                    users = database::User::search(email);
                    if (users.empty()) {
                        send_not_found(response);
                    }
                }  
                else if (form.has("first_name") && form.has("last_name")) {
                    std::string fn = form.get("first_name");
                    std::string ln = form.get("last_name");
                    users = database::User::search(fn, ln);
                    if (users.size() != 1) {
                        send_not_found(response);
                    }
                }
                if (users.size()) {
                    if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST && form.has("text") && form.has("id")) {
                        Poco::Net::HTTPRequest send_request(Poco::Net::HTTPRequest::HTTP_POST, "/messages/send", Poco::Net::HTTPMessage::HTTP_1_1);
                        HTMLForm send_form;
                        send_form.set("sender", form.get("id"));
                        send_form.set("recipient", std::to_string(users[0].get_id()));
                        send_form.set("text", form.get("text"));
                        send_form.prepareSubmit(send_request);

                        Poco::Net::HTTPClientSession *session = new Poco::Net::HTTPClientSession(_host, _port);
                        send_form.write(session->sendRequest(send_request));
                        Poco::Net::HTTPResponse res;
                        session->receiveResponse(res);
                        users = {database::User::read_by_id(std::stol(form.get("id")))};
                    }
                    make_search_response(users, response);
                }
            }
            catch (...)
            {
                send_not_found(response);
                return;
            }
            return;
        }
        else if (check_uri(uri,"/user/add") && 
                 form.has("email") && request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST)
        {
            database::User user;
            if (form.has("first_name")) {
                user.first_name() = form.get("first_name");
            }
            if (form.has("last_name")) {
                user.last_name() = form.get("last_name");
            }
            user.email() = form.get("email");

            bool check_result = true;
            std::string message;
            std::string reason;

            if (!check_email(user.get_email(), reason))
            {
                check_result = false;
                message += reason;
                message += "<br>";
            }

            if (check_result)
            {
                try
                {
                    auto users = database::User::search(user.get_email());
                    if (users.empty()) {
                        user.save_to_mysql();
                    }
                    response.redirect("/user/search?email=" + user.get_email());
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