#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace compiler::syntax {

enum class StatementKind {
    Select,
    Insert,
    Update,
    Delete,
    CreateTable
};

struct Statement {
    StatementKind kind;
    virtual ~Statement() = default;
    explicit Statement(StatementKind k) : kind(k) {}
};

struct ColumnRef {
    std::string qualifier;
    std::string name;
};

struct Condition {
    ColumnRef column;
    std::string op;
    std::string value;
};

struct ColumnDef {
    std::string name;
    std::string dataType;
    bool isPrimaryKey = false;
    bool isNotNull = false;
};

struct JoinClause {
    std::string joinType;
    std::string table;
    ColumnRef leftColumn;
    ColumnRef rightColumn;
};

struct SelectStatement : Statement {
    SelectStatement() : Statement(StatementKind::Select) {}
    bool selectAll = false;
    std::vector<ColumnRef> columns;
    std::string fromTable;
    std::vector<JoinClause> joins;
    std::vector<Condition> whereConditions;
};

struct InsertStatement : Statement {
    InsertStatement() : Statement(StatementKind::Insert) {}
    std::string intoTable;
    std::vector<std::string> columns;
    std::vector<std::string> values;
};

struct UpdateStatement : Statement {
    UpdateStatement() : Statement(StatementKind::Update) {}
    std::string table;
    std::vector<std::pair<std::string, std::string>> assignments;
    std::vector<Condition> whereConditions;
};

struct DeleteStatement : Statement {
    DeleteStatement() : Statement(StatementKind::Delete) {}
    std::string fromTable;
    std::vector<Condition> whereConditions;
};

struct CreateTableStatement : Statement {
    CreateTableStatement() : Statement(StatementKind::CreateTable) {}
    std::string tableName;
    std::vector<ColumnDef> columns;
};

}
