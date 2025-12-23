// CsvManager.cc
#include "CsvServices.hh"
#include <stdexcept>
#include "Logger.hh"

//std::map<G4String, G4int> CsvServices::tupleNameToId;
//std::mutex CsvManager::mutex;
//G4int CsvServices::tupleCounter = 0;

//G4AnalysisManager* CsvServices::getManager() {
//    auto* am = G4AnalysisManager::Instance();
//    am->SetVerboseLevel(1);
//    return (am);
//}

G4AnalysisManager* CsvServices::getManager() {
    thread_local G4AnalysisManager* cachedAm = [] {
        auto* ptr = G4AnalysisManager::Instance();
        Logger::debug("CSV", "Initialized analysis manager for thread");
        //G4cout << "[CsvServices] Initialized analysis manager for thread ID "
        //       << std::this_thread::get_id() << G4endl;
        return ptr;
    }();
    return cachedAm;
}

void CsvServices::openFile(const G4String& fileName) {
    auto* am = getManager();
    Logger::debug("CSV", "Open(" + fileName + ")");
    am->OpenFile(fileName);
    G4String amType = (am && !am->GetType().empty())
        ? am->GetType()
        : "undefined";
    Logger::debug("CSV", "G4 Analysis Manager (type: " + amType + ") has opened file: " + fileName);
}

G4int CsvServices::createTuple(const G4String& tupleIdString, const G4String& tupleDesc, const std::vector<std::pair<G4String, char>>& columns) {
    //std::lock_guard<std::mutex> lock(mutex);
    Logger::debug("CSV", "creating tuple (" + tupleIdString + ", " + tupleDesc + ")");
    auto* am = getManager();
    G4int id = am->CreateNtuple(tupleIdString, tupleDesc);
    Logger::debug("CSV", "Tuple [" + tupleIdString + "] created, id: " + std::to_string(id));
    for (const auto& col : columns) {
        const auto& name = col.first;
        char type = col.second;
        if (type == 'D') {
            Logger::debug("CSV", "[" + tupleIdString + "] " + name + " --> CreateNtupleDColumn");
            am->CreateNtupleDColumn(name);
        } else if (type == 'S') {
            Logger::debug("CSV", "[" + tupleIdString + "] " + name + " --> CreateNtupleSColumn");
            am->CreateNtupleSColumn(name);
        } else {
            throw std::invalid_argument("Unsupported column type for " + std::string(name));
        }
    }
    am->FinishNtuple();
    //tupleNameToId[tupleIdString] = id;
    return id;
}

//void CsvServices::addDataField(int fldPos, G4String s) {
//    auto* am = getManager();
//    am->FillNtupleSColumn(fldPos, s);
//}

//void CsvServices::addDataField(int fldPos, G4double d) {
//    auto* am = getManager();
//    am->FillNtupleDColumn(fldPos, d);
//}

void CsvServices::addDataField(int fldPos, std::string s) {
    auto* am = getManager();
    am->FillNtupleSColumn(fldPos, s);
}

void CsvServices::addDataField(int fldPos, char c) {
    auto* am = getManager();
    am->FillNtupleSColumn(fldPos, std::string(1, c));
}

void CsvServices::addDataField(int fldPos, int i) {
    auto* am = getManager();
    am->FillNtupleDColumn(fldPos, static_cast<double>(i));
}

//void CsvServices::addDataField(int fldPos, G4int i) {
//    auto* am = getManager();
//    am->FillNtupleDColumn(fldPos, static_cast<double>(i));
//}

void CsvServices::addDataField(int fldPos, double d) {
    auto* am = getManager();
    am->FillNtupleDColumn(fldPos, d);
}

void CsvServices::addRecord() {
    auto* am = getManager();
    am->AddNtupleRow();
}

/*
void CsvServices::addRXecord(const CsvFieldVector& fields) {
    auto* am = getManager();
    unsigned fIdx = 0;
    for (const auto& field : fields) {
        std::visit([am, &fIdx](auto&& val) {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_arithmetic_v<T>) {
                //G4cout << "[NUMERIC] " << val << G4endl;
                // Call numeric handler
                am->FillNtupleDColumn(fIdx++, val);
            } else {
                //G4cout << "[STRING] " << val << G4endl;
                // Call alphanumeric handler
                am->FillNtupleSColumn(fIdx++, val);
            }
        }, field);
    }
    am->AddNtupleRow();
}*/

//G4int CsvServices::getID(const G4String& tupleIdString) {
//    std::lock_guard<std::mutex> lock(mutex);
//    auto it = tupleNameToId.find(tupleIdString);
//    if (it == tupleNameToId.end()) {
//        throw std::runtime_error("Tuple ID string not found: " + std::string(tupleIdString));
//    }
//    return it->second;
//}

void CsvServices::close() {
    Logger::debug("CSV", "close()");
    auto* am = getManager();
    Logger::debug("CSV", "close(): Writing ...");
    am->Write();
    Logger::debug("CSV", "close(): Closing file ...");
    am->CloseFile();
}

