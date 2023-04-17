#include <sqlite3.h>
#include <iostream>
#include "sqlcon.h"
#include "logger.h"

constexpr auto dbLogFileName = "sql.log";

SQLcon::SQLcon()
{
    sqlite3* db;
    char* err_msg = nullptr;

    const char* messages_table_sql = "CREATE TABLE messages (id INTEGER PRIMARY KEY, user_id INTEGER, content TEXT)";
    int rc = sqlite3_exec(db, messages_table_sql, nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::cerr << "Error creating messages table: " << err_msg << std::endl;
        sqlite3_free(err_msg);
        sqlite3_close(db);
    }
}

SQLcon::~SQLcon()
{
}

void SQLcon::createDB()
{
    sqlite3* db;
    char* err_msg = nullptr;

    int rc = sqlite3_open("server.db", &db);
    if (rc != SQLITE_OK) {
        Logger(dbLogFileName).log(std::string("Error opening database: ") + sqlite3_errmsg(db));
        sqlite3_free(err_msg);
        sqlite3_close(db);
    }
    
}

void SQLcon::createTable()
{
    sqlite3* db;
    int rc = sqlite3_open("database.db", &db);

    if (rc != SQLITE_OK) {
        Logger(dbLogFileName).log(std::string("Error opening database: ") + sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    const char* create_table = "CREATE TABLE IF NOT EXISTS users ("
        "userID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT NOT NULL UNIQUE,"
        "name TEXT,"
        "surname TEXT,"
        "email TEXT NOT NULL UNIQUE,"
        "password TEXT NOT NULL"
        ");";

    char* err_msg = nullptr;
    rc = sqlite3_exec(db, create_table, nullptr, nullptr, &err_msg);

    if (rc != SQLITE_OK) {
        Logger(dbLogFileName).log(std::string("Error creating table: ") + sqlite3_errmsg(db));
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return;
    }

    sqlite3_close(db);
}

void SQLcon::insertUser()
{
    sqlite3* db;
    char* err_msg = nullptr;
}

void SQLcon::insertMessage()
{

}

void SQLcon::updateUser()
{
}

void SQLcon::updateMessage()
{
}
