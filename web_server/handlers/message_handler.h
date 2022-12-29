#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

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

class MessageHandler : public HTTPRequestHandler
{
private:
    void send_not_found(HTTPServerResponse &response) {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
        std::ostream &ostr = response.send();
        ostr << "{ \"result\": false , \"reason\": \"not found\" }";
    }

    std::string render_response(const std::vector<std::pair<database::Message, std::string>>& chat, const database::User& user_sender, const database::User& user_recipient) {
        std::ifstream t("./../templates/chat.html");
        std::string view((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

        std::string chat_view{"<p>{{from}} {{when}} {{text}}"};
        mstch::array messages;
        for (auto p: chat) {
            messages.push_back(mstch::map{
                {"from", p.second},
                {"when", std::string(ctime(&p.first.get_timestamp()))},
                {"text", p.first.get_text()}
            });
        }
        mstch::map context{
            {"email", user_recipient.get_email()},
            {"id", std::to_string(user_sender.get_id())},
            {"rec_id", std::to_string(user_recipient.get_id())},
            {"messages", messages}
        };
        return mstch::render(view, context, {{"message", chat_view}});
    }

    void make_search_response(const std::vector<std::pair<database::Message, std::string>>& chat, const database::User& user_sender, const database::User& user_recipient, HTTPServerResponse &response) {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setChunkedTransferEncoding(true);
        response.setContentType("text/html");
        std::ostream &ostr = response.send();
        ostr << render_response(chat, user_sender, user_recipient);
    }

    bool check_uri(const std::string &str,const std::string & prefix) {
        return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
    }

public:
    MessageHandler(const std::string &format) : _format(format)
    {
    }

    void handleRequest(HTTPServerRequest &request,
                       HTTPServerResponse &response)
    {
        HTMLForm form(request, request.stream());
        std::string uri = request.getURI();
        if (check_uri(uri,"/messages/get") && form.has("id"))
        {
            long id = atol(form.get("id").c_str());
            try
            {
                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                std::ostream &ostr = response.send();
                database::Message result = database::Message::read_by_id(id);
                Poco::JSON::Stringifier::stringify(result.toJSON(), ostr);
                return;
            }
            catch (...)
            {
                send_not_found(response);
                return;
            }
        }
        else if (check_uri(uri,"/messages/chat"))
        {
            try
            {
                if (form.has("sender") && form.has("recipient")) {
                    long sender = std::stol(form.get("sender"));
                    long recipient = std::stol(form.get("recipient"));
                    auto user_sender = database::User::read_by_id(sender);
                    auto user_recipient = database::User::read_by_id(recipient);
                    auto chat = database::Message::search(sender, recipient);
                    make_search_response(chat, user_sender, user_recipient, response);
                }
            }
            catch (...)
            {
                send_not_found(response);
                return;
            }
            return;
        }
        else if (check_uri(uri,"/messages/send") && 
                 form.has("sender") && form.has("recipient") && form.has("text") && request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST)
        {
            database::Message message;
            message.sender_id() = std::stol(form.get("sender"));
            message.recipient_id() = std::stol(form.get("recipient"));
            message.text() = form.get("text");
            try
            {
                message.save_to_mysql();
                response.redirect("/messages/chat?sender=" + form.get("sender") + "&recipient=" + form.get("recipient"));
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
};
#endif // !AUTHORHANDLER_H