#include <fstream>
#include <mutex>
#include <string>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include "proto/chat.grpc.pb.h"
// Returns the local time as a std::tm struct.
inline std::tm get_local_time(std::time_t timer)
{
    //remake//https://stackoverflow.com/a/38034148
    std::tm bt{};
#if defined(__unix__)
    localtime_r(&timer, &bt);
#elif defined(_WIN32)
    localtime_s(&bt, &timer);
#else
    static std::mutex mtx;
    std::scoped_lock lock(mtx);
    bt = *std::localtime(&timer);
#endif
    return bt;
}
// Returns the current time stamp as a string in the specified format.
inline std::string time_stamp(const std::string& fmt = "%F %T")
{
    //remake//https://stackoverflow.com/a/38034148
    auto bt = get_local_time(std::time(nullptr));
    std::ostringstream oss;
    oss << std::put_time(&bt, fmt.c_str());
    return oss.str();
}
/*Class writes timestamp(automatic) : info - msg(input value for log member function).
Avoid creating more than one instance with same filename, as doing it will remove thread safety.*/
class Logger {
public:
    explicit Logger(const std::string& filepath) : ofs_{ filepath, std::ios_base::app } {}
    ~Logger() { ofs_.close(); }

    template<typename T>
    inline void log(T&& info) {
        std::lock_guard<std::mutex> lock{ mutex_ };
        ofs_ << time_stamp() << " : " << std::forward<T>(info) << std::endl;
    }
    template<typename T>
    inline std::string read(int lineNumber) {
        std::istream file(ofs_);
        file.seekg(std::ios_base::beg);
        std::string line;

        int current_line = 1;
        while (std::getline(file, line) && current_line != line_number) {
            ++current_line;
        }
        return line;
    }

private:
    std::fstream ofs_;
    std::mutex mutex_;
};

inline void grpcLog(const std::string& path, const grpc::Status& status)
{
    std::ostringstream oss;
    oss << "[ERROR] gRPC error " << status.error_code() << ": " << status.error_message();
    if (!status.error_details().empty()) {
        oss << " (" << status.error_details() << ")";
    }
    Logger(path).log(oss.str());
    std::cout << "Error: " << status.error_code() << ": " << status.error_message() << std::endl;
    return;
}