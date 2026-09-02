#include "SemanticAnalyzer.h"

namespace compiler::semantic {

using namespace compiler::syntax;
using compiler::symbols::Symbol;
using compiler::symbols::SymbolKind;

SemanticAnalyzer::SemanticAnalyzer(compiler::symbols::SymbolTable& symbols) : symbols_(symbols) {}

void SemanticAnalyzer::CheckTableExists(const std::string& tableName) {
    if (!symbols_.Resolve(tableName)) {
        errors_.push_back(SemanticError{ "SEM001", tableName });
    }
}

void SemanticAnalyzer::AnalyzeSelect(const SelectStatement& stmt) {
    CheckTableExists(stmt.fromTable);
    for (const auto& join : stmt.joins) {
        CheckTableExists(join.table);
    }
}

void SemanticAnalyzer::AnalyzeInsert(const InsertStatement& stmt) {
    CheckTableExists(stmt.intoTable);
    if (!stmt.columns.empty() && stmt.columns.size() != stmt.values.size()) {
        errors_.push_back(SemanticError{ "SEM003", "" });
    }
}

void SemanticAnalyzer::AnalyzeUpdate(const UpdateStatement& stmt) {
    CheckTableExists(stmt.table);
}

void SemanticAnalyzer::AnalyzeDelete(const DeleteStatement& stmt) {
    CheckTableExists(stmt.fromTable);
}

void SemanticAnalyzer::AnalyzeCreateTable(const CreateTableStatement& stmt) {
    if (symbols_.ResolveLocal(stmt.tableName)) {
        errors_.push_back(SemanticError{ "SEM002", stmt.tableName });
        return;
    }
    std::vector<Symbol> columns;
    columns.reserve(stmt.columns.size());
    for (const auto& col : stmt.columns) {
        columns.push_back(Symbol{ compiler::symbols::SymbolTable::Normalize(col.name), SymbolKind::Column, col.dataType });
    }
    symbols_.InjectTable(stmt.tableName, columns);
}

void SemanticAnalyzer::Analyze(const Statement* statement) {
    if (!statement) return;
    switch (statement->kind) {
        case StatementKind::Select:
            AnalyzeSelect(static_cast<const SelectStatement&>(*statement));
            break;
        case StatementKind::Insert:
            AnalyzeInsert(static_cast<const InsertStatement&>(*statement));
            break;
        case StatementKind::Update:
            AnalyzeUpdate(static_cast<const UpdateStatement&>(*statement));
            break;
        case StatementKind::Delete:
            AnalyzeDelete(static_cast<const DeleteStatement&>(*statement));
            break;
        case StatementKind::CreateTable:
            AnalyzeCreateTable(static_cast<const CreateTableStatement&>(*statement));
            break;
    }
}

}
