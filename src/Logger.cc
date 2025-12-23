#include "Logger.hh"
#include "G4ios.hh"
#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <chrono>
#include <cstdlib>
#include <chrono>
#include <thread>
#include "G4Threading.hh"

//Levels are the G4Logging values
//EMERGENCY: 1
//ALERT: 2
//CRITICAL: 3
//ERROR: 4
//WARNING: 5
//INFO: 6
//DEBUG: 7
//TRACE: 8
int Logger::logLevel = 6;  // Default log level

//int Logger::logLevel = [] {
//    const char* envVar = std::getenv("CSS_LOG_LEVEL");
//    if (!envVar) return 6;  // Default to INFO level if not set
//    int level = std::atoi(envVar);
//    return (level >= 0 && level <= 8) ? level : 6;  // Clamp to valid range, default to INFO
//}();

//void Logger::initialize() {
//    const char* logLevelEnv = std::getenv("CSS_LOG_LEVEL");
//    if (logLevelEnv) {
//        logLevel = std::stoi(logLevelEnv);
//    }
//}

//std::string Logger::getTimestamp() {
//    using namespace std::chrono;
//
//    auto now = system_clock::now();
//    auto now_c = system_clock::to_time_t(now);
//    auto millisec = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
//
//    std::tm timeinfo;
//    localtime_r(&now_c, &timeinfo);
//
//    std::stringstream timestampStream;
//    timestampStream << std::put_time(&timeinfo, "%H:%M:%S");  // hh:mm:ss
//    timestampStream << ":" << std::setw(3) << std::setfill('0') << millisec.count();  // :SSS
//
//    return timestampStream.str();
//}

void Logger::setLevel(const int logLev)
{
  logLevel = (logLev >= 0 && logLev <= 8) ? logLev : 6; // Clamp to valid range, default to INFO
}

void Logger::log(const std::string& level, const std::string& component, const std::string& message, int threshold) {
    //std::xout << "[LOGGERDEBUG] logLevel=" << logLevel << ", threshold=" << threshold << ", level=" << level << std::endl;

    if (logLevel < threshold) return;  // Skip logging if current log level is below threshold
    //if (logLevel < threshold) return;

    //auto now = std::chrono::system_clock::now();
    //auto millisec = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    //auto time_t_now = std::chrono::system_clock::to_time_t(now);
    //std::tm* tm_now = std::localtime(&time_t_now);
    //std::ostringstream oss;
    //oss << "[" << std::put_time(tm_now, "%H:%M:%S") << ":" << std::setw(3) << std::setfill('0') << millisec.count() << "][" << level << "][" << component << "] " << message;

    auto now = std::chrono::system_clock::now();
    auto millisec = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm* tm_now = std::localtime(&time_t_now);

    std::ostringstream oss;
    oss << "[" << std::put_time(tm_now, "%y%m%d-") << std::put_time(tm_now, "%H:%M:%S") << ":" << std::setw(3) << std::setfill('0') << millisec.count() << "][" << std::this_thread::get_id() << "][" << G4Threading::G4GetThreadId() << "][" << level << "][" << component << "] " << message;

    // Get current timestamp
    //auto now = std::chrono::system_clock::now();
    //auto time_t_now = std::chrono::system_clock::to_time_t(now);
    //std::tm* tm_now = std::localtime(&time_t_now);

    //std::ostringstream oss;
    //oss << "[" << std::put_time(tm_now, "%y%m%d-%H%M%S") << "][" << level << "][" << component << "] " << message;

    //std::coXut << oss.str() << std::endl;
    G4cout << oss.str() << G4endl;
}

void Logger::debug(const std::string& component, const std::string& message) {
    log("DEBUG", component, message, 7);  // Only log if logLevel >= 7
}

void Logger::info(const std::string& component, const std::string& message) {
    log("INFO", component, message, 6);   // Only log if logLevel >= 6
}

void Logger::warn(const std::string& component, const std::string& message) {
    log("WARNING", component, message, 5);  // Only log if logLevel >= 5
}

void Logger::error(const std::string& component, const std::string& message) {
    log("ERROR", component, message, 4);  // Only log if logLevel >= 4
}

