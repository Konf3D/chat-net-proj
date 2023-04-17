#include <mutex>
struct User;
struct Message;

class SQLcon
{
public:
	SQLcon(const std::string& dbFileName);
	~SQLcon();

private:
	bool initTable();
	bool insertUser(const User& record);
	bool insertMessage(const Message& record);
private:
	std::mutex mutex_;
	const std::string filename;
};

enum StatusMsg
{
	SENT = 1,
	RECIEVED = 2,
	READ = 3,
};