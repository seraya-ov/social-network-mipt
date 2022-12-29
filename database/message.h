#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <vector>
#include "Poco/JSON/Object.h"
#include "user.h"

namespace database
{
    class Message{
        private:
            long _id;
            long _sender_id;
            long _recipient_id;
            std::time_t _timestamp;
            std::string _text;

        public:

            static Message fromJSON(const std::string & str);

            long get_id() const;
            const long &get_sender_id() const;
            const long &get_recipient_id() const;
            const std::time_t &get_timestamp() const;
            const std::string &get_text() const;

            long& id();
            long& sender_id();
            long& recipient_id();
            std::time_t &timestamp();
            std::string &text();

            static void init();
            static Message read_by_id(long id);
            static std::vector<Message> read_all();
            static std::vector<std::pair<Message, std::string>> search(long sender_id, long recipient_id);
            static void clean(long sender_id, long recipient_id);
            static std::vector<database::User> search_chats(long sender_id);
            void save_to_mysql();

            Poco::JSON::Object::Ptr toJSON() const;

    };
}

#endif