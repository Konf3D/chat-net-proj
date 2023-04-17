class SQLcon
{
public:
	SQLcon();
	~SQLcon();

private:
	void createDB();
	void createTable();
	void insertUser();
	void insertMessage();
	void updateUser();
	void updateMessage();
private:
	const std::string filename;

};
