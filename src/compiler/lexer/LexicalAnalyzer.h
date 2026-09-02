#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include "Token.h"

namespace compiler::lexer {

struct LexicalError {
    std::string code;
    std::string detail;
    size_t line;
    size_t column;
    char offendingChar;
};

class LexicalAnalyzer {
public:
    explicit LexicalAnalyzer(std::string_view source);
    explicit LexicalAnalyzer(std::string&& source) = delete;
    std::vector<Token> Tokenize();
    const std::vector<LexicalError>& Errors() const { return errors_; }

private:
    std::string_view source_;
    size_t pos_ = 0;
    size_t line_ = 1;
    size_t column_ = 1;
    std::vector<LexicalError> errors_;

    static const std::unordered_map<std::string, TokenType>& ReservedWords();

    char Peek(size_t offset = 0) const;
    char Advance();
    bool IsAtEnd() const;
    void SkipWhitespaceAndComments();

    Token MakeToken(TokenType type, size_t start, size_t startLine, size_t startCol);
    Token ScanIdentifierOrKeyword();
    Token ScanDelimitedIdentifier(char closingChar);
    Token ScanNumber();
    Token ScanString();
    Token ScanOperatorOrPunctuation();
};

}
