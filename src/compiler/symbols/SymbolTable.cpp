#include "SymbolTable.h"
#include <cctype>

namespace compiler::symbols {

std::string SymbolTable::Normalize(const std::string& raw) {
    std::string out(raw);
    for (char& c : out) c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
    return out;
}

void SymbolTable::Declare(const std::string& name, SymbolKind kind, const std::string& dataType) {
    std::string key = Normalize(name);
    symbols_[key] = Symbol{ key, kind, dataType };
}

const Symbol* SymbolTable::ResolveLocal(const std::string& name) const {
    auto it = symbols_.find(Normalize(name));
    return it != symbols_.end() ? &it->second : nullptr;
}

const Symbol* SymbolTable::Resolve(const std::string& name) const {
    const Symbol* local = ResolveLocal(name);
    if (local) return local;
    if (parent_) return parent_->Resolve(name);
    return nullptr;
}

SymbolTable* SymbolTable::CreateChildScope() {
    children_.push_back(std::make_unique<SymbolTable>(this));
    return children_.back().get();
}

void SymbolTable::InjectTable(const std::string& tableName, const std::vector<Symbol>& columns) {
    Declare(tableName, SymbolKind::Table);
    for (const auto& col : columns) {
        Declare(col.name, SymbolKind::Column, col.dataType);
    }
}

}
