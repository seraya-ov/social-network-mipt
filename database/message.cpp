#include "message.h"
#include "database.h"
#include "../config/config.h"

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

    void Message::init()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            //*
            Statement drop_stmt(session);
            drop_stmt << "DROP TABLE IF EXISTS Message", now;
            //*/

            // (re)create table
            Statement create_stmt(session);
            create_stmt << "CREATE TABLE IF NOT EXISTS `Message` (`id` INT NOT NULL AUTO_INCREMENT,"
                        << "`sender_id` INT NOT NULL,"
                        << "`recipient_id` INT NOT NULL,"
                        << "`timestamp` TIMESTAMP DEFAULT NOW() ON UPDATE NOW(),"
                        << "`text` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                        << "PRIMARY KEY (`id`), FOREIGN KEY (`sender_id`) REFERENCES User(`id`) ON DELETE CASCADE, FOREIGN KEY (`recipient_id`) REFERENCES User(`id`) ON DELETE CASCADE);",
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

    Poco::JSON::Object::Ptr Message::toJSON() const
    {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", _id);
        root->set("sender_id", _sender_id);
        root->set("recipient_id", _recipient_id);
        root->set("text", _text);
        root->set("timestamp", _timestamp);

        return root;
    }

    Message Message::fromJSON(const std::string &str)
    {
        Message message;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(str);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

        message.id() = object->getValue<long>("id");
        message.sender_id() = object->getValue<long>("sender_id");
        message.recipient_id() = object->getValue<long>("recipient_id");
        message.text() = object->getValue<std::string>("text");
        message.timestamp() = object->getValue<std::time_t>("timestamp");

        return message;
    }

    Message Message::read_by_id(long id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement select(session);
            Message m;
            select << "SELECT id, sender_id, recipient_id, text, timestamp FROM Message where id=?",
                into(m._id),
                into(m._sender_id),
                into(m._recipient_id),
                into(m._text),
                into(m._timestamp),
                use(id),
                range(0, 1); //  iterate over result set one row at a time
  
            select.execute();
            Poco::Data::RecordSet rs(select);
            if (!rs.moveFirst()) throw std::logic_error("not found");

            return m;
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

    std::vector<Message> Message::read_all()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            std::vector<Message> result;
            Message m;
            select << "SELECT id, sender_id, recipient_id, timestamp, text FROM Message",
                into(m._id),
                into(m._sender_id),
                into(m._recipient_id),
                into(m._timestamp),
                into(m._text),
                range(0, 1); //  iterate over result set one row at a time

            while (!select.done())
            {
                if(select.execute())
                result.push_back(m);
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


    std::vector<std::pair<Message, std::string>> Message::search(long sender_id, long recipient_id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            std::vector<std::pair<Message, std::string>> result;
            Message m;
            std::string s;
            select << "SELECT m.id, m.sender_id, m.recipient_id, Unix_Timestamp(m.timestamp) as timestamp, text, u.login FROM (Message m left join User u on m.sender_id = u.id) where (m.sender_id = ? and m.recipient_id = ?) or (m.sender_id = ? and m.recipient_id = ?) order by timestamp",
                into(m._id),
                into(m._sender_id),
                into(m._recipient_id),
                into(m._timestamp),
                into(m._text),
                into(s),
                use(sender_id),
                use(recipient_id),
                use(recipient_id),
                use(sender_id),
                range(0, 1); //  iterate over result set one row at a time

            while (!select.done())
            {
                if(select.execute())  result.push_back({m, s});
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


    void Message::clean(long sender_id, long recipient_id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            select << "DELETE FROM Message where (sender_id = ? and recipient_id = ?) or (sender_id = ? and recipient_id = ?)",
                use(sender_id),
                use(recipient_id),
                use(recipient_id),
                use(sender_id);
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


    std::vector<database::User> Message::search_chats(long sender_id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            std::vector<database::User> result;
            database::User u;
            select << "SELECT distinct id, login FROM User where id in (select sender_id from Message where recipient_id = ? union select recipient_id from Message where sender_id = ?);",
                into(u.id()),
                into(u.login()),
                use(sender_id),
                use(sender_id),
                range(0, 1); //  iterate over result set one row at a time

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

   
    void Message::save_to_mysql()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement insert(session);

            insert << "INSERT INTO Message (sender_id, recipient_id, text) VALUES (?, ?, ?)",
                use(_sender_id),
                use(_recipient_id),
                use(_text);

            insert.execute();

            Poco::Data::Statement select(session);
            select << "SELECT LAST_INSERT_ID()",
                into(_id),
                range(0, 1); //  iterate over result set one row at a time

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

    long Message::get_id() const
    {
        return _id;
    }

    const long &Message::get_sender_id() const
    {
        return _sender_id;
    }

    const long &Message::get_recipient_id() const
    {
        return _recipient_id;
    }

    const std::time_t &Message::get_timestamp() const
    {
        return _timestamp;
    }

    const std::string &Message::get_text() const
    {
        return _text;
    }

    long &Message::id()
    {
        return _id;
    }

    long &Message::sender_id()
    {
        return _sender_id;
    }

    long &Message::recipient_id()
    {
        return _recipient_id;
    }

    std::time_t &Message::timestamp()
    {
        return _timestamp;
    }

    std::string &Message::text()
    {
        return _text;
    }

}
