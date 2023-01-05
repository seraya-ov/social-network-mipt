#ifndef USER_H
#define USER_H

#include <string>
#include <vector>
#include "Poco/JSON/Object.h"

namespace database
{
    class User{
        private:
            long _id;
            std::string _login;
            std::string _password;

        public:

            static User fromJSON(const std::string & str);

            long get_id() const;
            const std::string &get_login() const;
            const std::string &get_password() const;

            long& id();
            std::string &login();
            std::string &password();

            static void init();
            static User read_by_id(long id);
            static std::vector<User> read_all();
            static std::vector<User> search(std::string login, std::string password);
            static std::vector<long> search_id(std::string login);
            static void clean(long id);
            void save_to_mysql();

            Poco::JSON::Object::Ptr toJSON() const;

            std::string encode(std::string password, std::string login);

    };
}

#endif
