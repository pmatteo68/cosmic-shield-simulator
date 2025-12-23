// CsvHelper.hh
#pragma once

#include "G4AnalysisManager.hh"
#include "globals.hh"
//#include <map>
#include <mutex>
#include <string>
#include <variant>
#include <vector>

using CsvField = std::variant<std::string, G4String, char, G4int, G4double, int, double>;
using CsvFieldVector = std::vector<CsvField>;

inline CsvFieldVector makeCsvFields() {
    return {};
}

template<typename T, typename... Rest>
inline CsvFieldVector makeCsvFields(T&& first, Rest&&... rest) {
    CsvFieldVector result = { CsvField(std::forward<T>(first)) };
    CsvFieldVector tail = makeCsvFields(std::forward<Rest>(rest)...);
    result.insert(result.end(), tail.begin(), tail.end());
    return result;
}

class CsvHelper {
public:
    G4AnalysisManager* getManager();
    void openFile(const G4String& fileName);
    G4int createTuple(const G4String& tupleIdString, const G4String& tupleDesc, const std::vector<std::pair<G4String, char>>& columns);
    //void addDataField(int fldPos, G4String s);
    //void addDataField(int fldPos, G4double d);
    void addDataField(int fldPos, std::string s);
    void addDataField(int fldPos, char c);
    void addDataField(int fldPos, int i);
    //void addDataField(int fldPos, G4int i);
    void addDataField(int fldPos, double d);
    void addRecord();

    //G4int getID(const G4String& tupleIdString);
    //void addRecord(const CsvFieldVector& fields);
    void close();

private:
    //std::map<G4String, G4int> tupleNameToId;
    //std::mutex mutex;
    //G4int tupleCounter;
};
