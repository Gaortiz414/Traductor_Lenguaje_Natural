#pragma once

#include <string>
#include <vector>

namespace compiler::symbols {

enum class SymbolKind {
    Table,
    Column,
    View,
    Alias
};

struct Symbol {
    std::string name;
    SymbolKind kind;
    std::string dataType;
};

}