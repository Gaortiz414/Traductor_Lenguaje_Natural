#pragma once

#include <memory>
#include <string>
#include <vector>
#include "Ast.h"
#include "../lexer/Token.h"

namespace compiler::syntax {

struct SyntaxError {
    std::string code;
    std::string detail;
    size_t line;
    size_t column;
};

class Parser {
public:
    explicit Parser(const std::vector<compiler::lexer::Token>& tokens);
    std::unique_ptr<Statement> ParseStatement();
    void Synchronize();

    const std::vector<SyntaxError>& Errors() const { return errors_; }
    bool IsAtEnd() const;

private:
    const std::vector<compiler::lexer::Token>& tokens_;
    size_t pos_ = 0;
    std::vector<SyntaxError> errors_;

    const compiler::lexer::Token& Peek(size_t offset = 0) const;
    const compiler::lexer::Token& Advance();
    bool Check(compiler::lexer::TokenType type) const;
    bool Match(compiler::lexer::TokenType type);
    const compiler::lexer::Token& Expect(compiler::lexer::TokenType type, const char* what);
    void ReportError(const std::string& code, const std::string& detail = "");

    std::string TokenText(const compiler::lexer::Token& t) const;
    ColumnRef ParseColumnRef();
    std::vector<Condition> ParseWhereClause();
    std::vector<JoinClause> ParseJoinClauses();
    Condition ParseCondition();
    std::string ParseOperator();

    std::unique_ptr<Statement> ParseSelect();
    std::unique_ptr<Statement> ParseInsert();
    std::unique_ptr<Statement> ParseUpdate();
    std::unique_ptr<Statement> ParseDelete();
    std::unique_ptr<Statement> ParseCreateTable();
};

}
