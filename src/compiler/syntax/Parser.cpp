#include "Parser.h"

namespace compiler::syntax {

using compiler::lexer::Token;
using compiler::lexer::TokenType;

Parser::Parser(const std::vector<Token>& tokens) : tokens_(tokens) {}

bool Parser::IsAtEnd() const {
    return pos_ >= tokens_.size() || tokens_[pos_].type == TokenType::TOKEN_EOF;
}

const Token& Parser::Peek(size_t offset) const {
    size_t idx = pos_ + offset;
    return idx < tokens_.size() ? tokens_[idx] : tokens_.back();
}

const Token& Parser::Advance() {
    const Token& t = Peek();
    if (!IsAtEnd()) pos_++;
    return t;
}

bool Parser::Check(TokenType type) const {
    return Peek().type == type;
}

bool Parser::Match(TokenType type) {
    if (Check(type)) { Advance(); return true; }
    return false;
}

const Token& Parser::Expect(TokenType type, const char* what) {
    if (Check(type)) return Advance();
    ReportError("SYN001", what);
    return Advance();
}

void Parser::ReportError(const std::string& code, const std::string& detail) {
    const Token& t = Peek();
    errors_.push_back(SyntaxError{ code, detail, t.line, t.column });
}

std::string Parser::TokenText(const Token& t) const {
    return std::string(t.lexeme);
}

ColumnRef Parser::ParseColumnRef() {
    ColumnRef ref;
    std::vector<std::string> parts;

    if (Check(TokenType::TOKEN_ASTERISCO)) {
        Advance();
        ref.name = "*";
        return ref;
    }

    parts.push_back(TokenText(Expect(TokenType::TOKEN_IDENTIFIER, "un identificador")));
    while (Match(TokenType::TOKEN_PUNTO)) {
        parts.push_back(TokenText(Expect(TokenType::TOKEN_IDENTIFIER, "un identificador despues de '.'")));
    }

    ref.name = parts.back();
    parts.pop_back();
    for (size_t i = 0; i < parts.size(); i++) {
        if (i > 0) ref.qualifier += ".";
        ref.qualifier += parts[i];
    }
    return ref;
}

std::string Parser::ParseOperator() {
    switch (Peek().type) {
        case TokenType::TOKEN_IGUAL:       Advance(); return "=";
        case TokenType::TOKEN_DISTINTO:    Advance(); return "!=";
        case TokenType::TOKEN_MAYOR:       Advance(); return ">";
        case TokenType::TOKEN_MAYOR_IGUAL: Advance(); return ">=";
        case TokenType::TOKEN_MENOR:       Advance(); return "<";
        case TokenType::TOKEN_MENOR_IGUAL: Advance(); return "<=";
        case TokenType::TOKEN_LIKE:        Advance(); return "LIKE";
        case TokenType::TOKEN_IN:          Advance(); return "IN";
        case TokenType::TOKEN_NOT:
            if (Peek(1).type == TokenType::TOKEN_IN) {
                Advance();
                Advance();
                return "NOT IN";
            }
            ReportError("SYN002");
            return "";
        default:
            ReportError("SYN002");
            return "";
    }
}

Condition Parser::ParseCondition() {
    Condition cond;
    cond.column = ParseColumnRef();
    cond.op = ParseOperator();
    if (cond.op.empty()) {
        return cond;
    }
    if (cond.op == "IN" || cond.op == "NOT IN") {
        std::string value = "(";
        Expect(TokenType::TOKEN_PARENTESIS_IZQ, "'('");
        value += TokenText(Advance());
        while (Match(TokenType::TOKEN_COMA)) {
            value += ", ";
            value += TokenText(Advance());
        }
        Expect(TokenType::TOKEN_PARENTESIS_DER, "')'");
        value += ")";
        cond.value = value;
        return cond;
    }
    const Token& valueTok = Advance();
    cond.value = TokenText(valueTok);
    return cond;
}

std::vector<Condition> Parser::ParseWhereClause() {
    std::vector<Condition> conditions;
    conditions.push_back(ParseCondition());
    while (Match(TokenType::TOKEN_AND) || Match(TokenType::TOKEN_OR)) {
        conditions.push_back(ParseCondition());
    }
    return conditions;
}

std::vector<JoinClause> Parser::ParseJoinClauses() {
    std::vector<JoinClause> joins;
    while (Check(TokenType::TOKEN_INNER) || Check(TokenType::TOKEN_LEFT) ||
           Check(TokenType::TOKEN_RIGHT) || Check(TokenType::TOKEN_JOIN)) {
        JoinClause join;
        if (Check(TokenType::TOKEN_INNER)) {
            join.joinType = "INNER";
            Advance();
            Expect(TokenType::TOKEN_JOIN, "JOIN");
        } else if (Check(TokenType::TOKEN_LEFT)) {
            join.joinType = "LEFT";
            Advance();
            Expect(TokenType::TOKEN_JOIN, "JOIN");
        } else if (Check(TokenType::TOKEN_RIGHT)) {
            join.joinType = "RIGHT";
            Advance();
            Expect(TokenType::TOKEN_JOIN, "JOIN");
        } else {
            join.joinType = "INNER";
            Advance();
        }

        join.table = TokenText(Expect(TokenType::TOKEN_IDENTIFIER, "el nombre de una tabla"));
        Expect(TokenType::TOKEN_ON, "ON");
        join.leftColumn = ParseColumnRef();
        Expect(TokenType::TOKEN_IGUAL, "'='");
        join.rightColumn = ParseColumnRef();

        joins.push_back(std::move(join));
    }
    return joins;
}

std::unique_ptr<Statement> Parser::ParseSelect() {
    Expect(TokenType::TOKEN_SELECT, "SELECT");
    auto stmt = std::make_unique<SelectStatement>();

    if (Check(TokenType::TOKEN_ASTERISCO)) {
        Advance();
        stmt->selectAll = true;
    } else {
        stmt->columns.push_back(ParseColumnRef());
        while (Match(TokenType::TOKEN_COMA)) {
            stmt->columns.push_back(ParseColumnRef());
        }
    }

    Expect(TokenType::TOKEN_FROM, "FROM");
    stmt->fromTable = TokenText(Expect(TokenType::TOKEN_IDENTIFIER, "el nombre de una tabla"));

    stmt->joins = ParseJoinClauses();

    if (Match(TokenType::TOKEN_WHERE)) {
        stmt->whereConditions = ParseWhereClause();
    }

    return stmt;
}

std::unique_ptr<Statement> Parser::ParseInsert() {
    Expect(TokenType::TOKEN_INSERT, "INSERT");
    Expect(TokenType::TOKEN_INTO, "INTO");
    auto stmt = std::make_unique<InsertStatement>();
    stmt->intoTable = TokenText(Expect(TokenType::TOKEN_IDENTIFIER, "el nombre de una tabla"));

    if (Match(TokenType::TOKEN_PARENTESIS_IZQ)) {
        stmt->columns.push_back(TokenText(Expect(TokenType::TOKEN_IDENTIFIER, "un nombre de columna")));
        while (Match(TokenType::TOKEN_COMA)) {
            stmt->columns.push_back(TokenText(Expect(TokenType::TOKEN_IDENTIFIER, "un nombre de columna")));
        }
        Expect(TokenType::TOKEN_PARENTESIS_DER, "')'");
    }

    Expect(TokenType::TOKEN_VALUES, "VALUES");
    Expect(TokenType::TOKEN_PARENTESIS_IZQ, "'('");
    stmt->values.push_back(TokenText(Advance()));
    while (Match(TokenType::TOKEN_COMA)) {
        stmt->values.push_back(TokenText(Advance()));
    }
    Expect(TokenType::TOKEN_PARENTESIS_DER, "')'");

    return stmt;
}

std::unique_ptr<Statement> Parser::ParseUpdate() {
    Expect(TokenType::TOKEN_UPDATE, "UPDATE");
    auto stmt = std::make_unique<UpdateStatement>();
    stmt->table = TokenText(Expect(TokenType::TOKEN_IDENTIFIER, "el nombre de una tabla"));
    Expect(TokenType::TOKEN_SET, "SET");

    auto parseAssignment = [&]() {
        std::string col = TokenText(Expect(TokenType::TOKEN_IDENTIFIER, "un nombre de columna"));
        Expect(TokenType::TOKEN_IGUAL, "'='");
        std::string val = TokenText(Advance());
        stmt->assignments.emplace_back(col, val);
    };
    parseAssignment();
    while (Match(TokenType::TOKEN_COMA)) parseAssignment();

    if (Match(TokenType::TOKEN_WHERE)) {
        stmt->whereConditions = ParseWhereClause();
    }

    return stmt;
}

std::unique_ptr<Statement> Parser::ParseDelete() {
    Expect(TokenType::TOKEN_DELETE, "DELETE");
    Expect(TokenType::TOKEN_FROM, "FROM");
    auto stmt = std::make_unique<DeleteStatement>();
    stmt->fromTable = TokenText(Expect(TokenType::TOKEN_IDENTIFIER, "el nombre de una tabla"));

    if (Match(TokenType::TOKEN_WHERE)) {
        stmt->whereConditions = ParseWhereClause();
    }

    return stmt;
}

std::unique_ptr<Statement> Parser::ParseCreateTable() {
    Expect(TokenType::TOKEN_CREATE, "CREATE");
    Expect(TokenType::TOKEN_TABLE, "TABLE");
    auto stmt = std::make_unique<CreateTableStatement>();
    stmt->tableName = TokenText(Expect(TokenType::TOKEN_IDENTIFIER, "el nombre de una tabla"));
    Expect(TokenType::TOKEN_PARENTESIS_IZQ, "'('");

    auto parseColumnDef = [&]() {
        ColumnDef col;
        col.name = TokenText(Expect(TokenType::TOKEN_IDENTIFIER, "un nombre de columna"));
        col.dataType = TokenText(Expect(TokenType::TOKEN_IDENTIFIER, "un tipo de dato"));
        while (Check(TokenType::TOKEN_CONSTRAINT) || Check(TokenType::TOKEN_MODIFIER) || Check(TokenType::TOKEN_NOT)) {
            std::string modifier = TokenText(Peek());
            if (Peek().type == TokenType::TOKEN_CONSTRAINT) col.isPrimaryKey = true;
            if (Peek().type == TokenType::TOKEN_MODIFIER) col.isNotNull = true;
            Advance();
        }
        stmt->columns.push_back(col);
    };
    parseColumnDef();
    while (Match(TokenType::TOKEN_COMA)) parseColumnDef();

    Expect(TokenType::TOKEN_PARENTESIS_DER, "')'");
    return stmt;
}

std::unique_ptr<Statement> Parser::ParseStatement() {
    std::unique_ptr<Statement> stmt;
    switch (Peek().type) {
        case TokenType::TOKEN_SELECT: stmt = ParseSelect(); break;
        case TokenType::TOKEN_INSERT: stmt = ParseInsert(); break;
        case TokenType::TOKEN_UPDATE: stmt = ParseUpdate(); break;
        case TokenType::TOKEN_DELETE: stmt = ParseDelete(); break;
        case TokenType::TOKEN_CREATE: stmt = ParseCreateTable(); break;
        default:
            ReportError("SYN003");
            Advance();
            return nullptr;
    }
    Match(TokenType::TOKEN_PUNTO_Y_COMA);
    return stmt;
}

void Parser::Synchronize() {
    while (!IsAtEnd()) {
        if (tokens_[pos_].type == TokenType::TOKEN_PUNTO_Y_COMA) {
            Advance();
            return;
        }
        switch (Peek().type) {
            case TokenType::TOKEN_SELECT:
            case TokenType::TOKEN_INSERT:
            case TokenType::TOKEN_UPDATE:
            case TokenType::TOKEN_DELETE:
            case TokenType::TOKEN_CREATE:
                return;
            default:
                break;
        }
        Advance();
    }
}

}