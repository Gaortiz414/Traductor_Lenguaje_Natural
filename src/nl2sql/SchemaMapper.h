#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace nl2sql {

struct ColumnSchema {
    std::string idCanonico;
    std::string tipoDato;
};

struct TableSchema {
    std::string idCanonico;
    std::vector<ColumnSchema> columnas;
};

struct Relation {
    std::string origen;
    std::string destino;
    std::string condicionJoin;
    std::string tipoDefault;
};

class SchemaMapper {
public:
    bool LoadFromFile(const std::string& path);
    std::string ResolveKeyword(const std::string& wordLower) const;
    std::string ResolveTableAlias(const std::string& wordLower) const;
    std::string ResolveColumnAlias(const std::string& wordLower) const;
    std::string ResolveColumnType(const std::string& columnIdCanonicoLower) const;
    std::string ResolveOperatorPhrase(const std::string& phraseLower) const;
    bool IsIgnoredWord(const std::string& wordLower) const;
    bool IsKnownTable(const std::string& tableIdCanonico) const;

    const std::vector<TableSchema>& Tables() const { return tables_; }
    const std::vector<Relation>& Relations() const { return relations_; }

private:
    std::unordered_map<std::string, std::string> keywordAliases_;
    std::unordered_map<std::string, std::string> tableAliases_;
    std::unordered_map<std::string, std::string> columnAliases_;
    std::unordered_map<std::string, std::string> columnTypes_;
    std::unordered_map<std::string, std::string> operatorPhrases_;
    std::unordered_set<std::string> ignoredWords_;
    std::vector<TableSchema> tables_;
    std::vector<Relation> relations_;
};

}