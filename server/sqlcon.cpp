#include <iostream>
#include <sqlite3.h>
#include "sqlcon.h"
#include "logger.h"
#include "server.h"
constexpr auto dbLogFileName = "sql.log";

SQLcon::SQLcon(const std::string& dbFileName)
{

}

SQLcon::~SQLcon()
{
}

bool SQLcon::initTable()
{
    constexpr auto create_table_users = "CREATE TABLE IF NOT EXISTS users ("
        "userID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT NOT NULL UNIQUE,"
        "name TEXT,"
        "surname TEXT,"
        "email TEXT NOT NULL UNIQUE,"
        "password TEXT NOT NULL"
        ");";
    constexpr auto create_table_messages = "CREATE TABLE messages ("
        "messageID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "messageContent TEXT NOT NULL,"
        "senderID INTEGER NOT NULL,"
        "recieverID INTEGER NOT NULL,"
        "FOREIGN KEY(senderID) REFERENCES users(userID),"
        "FOREIGN KEY(recieverID) REFERENCES users(userID)"
        ");";

    sqlite3* db;
    int rc = sqlite3_open("database.db", &db);

    if (rc != SQLITE_OK) {
        Logger(dbLogFileName).log(std::string("Error opening database: ") + sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }

    char* err_msg = nullptr;
    rc = sqlite3_exec(db, create_table_users, nullptr, nullptr, &err_msg);

    if (rc != SQLITE_OK) {
        Logger(dbLogFileName).log(std::string("Error creating table: ") + sqlite3_errmsg(db));
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 0;
    }
    rc = sqlite3_exec(db, create_table_messages, nullptr, nullptr, &err_msg);

    if (rc != SQLITE_OK) {
        Logger(dbLogFileName).log(std::string("Error creating table: ") + sqlite3_errmsg(db));
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 0;
    }

    sqlite3_close(db);
    return 1;
}

bool SQLcon::insertUser(const User& record)
{
    // Open database connection
    sqlite3* db;
    int rc = sqlite3_open("mydatabase.db", &db);
    if (rc != SQLITE_OK) {
        Logger(dbLogFileName).log(std::string("Error opening database: ") + sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }

    // Prepare SQL statement
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db, "INSERT INTO users (userID, username, name, surname, email, password) VALUES (?, ?, ?, ?, ?, ?)", -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        Logger(dbLogFileName).log(std::string("Error preparing SQL statement: ") + sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }

    // Bind parameters
    rc = sqlite3_bind_int(stmt, 1, 1); // userID
    rc |= sqlite3_bind_text(stmt, 2, record.username.c_str(), -1, SQLITE_STATIC); // username
    rc |= sqlite3_bind_text(stmt, 3, record.name.c_str(), -1, SQLITE_STATIC); // name
    rc |= sqlite3_bind_text(stmt, 4, record.surname.c_str(), -1, SQLITE_STATIC); // surname
    rc |= sqlite3_bind_text(stmt, 5, record.email.c_str(), -1, SQLITE_STATIC); // email
    rc |= sqlite3_bind_text(stmt, 6, record.password.c_str(), -1, SQLITE_STATIC); // password
    if (rc != SQLITE_OK) {
        Logger(dbLogFileName).log(std::string("Error binding parameters: ") + sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 0;
    }

    // Execute SQL statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        Logger(dbLogFileName).log(std::string("Error inserting users record: ") + sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 0;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 1;
}

bool SQLcon::insertMessage(const Message& record)
{
    // Open a connection to the database
    sqlite3* db;
    int rc = sqlite3_open("my_database.db", &db);
    if (rc != SQLITE_OK) {
        Logger(dbLogFileName).log(std::string("Error opening database: ") + sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }

    // Prepare an SQL statement to insert a record into the messages table
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO messages (messageContent, senderID, recieverID) VALUES (?, ?, ?)";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger(dbLogFileName).log(std::string("Error preparing SQL statement: ") + sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }

    // Bind values to the parameters in the SQL statement
    sqlite3_bind_text(stmt, 1, record.content.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, record.sender);
    sqlite3_bind_int(stmt, 3, record.reciever);
    if (rc != SQLITE_OK) {
        Logger(dbLogFileName).log(std::string("Error binding parameters: ") + sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }

    // Execute the SQL statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        Logger(dbLogFileName).log(std::string("Error inserting messages record: ") + sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 0;
    }

    // Clean up
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    //std::cout << "Record inserted successfully!" << std::endl;
    return 1;
}
