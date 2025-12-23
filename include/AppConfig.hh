#ifndef APPCONFIG_HH
#define APPCONFIG_HH

#include <string>

class AppConfig {
public:
    explicit AppConfig(const std::string& filename);
    std::string GetPhysicsListName() const;

private:
    std::string physicsListName = "QGSP_BIC_AllHP";
};

#endif
