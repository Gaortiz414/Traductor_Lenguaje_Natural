#include "ErrorDictionary.h"
#include "../../libs/json/json.hpp"
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#endif

using json = nlohmann::json;

namespace errors {

bool ErrorDictionary::LoadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    json root;
    try {
        file >> root;
    } catch (const json::parse_error&) {
        return false;
    }

    try {
        plantillas_.clear();
        if (root.contains("errores")) {
            for (const auto& [codigo, mensaje] : root["errores"].items()) {
                plantillas_[codigo] = mensaje.get<std::string>();
            }
        }
    } catch (const json::exception&) {
        return false;
    }

    return true;
}

std::string ErrorDictionary::Resolve(const std::string& code, const std::string& detalle) const {
    auto it = plantillas_.find(code);
    if (it == plantillas_.end()) {
        return "[codigo desconocido: " + code + "]";
    }

    std::string result = it->second;
    const std::string placeholder = "{detalle}";
    size_t pos = result.find(placeholder);
    if (pos != std::string::npos) {
        result.replace(pos, placeholder.size(), detalle);
    }
    return result;
}

#ifdef _WIN32
namespace {
std::string NarrowPath(const std::wstring& wide) {
    if (wide.empty()) return std::string();
    int size = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), (int)wide.size(), nullptr, 0, nullptr, nullptr);
    std::string out(size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), (int)wide.size(), out.data(), size, nullptr, nullptr);
    return out;
}
}
#endif

const ErrorDictionary& GetErrorDictionary(bool* outLoadedOk) {
    static ErrorDictionary dict;
    static bool attempted = false;
    static bool loadedOk = false;
    if (!attempted) {
#ifdef _WIN32
        wchar_t exePath[MAX_PATH];
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        std::wstring exeDir(exePath);
        size_t slash = exeDir.find_last_of(L"\\/");
        exeDir = (slash != std::wstring::npos) ? exeDir.substr(0, slash) : L".";

        std::string candidate1 = NarrowPath(exeDir) + "\\config\\dictionary_errors.json";
        loadedOk = dict.LoadFromFile(candidate1) || dict.LoadFromFile("config/dictionary_errors.json");
#else
        loadedOk = dict.LoadFromFile("config/dictionary_errors.json");
#endif
        attempted = true;
    }
    if (outLoadedOk) *outLoadedOk = loadedOk;
    return dict;
}

}
