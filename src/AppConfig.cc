#include "AppConfig.hh"
#include "nlohmann/json.hpp"
#include <fstream>
#include <stdexcept>
#include "Logger.hh"

using json = nlohmann::json;

AppConfig::AppConfig(const std::string& filename) {
    Logger::debug("DRIVER", "Opening main configuration file: " + filename);
    std::ifstream file(filename);
    if (!file.is_open()) throw std::runtime_error("Cannot open config file: " + filename);

    json data;
    file >> data;

    if (data.contains("physicsList")) {
        physicsListName = data["physicsList"].get<std::string>();
        Logger::debug("DRIVER", "physicsList configuration retrieved: " + physicsListName);
    }
}

std::string AppConfig::GetPhysicsListName() const {
    return physicsListName;
}

