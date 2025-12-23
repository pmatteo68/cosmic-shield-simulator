#ifndef LOGGER_HH
#define LOGGER_HH

#include <string>

class Logger {
public:
    //static void initialize();  // To initialize log level from env variable
    static void debug(const std::string& component, const std::string& message);
    static void info(const std::string& component, const std::string& message);
    static void warn(const std::string& component, const std::string& message);
    static void error(const std::string& component, const std::string& message);
    static void setLevel(const int logLev);

private:
    static void log(const std::string& level, const std::string& component, const std::string& message, int threshold);
    static int logLevel;  // Static log level initialized at startup
};

#endif  // LOGGER_HH

