#include "user.h"
#include "database.h"
#include "../config/config.h"

#include <Poco/Crypto/CipherKey.h>
#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

#include <sstream>
#include <exception>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

namespace database
{
    std::string User::encode(std::string password, std::string login) {
        if (login.size()) {}
        return password; //TODO: implement
    }

    void User::init()
    {
        try
        {

            Poco::Data::Session session = database::Database::get().create_session();
            //*
            Statement drop_stmt(session);
            drop_stmt << "DROP TABLE IF EXISTS Message, User", now;
            //*/

            // (re)create table
            Statement create_stmt(session);
            create_stmt << "CREATE TABLE IF NOT EXISTS `User` (`id` INT NOT NULL AUTO_INCREMENT,"
                        << "`login` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                        << "`password` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                        << "PRIMARY KEY (`id`), UNIQUE (`login`));",
                now;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    Poco::JSON::Object::Ptr User::toJSON() const
    {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", _id);
        root->set("login", _login);
        root->set("password", _password);

        return root;
    }

    User User::fromJSON(const std::string &str)
    {
        User user;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(str);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

        user.id() = object->getValue<long>("id");
        user.login() = object->getValue<std::string>("login");
        user.password() = user.encode(object->getValue<std::string>("password"), user.get_login());

        return user;
    }

    User User::read_by_id(long id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement select(session);
            User u;
            select << "SELECT id, login, password FROM User where id=?",
                into(u._id),
                into(u._login),
                into(u._password),
                use(id),
                range(0, 1);
  
            select.execute();
            Poco::Data::RecordSet rs(select);
            if (!rs.moveFirst()) throw std::logic_error("not found");

            return u;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    std::vector<User> User::read_all()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            std::vector<User> result;
            User u;
            select << "SELECT id, login, password FROM User",
                into(u._id),
                into(u._login),
                into(u._password),
                range(0, 1);

            while (!select.done())
            {
                if(select.execute())
                result.push_back(u);
            }
            return result;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    std::vector<User> User::search(std::string login, std::string password)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            std::vector<User> result;
            User u;
            std::string e_password = u.encode(password, login);
            select << "SELECT id, login, password FROM User where login = ? and password = ? limit 1",
                into(u._id),
                into(u._login),
                into(u._password),
                use(login),
                use(e_password),
                range(0, 1);

            while (!select.done())
            {
                if(select.execute())  result.push_back(u);
            }
            return result;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    std::vector<long> User::search_id(std::string login)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            std::vector<long> result;
            long u;
            select << "SELECT id FROM User where login = ? limit 1",
                into(u),
                use(login),
                range(0, 1);

            while (!select.done())
            {
                if(select.execute())  result.push_back(u);
            }
            return result;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }


    void User::clean(long id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            select << "DELETE FROM User where id = ?",
                use(id);
                
            select.execute();
            return;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

   
    void User::save_to_mysql()
    {

        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement insert(session);
            std::string e_password = this->encode(_password, _login);

            insert << "INSERT INTO User (login,password) VALUES(?, ?)",
                use(_login),
                use(e_password);

            insert.execute();

            Poco::Data::Statement select(session);
            select << "SELECT LAST_INSERT_ID()",
                into(_id),
                range(0, 1);

            if (!select.done())
            {
                select.execute();
            }
            std::cout << "inserted:" << _id << std::endl;
        }
        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    long User::get_id() const
    {
        return _id;
    }

    const std::string &User::get_login() const
    {
        return _login;
    }

    const std::string &User::get_password() const
    {
        return _password;
    }

    long &User::id()
    {
        return _id;
    }

    std::string &User::login()
    {
        return _login;
    }

    std::string &User::password()
    {
        return _password;
    }

}
