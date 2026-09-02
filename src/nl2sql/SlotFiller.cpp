#include "SlotFiller.h"
#include <cctype>
#include <sstream>

namespace nl2sql {

namespace {
std::string ToLowerCopy(const std::string& s) {
    std::string out(s);
    for (char& c : out) c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
    return out;
}
}

std::string QueryTemplate::ToSqlString() const {
    std::string sql = "select ";
    if (columns.empty()) {
        sql += "*";
    } else {
        for (size_t i = 0; i < columns.size(); i++) {
            if (i > 0) sql += ", ";
            sql += columns[i];
        }
    }
    sql += " from " + table;

    for (const auto& join : joins) {
        sql += " " + join.tipo + " " + join.tablaDestino + " on " + join.condicion;
    }

    if (!conditions.empty()) {
        sql += " where ";
        for (size_t i = 0; i < conditions.size(); i++) {
            if (i > 0) {
                std::string connector = conditions[i].connector.empty() ? "AND" : conditions[i].connector;
                sql += " " + connector + " ";
            }
            sql += conditions[i].column + " " + conditions[i].op + " " + conditions[i].value;
        }
    }
    return sql;
}

SlotFiller::SlotFiller(const SchemaMapper& schema) : schema_(schema) {}

std::vector<std::string> SlotFiller::Tokenize(const std::string& input) const {
    std::vector<std::string> tokens;
    std::string current;
    for (char c : input) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') {
            current += static_cast<char>(::tolower(static_cast<unsigned char>(c)));
        } else {
            if (!current.empty()) { tokens.push_back(current); current.clear(); }
        }
    }
    if (!current.empty()) tokens.push_back(current);
    return tokens;
}

SlotFillResult SlotFiller::Fill(const std::string& naturalLanguageInput) const {
    SlotFillResult result;
    NlTokenizer tokenizer(schema_);
    std::vector<WordInfo> tokens = tokenizer.TokenizeWords(naturalLanguageInput);

    if (tokens.empty()) {
        result.errors.push_back({ "NL001", "", 1 });
        result.success = false;
        return result;
    }

    size_t actionIdx = tokens.size();
    for (size_t i = 0; i < tokens.size(); i++) {
        if (schema_.ResolveKeyword(tokens[i].text) == "select") { actionIdx = i; break; }
    }
    if (actionIdx == tokens.size()) {
        result.errors.push_back({ "NL001", "", 1 });
    } else {
        result.query.action = "select";
    }

    size_t fromIdx = tokens.size();
    size_t searchFromStart = (actionIdx < tokens.size()) ? actionIdx + 1 : 0;
    for (size_t i = searchFromStart; i < tokens.size(); i++) {
        if (schema_.ResolveKeyword(tokens[i].text) == "from") { fromIdx = i; break; }
    }
    if (fromIdx == tokens.size()) {
        result.errors.push_back({ "NL002", "", 1 });
    }

    size_t whereIdx = tokens.size();
    size_t searchWhereStart = (fromIdx < tokens.size()) ? fromIdx + 1 : 0;
    for (size_t i = searchWhereStart; i < tokens.size(); i++) {
        if (schema_.ResolveKeyword(tokens[i].text) == "where") { whereIdx = i; break; }
    }

    std::string primaryTable;
    size_t tableTokenIdx = tokens.size();
    size_t scanTableStart = (fromIdx < tokens.size()) ? fromIdx + 1 : 0;
    size_t scanTableEnd = (whereIdx < tokens.size()) ? whereIdx : tokens.size();
    for (size_t i = scanTableStart; i < scanTableEnd; i++) {
        if (schema_.IsIgnoredWord(tokens[i].text)) continue;
        std::string table = schema_.ResolveTableAlias(tokens[i].text);
        if (!table.empty()) { primaryTable = table; tableTokenIdx = i; break; }
    }
    if (primaryTable.empty()) {
        result.errors.push_back({ "NL003", "", 1 });
    } else {
        result.query.table = primaryTable;
    }

    std::string relatedTable;
    if (!primaryTable.empty()) {
        for (size_t i = 0; i < tokens.size(); i++) {
            if (i == tableTokenIdx) continue;
            std::string table = schema_.ResolveTableAlias(tokens[i].text);
            if (!table.empty() && table != primaryTable) { relatedTable = table; break; }
        }
    }
    size_t relatedTableTokenIdx = tokens.size();
    if (!relatedTable.empty() && !primaryTable.empty()) {
        JoinResolver joinResolver(schema_);
        result.query.joins = joinResolver.FindJoinPath(primaryTable, relatedTable);
        for (size_t i = 0; i < tokens.size(); i++) {
            if (i == tableTokenIdx) continue;
            std::string table = schema_.ResolveTableAlias(tokens[i].text);
            if (!table.empty() && table == relatedTable) { relatedTableTokenIdx = i; break; }
        }
    }

    size_t colsScanStart = (actionIdx < tokens.size()) ? actionIdx + 1 : 0;
    size_t colsScanEnd = (fromIdx < tokens.size()) ? fromIdx : (tableTokenIdx < tokens.size() ? tableTokenIdx : tokens.size());
    for (size_t i = colsScanStart; i < colsScanEnd; i++) {
        if (schema_.IsIgnoredWord(tokens[i].text)) continue;
        if (i == relatedTableTokenIdx) continue;
        std::string col = schema_.ResolveColumnAlias(tokens[i].text);
        if (!col.empty()) {
            result.query.columns.push_back(col);
            continue;
        }
        result.errors.push_back({ "NL004", tokens[i].text, tokens[i].line });
    }

    if (tableTokenIdx < tokens.size()) {
        size_t orderScanLimit = (whereIdx < tokens.size()) ? whereIdx : tokens.size();
        for (size_t i = tableTokenIdx + 1; i < orderScanLimit; i++) {
            if (i == relatedTableTokenIdx) continue;
            if (schema_.IsIgnoredWord(tokens[i].text)) continue;
            std::string col = schema_.ResolveColumnAlias(tokens[i].text);
            if (!col.empty()) {
                result.errors.push_back({ "NL005", col, tokens[i].line });
                continue;
            }
            result.errors.push_back({ "NL006", tokens[i].text, tokens[i].line });
        }
    }

    if (whereIdx < tokens.size()) {
        size_t i = whereIdx + 1;
        std::string pendingConnector;
        bool first = true;

        while (i < tokens.size()) {
            while (i < tokens.size() && schema_.IsIgnoredWord(tokens[i].text)) i++;
            if (i >= tokens.size()) {
                if (first) {
                    result.errors.push_back({ "NL007", "", (whereIdx < tokens.size() ? tokens[whereIdx].line : 1) });
                } else {
                    result.errors.push_back({ "NL011", "", (i > 0 ? tokens[i-1].line : 1) });
                }
                break;
            }
            std::string col = schema_.ResolveColumnAlias(tokens[i].text);
            if (col.empty()) {
                result.errors.push_back({ "NL008", tokens[i].text, tokens[i].line });
                i++;
                continue;
            }
            size_t colLine = tokens[i].line;
            i++;

            std::string opSymbol;
            size_t consumed = 0;
            for (size_t window = 3; window >= 1 && opSymbol.empty(); window--) {
                if (i + window > tokens.size()) continue;
                std::string phrase;
                for (size_t w = 0; w < window; w++) {
                    if (w > 0) phrase += " ";
                    phrase += tokens[i + w].text;
                }
                opSymbol = schema_.ResolveOperatorPhrase(phrase);
                if (!opSymbol.empty()) consumed = window;
            }
            if (opSymbol.empty()) {
                result.errors.push_back({ "NL009", "", colLine });
                i++;
                continue;
            }
            i += consumed;

            if (i >= tokens.size()) {
                result.errors.push_back({ "NL010", "", colLine });
                break;
            }

            std::string colType = schema_.ResolveColumnType(col);
            bool isTextColumn = (colType == "varchar" || colType == "char" || colType == "text" ||
                colType == "nvarchar" || colType == "string");

            std::string finalOp = opSymbol;
            std::string value;

            if (opSymbol == "IN" || opSymbol == "NOT IN") {
                value = "(";
                bool firstVal = true;
                size_t j = i;
                for (; j < tokens.size(); j++) {
                    std::string resolved = schema_.ResolveKeyword(tokens[j].text);
                    if (resolved == "and" || resolved == "or") break;
                    std::string v = tokens[j].text;
                    if (isTextColumn) v = "'" + v + "'";
                    if (!firstVal) value += ", ";
                    value += v;
                    firstVal = false;
                }
                value += ")";
                i = j;
            } else if (opSymbol == "LIKE_PREFIX") {
                finalOp = "LIKE";
                value = "'" + tokens[i].text + "%'";
                i++;
            } else if (opSymbol == "LIKE_SUFFIX") {
                finalOp = "LIKE";
                value = "'%" + tokens[i].text + "'";
                i++;
            } else if (opSymbol == "like") {
                finalOp = "LIKE";
                value = "'%" + tokens[i].text + "%'";
                i++;
            } else {
                value = tokens[i].text;
                if (isTextColumn) {
                    value = "'" + value + "'";
                }
                i++;
            }

            result.query.conditions.push_back(NlCondition{ col, finalOp, value, pendingConnector });
            first = false;

            size_t k = i;
            while (k < tokens.size() && schema_.IsIgnoredWord(tokens[k].text)) k++;
            if (k >= tokens.size()) {
                break;
            }
            std::string maybeConnector = schema_.ResolveKeyword(tokens[k].text);
            if (maybeConnector == "and") {
                pendingConnector = "AND";
                i = k + 1;
                continue;
            }
            if (maybeConnector == "or") {
                pendingConnector = "OR";
                i = k + 1;
                continue;
            }
            result.errors.push_back({ "NL012", tokens[k].text, tokens[k].line });
            i = k + 1;
        }
    }

    result.success = result.errors.empty();
    return result;
}

}
