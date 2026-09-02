#include "SchemaMapper.h"
#include "../../libs/json/json.hpp"
#include <fstream>
#include <cctype>

namespace nl2sql {

using json = nlohmann::json;

namespace {
std::string ToLowerCopy(const std::string& s) {
    std::string out(s);
    for (char& c : out) c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
    return out;
}

std::string KeywordFromTokenName(const std::string& tokenName) {
    const std::string prefix = "TOKEN_";
    std::string name = (tokenName.rfind(prefix, 0) == 0) ? tokenName.substr(prefix.size()) : tokenName;
    return ToLowerCopy(name);
}
}

bool SchemaMapper::LoadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    json root;
    try {
        file >> root;
    } catch (const json::parse_error&) {
        return false;
    }

    try {

    keywordAliases_.clear();
    tableAliases_.clear();
    columnAliases_.clear();
    columnTypes_.clear();
    operatorPhrases_.clear();
    ignoredWords_.clear();
    tables_.clear();
    relations_.clear();

    if (root.contains("configuracion_lexica")) {
        const auto& lex = root["configuracion_lexica"];

        if (lex.contains("palabras_reservadas")) {
            for (const auto& [_, categoria] : lex["palabras_reservadas"].items()) {
                for (const auto& [tokenName, aliases] : categoria.items()) {
                    std::string canonical = KeywordFromTokenName(tokenName);
                    for (const auto& alias : aliases) {
                        keywordAliases_[ToLowerCopy(alias.get<std::string>())] = canonical;
                    }
                }
            }
        }

        if (lex.contains("filtros_nl")) {
            for (const auto& [_, palabras] : lex["filtros_nl"].items()) {
                for (const auto& palabra : palabras) {
                    ignoredWords_.insert(ToLowerCopy(palabra.get<std::string>()));
                }
            }
        }

        if (lex.contains("operadores_nl")) {
            for (const auto& [_, frases] : lex["operadores_nl"].items()) {
                if (frases.empty()) continue;
                std::string symbol = frases[0].get<std::string>();
                for (const auto& frase : frases) {
                    operatorPhrases_[ToLowerCopy(frase.get<std::string>())] = symbol;
                }
            }
        }
    }

    if (root.contains("esquema_base") && root["esquema_base"].contains("tablas")) {
        for (const auto& tablaJson : root["esquema_base"]["tablas"]) {
            TableSchema table;
            table.idCanonico = tablaJson.value("id_canonico", "");
            tableAliases_[ToLowerCopy(table.idCanonico)] = table.idCanonico;
            if (tablaJson.contains("alias_nl")) {
                for (const auto& alias : tablaJson["alias_nl"]) {
                    tableAliases_[ToLowerCopy(alias.get<std::string>())] = table.idCanonico;
                }
            }
            if (tablaJson.contains("columnas")) {
                for (const auto& colJson : tablaJson["columnas"]) {
                    ColumnSchema col;
                    col.idCanonico = colJson.value("id_canonico", "");
                    col.tipoDato = colJson.value("tipo_dato", "");
                    columnAliases_[ToLowerCopy(col.idCanonico)] = col.idCanonico;
                    columnTypes_[ToLowerCopy(col.idCanonico)] = ToLowerCopy(col.tipoDato);
                    if (colJson.contains("alias_nl")) {
                        for (const auto& alias : colJson["alias_nl"]) {
                            columnAliases_[ToLowerCopy(alias.get<std::string>())] = col.idCanonico;
                        }
                    }
                    table.columnas.push_back(col);
                }
            }
            tables_.push_back(table);
        }
    }

    if (root.contains("relaciones")) {
        for (const auto& relJson : root["relaciones"]) {
            Relation rel;
            rel.origen = relJson.value("origen", "");
            rel.destino = relJson.value("destino", "");
            rel.condicionJoin = relJson.value("condicion_join", "");
            rel.tipoDefault = relJson.value("tipo_default", "INNER JOIN");
            relations_.push_back(rel);
        }
    }

    } catch (const json::exception&) {
        keywordAliases_.clear();
        tableAliases_.clear();
        columnAliases_.clear();
        columnTypes_.clear();
        operatorPhrases_.clear();
        ignoredWords_.clear();
        tables_.clear();
        relations_.clear();
        return false;
    }

    return true;
}

std::string SchemaMapper::ResolveKeyword(const std::string& wordLower) const {
    auto it = keywordAliases_.find(wordLower);
    return it != keywordAliases_.end() ? it->second : std::string();
}

std::string SchemaMapper::ResolveTableAlias(const std::string& wordLower) const {
    auto it = tableAliases_.find(wordLower);
    return it != tableAliases_.end() ? it->second : std::string();
}

std::string SchemaMapper::ResolveColumnAlias(const std::string& wordLower) const {
    auto it = columnAliases_.find(wordLower);
    return it != columnAliases_.end() ? it->second : std::string();
}

std::string SchemaMapper::ResolveColumnType(const std::string& columnIdCanonicoLower) const {
    auto it = columnTypes_.find(columnIdCanonicoLower);
    return it != columnTypes_.end() ? it->second : std::string();
}

std::string SchemaMapper::ResolveOperatorPhrase(const std::string& phraseLower) const {
    auto it = operatorPhrases_.find(phraseLower);
    return it != operatorPhrases_.end() ? it->second : std::string();
}

bool SchemaMapper::IsIgnoredWord(const std::string& wordLower) const {
    return ignoredWords_.count(wordLower) > 0;
}

bool SchemaMapper::IsKnownTable(const std::string& tableIdCanonico) const {
    for (const auto& t : tables_) {
        if (t.idCanonico == tableIdCanonico) return true;
    }
    return false;
}

}