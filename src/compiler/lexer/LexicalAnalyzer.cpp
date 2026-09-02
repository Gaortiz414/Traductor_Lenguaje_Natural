#include "LexicalAnalyzer.h"
#include <cctype>
#include <cstdlib>

namespace compiler::lexer {

namespace {
std::string ToLowerCopy(std::string_view sv) {
    std::string out(sv);
    for (char& c : out) c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
    return out;
}
}

const std::unordered_map<std::string, TokenType>& LexicalAnalyzer::ReservedWords() {
    static const std::unordered_map<std::string, TokenType> words = {
        {"select", TokenType::TOKEN_SELECT}, {"from", TokenType::TOKEN_FROM},
        {"where", TokenType::TOKEN_WHERE},   {"insert", TokenType::TOKEN_INSERT},
        {"into", TokenType::TOKEN_INTO},     {"values", TokenType::TOKEN_VALUES},
        {"update", TokenType::TOKEN_UPDATE}, {"set", TokenType::TOKEN_SET},
        {"delete", TokenType::TOKEN_DELETE}, {"join", TokenType::TOKEN_JOIN},
        {"inner", TokenType::TOKEN_INNER},   {"left", TokenType::TOKEN_LEFT},
        {"right", TokenType::TOKEN_RIGHT},   {"on", TokenType::TOKEN_ON},
        {"and", TokenType::TOKEN_AND},       {"or", TokenType::TOKEN_OR},
        {"order", TokenType::TOKEN_ORDER},   {"by", TokenType::TOKEN_BY},
        {"group", TokenType::TOKEN_GROUP},   {"having", TokenType::TOKEN_HAVING},
        {"create", TokenType::TOKEN_CREATE}, {"table", TokenType::TOKEN_TABLE},
        {"alter", TokenType::TOKEN_ALTER},   {"drop", TokenType::TOKEN_DROP},
        {"primary", TokenType::TOKEN_CONSTRAINT}, {"foreign", TokenType::TOKEN_CONSTRAINT},
        {"key", TokenType::TOKEN_CONSTRAINT},
        {"identity", TokenType::TOKEN_MODIFIER}, {"not", TokenType::TOKEN_NOT},
        {"null", TokenType::TOKEN_MODIFIER},
        {"like", TokenType::TOKEN_LIKE},     {"in", TokenType::TOKEN_IN},
    };
    return words;
}

LexicalAnalyzer::LexicalAnalyzer(std::string_view source) : source_(source) {}

bool LexicalAnalyzer::IsAtEnd() const { return pos_ >= source_.size(); }

char LexicalAnalyzer::Peek(size_t offset) const {
    size_t idx = pos_ + offset;
    return idx < source_.size() ? source_[idx] : '\0';
}

char LexicalAnalyzer::Advance() {
    char c = source_[pos_++];
    if (c == '\n') { line_++; column_ = 1; }
    else { column_++; }
    return c;
}

void LexicalAnalyzer::SkipWhitespaceAndComments() {
    while (!IsAtEnd()) {
        char c = Peek();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            Advance();
        } else if (c == '-' && Peek(1) == '-') {
            while (!IsAtEnd() && Peek() != '\n') Advance();
        } else {
            break;
        }
    }
}

Token LexicalAnalyzer::MakeToken(TokenType type, size_t start, size_t startLine, size_t startCol) {
    std::string_view lexeme = source_.substr(start, pos_ - start);
    return Token{ type, lexeme, startLine, startCol };
}

Token LexicalAnalyzer::ScanIdentifierOrKeyword() {
    size_t start = pos_, startLine = line_, startCol = column_;
    while (!IsAtEnd() && (std::isalnum(static_cast<unsigned char>(Peek())) || Peek() == '_')) {
        Advance();
    }
    std::string_view lexeme = source_.substr(start, pos_ - start);
    std::string lowered = ToLowerCopy(lexeme);
    const auto& words = ReservedWords();
    auto it = words.find(lowered);
    TokenType type = (it != words.end()) ? it->second : TokenType::TOKEN_IDENTIFIER;
    return Token{ type, lexeme, startLine, startCol };
}

Token LexicalAnalyzer::ScanDelimitedIdentifier(char closingChar) {
    size_t startLine = line_, startCol = column_;
    Advance();
    size_t contentStart = pos_;
    while (!IsAtEnd() && Peek() != closingChar) {
        Advance();
    }
    std::string_view lexeme = source_.substr(contentStart, pos_ - contentStart);
    if (!IsAtEnd()) {
        Advance();
    } else {
        errors_.push_back(LexicalError{ "LEX002", "", startLine, startCol, closingChar });
    }
    return Token{ TokenType::TOKEN_IDENTIFIER_DELIM, lexeme, startLine, startCol };
}

Token LexicalAnalyzer::ScanNumber() {
    size_t start = pos_, startLine = line_, startCol = column_;
    while (!IsAtEnd() && std::isdigit(static_cast<unsigned char>(Peek()))) Advance();
    if (Peek() == '.' && std::isdigit(static_cast<unsigned char>(Peek(1)))) {
        Advance();
        while (!IsAtEnd() && std::isdigit(static_cast<unsigned char>(Peek()))) Advance();
    }
    return MakeToken(TokenType::TOKEN_NUMBER, start, startLine, startCol);
}

Token LexicalAnalyzer::ScanString() {
    size_t startLine = line_, startCol = column_;
    Advance();
    size_t contentStart = pos_;
    while (!IsAtEnd() && Peek() != '\'') Advance();
    std::string_view lexeme = source_.substr(contentStart, pos_ - contentStart);
    if (!IsAtEnd()) {
        Advance();
    } else {
        errors_.push_back(LexicalError{ "LEX003", "", startLine, startCol, '\'' });
    }
    return Token{ TokenType::TOKEN_STRING, lexeme, startLine, startCol };
}

Token LexicalAnalyzer::ScanOperatorOrPunctuation() {
    size_t start = pos_, startLine = line_, startCol = column_;
    char c = Advance();
    switch (c) {
        case '=': return MakeToken(TokenType::TOKEN_IGUAL, start, startLine, startCol);
        case ',': return MakeToken(TokenType::TOKEN_COMA, start, startLine, startCol);
        case '.': return MakeToken(TokenType::TOKEN_PUNTO, start, startLine, startCol);
        case ';': return MakeToken(TokenType::TOKEN_PUNTO_Y_COMA, start, startLine, startCol);
        case '(': return MakeToken(TokenType::TOKEN_PARENTESIS_IZQ, start, startLine, startCol);
        case ')': return MakeToken(TokenType::TOKEN_PARENTESIS_DER, start, startLine, startCol);
        case '*': return MakeToken(TokenType::TOKEN_ASTERISCO, start, startLine, startCol);
        case '!':
            if (Peek() == '=') { Advance(); return MakeToken(TokenType::TOKEN_DISTINTO, start, startLine, startCol); }
            break;
        case '<':
            if (Peek() == '=') { Advance(); return MakeToken(TokenType::TOKEN_MENOR_IGUAL, start, startLine, startCol); }
            if (Peek() == '>') { Advance(); return MakeToken(TokenType::TOKEN_DISTINTO, start, startLine, startCol); }
            return MakeToken(TokenType::TOKEN_MENOR, start, startLine, startCol);
        case '>':
            if (Peek() == '=') { Advance(); return MakeToken(TokenType::TOKEN_MAYOR_IGUAL, start, startLine, startCol); }
            return MakeToken(TokenType::TOKEN_MAYOR, start, startLine, startCol);
        default:
            break;
    }
    errors_.push_back(LexicalError{ "LEX001", std::string(1, c), startLine, startCol, c });
    return Token{ TokenType::TOKEN_ERROR, source_.substr(start, 1), startLine, startCol };
}

std::vector<Token> LexicalAnalyzer::Tokenize() {
    std::vector<Token> tokens;
    while (true) {
        SkipWhitespaceAndComments();
        if (IsAtEnd()) {
            tokens.push_back(Token{ TokenType::TOKEN_EOF, std::string_view(), line_, column_ });
            break;
        }

        char c = Peek();
        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            tokens.push_back(ScanIdentifierOrKeyword());
        } else if (std::isdigit(static_cast<unsigned char>(c))) {
            tokens.push_back(ScanNumber());
        } else if (c == '[') {
            tokens.push_back(ScanDelimitedIdentifier(']'));
        } else if (c == '"') {
            tokens.push_back(ScanDelimitedIdentifier('"'));
        } else if (c == '\'') {
            tokens.push_back(ScanString());
        } else {
            tokens.push_back(ScanOperatorOrPunctuation());
        }
    }
    return tokens;
}

}
