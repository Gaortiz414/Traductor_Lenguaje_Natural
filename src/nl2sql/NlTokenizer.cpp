#include "NlTokenizer.h"
#include <cctype>
#include <algorithm>

namespace nl2sql {

namespace {
std::string ToUpperCopy(const std::string& s) {
    std::string out(s);
    for (char& c : out) c = static_cast<char>(::toupper(static_cast<unsigned char>(c)));
    return out;
}

bool IsAllDigits(const std::string& s) {
    if (s.empty()) return false;
    for (char c : s) {
        if (!std::isdigit(static_cast<unsigned char>(c))) return false;
    }
    return true;
}

std::string PatronParaKeyword(const std::string& keyword) {
    if (keyword == "select") return "Palabra reservada: accion de consulta (equivalente a SELECT)";
    if (keyword == "from")   return "Palabra reservada: introduce la tabla (equivalente a FROM)";
    if (keyword == "where")  return "Palabra reservada: introduce una condicion (equivalente a WHERE)";
    if (keyword == "create") return "Palabra reservada: accion de creacion (equivalente a CREATE)";
    if (keyword == "and")    return "Palabra reservada: conector logico (equivalente a AND)";
    if (keyword == "or")     return "Palabra reservada: conector logico (equivalente a OR)";
    return "Palabra reservada del lenguaje";
}
}

NlTokenizer::NlTokenizer(const SchemaMapper& schema) : schema_(schema) {}

std::vector<WordInfo> NlTokenizer::TokenizeWords(const std::string& input) const {
    std::vector<WordInfo> tokens;
    std::string current;
    size_t line = 1;
    size_t col = 1;
    size_t startLine = 1;
    size_t startCol = 1;

    size_t n = input.size();
    for (size_t idx = 0; idx < n; idx++) {
        char c = input[idx];
        size_t curCharLine = line;
        size_t curCharCol = col;

        if (c == '\n') {
            line++;
            col = 1;
        } else {
            col++;
        }

        if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') {
            if (current.empty()) {
                startLine = curCharLine;
                startCol = curCharCol;
            }
            current += static_cast<char>(::tolower(static_cast<unsigned char>(c)));

            if (std::isdigit(static_cast<unsigned char>(c)) && IsAllDigits(current)) {
                if (idx + 1 < n && input[idx + 1] == '.' &&
                    idx + 2 < n && std::isdigit(static_cast<unsigned char>(input[idx + 2]))) {
                    current += '.';
                    idx++;
                    if (input[idx] == '\n') { line++; col = 1; } else { col++; }
                    while (idx + 1 < n && std::isdigit(static_cast<unsigned char>(input[idx + 1]))) {
                        current += static_cast<char>(input[idx + 1]);
                        idx++;
                        if (input[idx] == '\n') { line++; col = 1; } else { col++; }
                    }
                }
            }
        } else {
            if (!current.empty()) {
                tokens.push_back(WordInfo{ current, startLine, startCol });
                current.clear();
            }
        }
    }
    if (!current.empty()) {
        tokens.push_back(WordInfo{ current, startLine, startCol });
    }
    return tokens;
}

std::vector<NlToken> NlTokenizer::Tokenize(const std::string& input) const {
    std::vector<NlToken> result;
    std::vector<WordInfo> tokens = TokenizeWords(input);

    size_t i = 0;
    while (i < tokens.size()) {
        size_t tokLine = tokens[i].line;
        size_t tokCol = tokens[i].column;

        std::string opSymbol;
        std::string opPhrase;
        size_t consumed = 0;
        for (size_t window = 3; window >= 1; window--) {
            if (i + window > tokens.size()) continue;
            std::string phrase;
            for (size_t w = 0; w < window; w++) {
                if (w > 0) phrase += " ";
                phrase += tokens[i + w].text;
            }
            std::string symbol = schema_.ResolveOperatorPhrase(phrase);
            if (!symbol.empty()) {
                opSymbol = symbol;
                opPhrase = phrase;
                consumed = window;
                break;
            }
        }
        if (!opSymbol.empty()) {
            NlToken tok;
            tok.tokenName = "TOKEN_OPERADOR";
            tok.lexema = opPhrase;
            tok.patron = "Operador de comparacion (" + opSymbol + ")";
            tok.esPalabraReservada = true;
            tok.linea = tokLine;
            tok.columna = tokCol;
            result.push_back(tok);
            i += consumed;
            continue;
        }

        const std::string& word = tokens[i].text;

        std::string keyword = schema_.ResolveKeyword(word);
        if (!keyword.empty()) {
            NlToken tok;
            tok.tokenName = "TOKEN_" + ToUpperCopy(keyword);
            tok.lexema = word;
            tok.patron = PatronParaKeyword(keyword);
            tok.esPalabraReservada = true;
            tok.linea = tokLine;
            tok.columna = tokCol;
            result.push_back(tok);
            i++;
            continue;
        }

        if (schema_.IsIgnoredWord(word)) {
            NlToken tok;
            tok.tokenName = "TOKEN_IGNORADO";
            tok.lexema = word;
            tok.patron = "Palabra de relleno (se ignora en el analisis)";
            tok.esPalabraReservada = true;
            tok.linea = tokLine;
            tok.columna = tokCol;
            result.push_back(tok);
            i++;
            continue;
        }

        std::string tableCanonical = schema_.ResolveTableAlias(word);
        if (!tableCanonical.empty()) {
            NlToken tok;
            tok.tokenName = "IDENTIFICADOR_TABLA";
            tok.lexema = word;
            tok.patron = "Identificador de tabla del esquema (-> " + tableCanonical + ")";
            tok.esPalabraReservada = false;
            tok.linea = tokLine;
            tok.columna = tokCol;
            result.push_back(tok);
            i++;
            continue;
        }

        std::string columnCanonical = schema_.ResolveColumnAlias(word);
        if (!columnCanonical.empty()) {
            NlToken tok;
            tok.tokenName = "IDENTIFICADOR_COLUMNA";
            tok.lexema = word;
            tok.patron = "Identificador de columna del esquema (-> " + columnCanonical + ")";
            tok.esPalabraReservada = false;
            tok.linea = tokLine;
            tok.columna = tokCol;
            result.push_back(tok);
            i++;
            continue;
        }

        if (word.find('.') != std::string::npos) {
            NlToken tok;
            tok.tokenName = "DECIMAL";
            tok.lexema = word;
            tok.patron = "Valor numerico decimal";
            tok.esPalabraReservada = false;
            tok.linea = tokLine;
            tok.columna = tokCol;
            result.push_back(tok);
            i++;
            continue;
        }

        if (IsAllDigits(word)) {
            NlToken tok;
            tok.tokenName = "ENTERO";
            tok.lexema = word;
            tok.patron = "Valor numerico entero (int)";
            tok.esPalabraReservada = false;
            tok.linea = tokLine;
            tok.columna = tokCol;
            result.push_back(tok);
            i++;
            continue;
        }

        {
            NlToken tok;
            tok.tokenName = "CADENA";
            tok.lexema = word;
            tok.patron = "Cadena de texto (string)";
            tok.esPalabraReservada = false;
            tok.linea = tokLine;
            tok.columna = tokCol;
            result.push_back(tok);
            i++;
        }
    }

    return result;
}

}
