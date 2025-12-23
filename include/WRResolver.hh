#ifndef WR_RESOLVER_HH
#define WR_RESOLVER_HH

#include "globals.hh"
#include <vector>
#include <optional>

struct WRRule {
    G4int ruleId;
    G4String particName;
    std::optional<G4double> enLow;
    std::optional<G4double> enHigh;
    G4double wr;
};

class WRResolver {
public:
    WRResolver(const G4String& wmfilepath);

    G4double ResolveWR(const G4String& particleName, G4double energy) const;

private:
    std::vector<WRRule> fWRRules;
    G4double fDefaultWr = 5.0;
    void LoadFromFile(const G4String& wmfilepath);
};

#endif

