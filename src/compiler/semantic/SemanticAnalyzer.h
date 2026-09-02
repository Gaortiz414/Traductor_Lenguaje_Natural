#pragma once

#include <string>
#include <vector>
#include "../syntax/Ast.h"
#include "../symbols/SymbolTable.h"

namespace compiler::semantic {

struct SemanticError {
    std::string code;
    std::string detail;
};
class SemanticAnalyzer {
public:
    explicit SemanticAnalyzer(compiler::symbols::SymbolTable& symbols);

    void Analyze(const compiler::syntax::Statement* statement);

    const std::vector<SemanticError>& Errors() const { return errors_; }

private:
    compiler::symbols::SymbolTable& symbols_;
    std::vector<SemanticError> errors_;

    void AnalyzeSelect(const compiler::syntax::SelectStatement& stmt);
    void AnalyzeInsert(const compiler::syntax::InsertStatement& stmt);
    void AnalyzeUpdate(const compiler::syntax::UpdateStatement& stmt);
    void AnalyzeDelete(const compiler::syntax::DeleteStatement& stmt);
    void AnalyzeCreateTable(const compiler::syntax::CreateTableStatement& stmt);

    void CheckTableExists(const std::string& tableName);
};

}
