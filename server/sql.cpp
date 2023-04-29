// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include <filesystem>
#include "sql.h"
#include "logger.h"
constexpr auto dbLogFileName = "sql.log";

SQLcon::SQLcon(const std::string& dbFileName)
{
    if (!std::filesystem::exists(dbFileName))
        std::ofstream(dbFileName).close();
    int rc = sqlite3_open(dbFileName.c_str(), &db);

    initTable();

    if (rc != SQLITE_OK) {
        Logger(dbLogFileName).log(std::string("Error opening database: ") + sqlite3_errmsg(db));
        sqlite3_close(db);
        throw;
    }
}

SQLcon::~SQLcon()
{
    sqlite3_free(err_msg);
    sqlite3_close(db);
}

bool SQLcon::initTable()
{
    constexpr auto create_table_users = "CREATE TABLE IF NOT EXISTS users ("
        "userID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT NOT NULL UNIQUE,"
        "name TEXT,"
        "surname TEXT,"
        "emailname TEXT NOT NULL,"
        "emaildomain TEXT NOT NULL,"
        "password TEXT NOT NULL"
        ");";
    constexpr auto create_table_messages = "CREATE TABLE messages ("
        "messageID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "messageContent TEXT NOT NULL,"
        "senderID INTEGER NOT NULL,"
        "recieverID INTEGER NOT NULL,"
        "status INTEGER NOT NULL,"
        "FOREIGN KEY(senderID) REFERENCES users(userID),"
        "FOREIGN KEY(recieverID) REFERENCES users(userID)"
        ");";
    constexpr auto create_table_tokens = "CREATE TABLE tokens ("
        "token TEXT PRIMARY KEY,"
        "userID INTEGER,"
        "date DATE,"
        "FOREIGN KEY(userID) REFERENCES users(userID)"
        ");";

    rc = sqlite3_exec(db, create_table_users, nullptr, nullptr, &err_msg);

    if (rc != SQLITE_OK) {
        Logger(dbLogFileName).log(std::string("Error creating table: ") + sqlite3_errmsg(db));
        sqlite3_free(err_msg);
        return 0;
    }
    rc = sqlite3_exec(db, create_table_messages, nullptr, nullptr, &err_msg);

    if (rc != SQLITE_OK) {
        Logger(dbLogFileName).log(std::string("Error creating table: ") + sqlite3_errmsg(db));
        sqlite3_free(err_msg);
        return 0;
    }
    rc = sqlite3_exec(db, create_table_tokens, nullptr, nullptr, &err_msg);

    if (rc != SQLITE_OK) {
        Logger(dbLogFileName).log(std::string("Error creating table: ") + sqlite3_errmsg(db));
        sqlite3_free(err_msg);
        return 0;
    }

    return 1;
}

bool SQLcon::insertUser(const User& record)
{
    // Prepare SQL statement
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db, "INSERT INTO users (username, name, surname, emailname, emaildomain, password) VALUES (?, ?, ?, ?, ?, ?)", -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        Logger(dbLogFileName).log(std::string("Error preparing SQL statement: ") + sqlite3_errmsg(db));
        return 0;
    }

    // Bind parameters
    rc |= sqlite3_bind_text(stmt, 1, record.username.c_str(), -1, SQLITE_STATIC); // username
    rc |= sqlite3_bind_text(stmt, 2, record.name.c_str(), -1, SQLITE_STATIC); // name
    rc |= sqlite3_bind_text(stmt, 3, record.surname.c_str(), -1, SQLITE_STATIC); // surname
    rc |= sqlite3_bind_text(stmt, 4, record.emailname.c_str(), -1, SQLITE_STATIC); // emaildomain
    rc |= sqlite3_bind_text(stmt, 5, record.emaildomain.c_str(), -1, SQLITE_STATIC); // emaildomain
    rc |= sqlite3_bind_text(stmt, 6, record.password.c_str(), -1, SQLITE_STATIC); // password
    if (rc != SQLITE_OK) {
        Logger(dbLogFileName).log(std::string("Error binding parameters: ") + sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 0;
    }

    // Execute SQL statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        Logger(dbLogFileName).log(std::string("Error inserting users record: ") + sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 0;
    }

    sqlite3_finalize(stmt);
    return 1;
}

bool SQLcon::insertMessage(const Message& record)
{

    // Prepare an SQL statement to insert a record into the messages table
    sqlite3_stmt* stmt;
    std::string sql = "INSERT INTO messages (messageContent, senderID, recieverID, status) VALUES (?, ?, ?, ?)";
    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger(dbLogFileName).log(std::string("Error preparing SQL statement: ") + sqlite3_errmsg(db));
        return 0;
    }

    // Bind values to the parameters in the SQL statement
    sqlite3_bind_text(stmt, 1, record.content.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, record.sender);
    sqlite3_bind_int(stmt, 3, record.reciever);
    sqlite3_bind_int(stmt, 4, StatusMsg::SENT);
    if (rc != SQLITE_OK) {
        Logger(dbLogFileName).log(std::string("Error binding parameters: ") + sqlite3_errmsg(db));
        return 0;
    }

    // Execute the SQL statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        Logger(dbLogFileName).log(std::string("Error inserting messages record: ") + sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 0;
    }

    // Clean up
    sqlite3_finalize(stmt);

    return 1;
}

bool SQLcon::insertToken(const std::string& token, int userID)
{
    // create insert statement
    std::string insert_sql = "INSERT INTO tokens (token, userID, date) VALUES (?, ?, ?)";
    sqlite3_stmt* stmt;
    const std::string date = time_stamp();

    rc = sqlite3_prepare_v2(db, insert_sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger(dbLogFileName).log(std::string("Error preparing SQL statement: ") + sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, userID);
    sqlite3_bind_text(stmt, 3, date.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        Logger(dbLogFileName).log(std::string("Error inserting tokens record: ") + sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 0;
    }

    sqlite3_finalize(stmt);
    return false;
}

std::vector<Message> SQLcon::getMessages(int userID)
{
    std::vector<Message> msg;
    std::string query = "SELECT messageContent, senderID, recieverID FROM messages WHERE (senderID = "
        + std::to_string(userID) + " OR recieverID = " + std::to_string(userID) + ") AND status = 1 ;";
    int rc = sqlite3_exec(db, query.c_str(), callbackMsg, &msg, &err_msg);
    if (rc != SQLITE_OK) {
        Logger(dbLogFileName).log(std::string("Error executing query: ") + sqlite3_errmsg(db));
        sqlite3_free(err_msg);
    }
    Logger(dbLogFileName).log(std::string("Message query executed succesfully"));
    // The messages vector now contains all the messages in the message table
    return msg;
}

std::unique_ptr<User> SQLcon::getUser(int userID)
{
    std::string sql = "SELECT * FROM users WHERE UserID = ? LIMIT 1;";
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger(dbLogFileName).log(std::string("Error preparing statement: ") + sqlite3_errmsg(db));
        return nullptr;
    }

    rc = sqlite3_bind_int(stmt, 1, userID);
    if (rc != SQLITE_OK) {
        Logger(dbLogFileName).log(std::string("Error binding parameter : ") + sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return nullptr;
    }
    rc = sqlite3_bind_int(stmt, 1, userID);
    if (rc != SQLITE_OK) {
        Logger(dbLogFileName).log(std::string("Error binding parameter: ") + sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return nullptr;
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        std::unique_ptr<User> user(new User);
        user->username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user->name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user->surname = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        user->emailname = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        user->emaildomain = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        user->password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        Logger(dbLogFileName).log(std::string("User found by ID: ") + std::to_string(userID));
        return user;
    }
    Logger(dbLogFileName).log(std::string("User not found by ID: ") + std::to_string(userID));
    return nullptr;

}

int SQLcon::getUser(const std::string& emailname, const std::string& emaildomain)
{
    if (emaildomain.empty() || emailname.empty())
        return 0;
    sqlite3_stmt* stmt;
    std::string query = "SELECT userID FROM users WHERE emailname = ? AND emaildomain = ?";
    rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger(dbLogFileName).log(std::string("Error preparing statement: ") + sqlite3_errmsg(db));
    }
    rc = sqlite3_bind_text(stmt, 1, emailname.c_str(), -1, SQLITE_TRANSIENT);
    rc = sqlite3_bind_text(stmt, 2, emaildomain.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        Logger(dbLogFileName).log(std::string("Error binding parameter : ") + sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
    }

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        return sqlite3_column_int(stmt, 0);
    }
    else if (rc == SQLITE_DONE) {
        Logger(dbLogFileName).log(std::string("No user found with email = ") + emailname + "@" + emaildomain);
        return 0;
    }
    Logger(dbLogFileName).log(std::string("Error executing statement: ") + sqlite3_errmsg(db));
    return 0;
}

int SQLcon::getUser(const std::string& username)
{
    if (username.empty())
        return 0;
    sqlite3_stmt* stmt;
    std::string sql = "SELECT userID FROM users WHERE username = ?";
    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger(dbLogFileName).log(std::string("Error preparing statement: ") + sqlite3_errmsg(db));
    }
    rc = sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        Logger(dbLogFileName).log(std::string("Error binding parameter: ") + sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
    }
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        return sqlite3_column_int(stmt, 0);
    }
    else if (rc == SQLITE_DONE) {
        Logger(dbLogFileName).log(std::string("No user found with username = ") + username);
        return 0;
    }
    Logger(dbLogFileName).log(std::string("Error executing statement: ") + sqlite3_errmsg(db));
    return 0;
}

int SQLcon::getTokenUser(const std::string& token)
{
    std::string sql = "SELECT userID FROM Tokens WHERE token = ?;";
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        Logger(dbLogFileName).log(std::string("Error preparing statement: ") + sqlite3_errmsg(db));
        return 0;
    }

    rc = sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        Logger(dbLogFileName).log(std::string("Error binding parameter: ") + sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 0;
    }
    
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        Logger(dbLogFileName).log(std::string("UserID token found = ") + std::to_string(sqlite3_column_int(stmt, 0)));
        return sqlite3_column_int(stmt, 0);
    }
    else if (rc == SQLITE_DONE) {
        Logger(dbLogFileName).log(std::string("No record found for token") + token);
        return 0;
    }
    
    Logger(dbLogFileName).log(std::string("Error executing statement: ") + sqlite3_errmsg(db));

    sqlite3_finalize(stmt);
    return 0;
}
