#pragma once

#include <string>
#include <vector>
#include "SchemaMapper.h"
#include "JoinResolver.h"
#include "NlTokenizer.h"

namespace nl2sql {

struct NlCondition {
    std::string column;
    std::string op;
    std::string value;
    std::string connector;
};

struct QueryTemplate {
    std::string action;
    std::vector<std::string> columns;
    std::string table;
    std::vector<NlCondition> conditions;
    std::vector<JoinStep> joins;
    std::string ToSqlString() const;
};

struct SlotFillError {
    std::string code;
    std::string detail;
    size_t line = 1;
};

struct SlotFillResult {
    bool success = false;
    QueryTemplate query;
    std::vector<SlotFillError> errors;
};

class SlotFiller {
public:
    explicit SlotFiller(const SchemaMapper& schema);

    SlotFillResult Fill(const std::string& naturalLanguageInput) const;

private:
    const SchemaMapper& schema_;

    std::vector<std::string> Tokenize(const std::string& input) const;
};

}