// CsvServices.hh
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

class CsvServices {
public:
    static G4AnalysisManager* getManager();
    static void openFile(const G4String& fileName);
    static G4int createTuple(const G4String& tupleIdString, const G4String& tupleDesc, const std::vector<std::pair<G4String, char>>& columns);
    //static void addDataField(int fldPos, G4String s);
    //static void addDataField(int fldPos, G4double d);
    static void addDataField(int fldPos, std::string s);
    static void addDataField(int fldPos, char c);
    static void addDataField(int fldPos, int i);
    //static void addDataField(int fldPos, G4int i);
    static void addDataField(int fldPos, double d);
    static void addRecord();

    //static G4int getID(const G4String& tupleIdString);
    //static void addRecord(const CsvFieldVector& fields);
    static void close();

private:
    //static std::map<G4String, G4int> tupleNameToId;
    //static std::mutex mutex;
    //static G4int tupleCounter;
};
