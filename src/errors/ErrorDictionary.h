#pragma once

#include <string>
#include <unordered_map>

namespace errors {

class ErrorDictionary {
public:

    bool LoadFromFile(const std::string& path);

    std::string Resolve(const std::string& code, const std::string& detalle = "") const;

private:
    std::unordered_map<std::string, std::string> plantillas_;
};

const ErrorDictionary& GetErrorDictionary(bool* outLoadedOk = nullptr);

}
