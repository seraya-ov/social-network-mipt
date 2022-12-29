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
            std::string _first_name;
            std::string _last_name;
            std::string _email;

        public:

            static User fromJSON(const std::string & str);

            long get_id() const;
            const std::string &get_first_name() const;
            const std::string &get_last_name() const;
            const std::string &get_email() const;

            long& id();
            std::string &first_name();
            std::string &last_name();
            std::string &email();

            static void init();
            static User read_by_id(long id);
            static std::vector<User> read_all();
            static std::vector<User> search(std::string first_name, std::string last_name);
            static std::vector<User> search(std::string email);
            static void clean(long id);
            void save_to_mysql();

            Poco::JSON::Object::Ptr toJSON() const;

    };
}

#endif
