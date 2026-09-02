#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "Symbol.h"

namespace compiler::symbols {

class SymbolTable {
public:
    SymbolTable() = default;
    explicit SymbolTable(SymbolTable* parent) : parent_(parent) {}

    SymbolTable(const SymbolTable&) = delete;
    SymbolTable& operator=(const SymbolTable&) = delete;
    SymbolTable(SymbolTable&&) = delete;
    SymbolTable& operator=(SymbolTable&&) = delete;

    void Declare(const std::string& name, SymbolKind kind, const std::string& dataType = "");
    const Symbol* Resolve(const std::string& name) const;
    const Symbol* ResolveLocal(const std::string& name) const;
    SymbolTable* CreateChildScope();
    void InjectTable(const std::string& tableName, const std::vector<Symbol>& columns);

    static std::string Normalize(const std::string& raw);

private:
    SymbolTable* parent_ = nullptr;
    std::unordered_map<std::string, Symbol> symbols_;
    std::vector<std::unique_ptr<SymbolTable>> children_;
};

}