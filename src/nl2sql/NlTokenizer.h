#pragma once

#include <string>
#include <vector>
#include "SchemaMapper.h"

namespace nl2sql {

struct NlToken {
    std::string tokenName;
    std::string lexema;
    std::string patron;
    bool esPalabraReservada;
    size_t linea = 1;
    size_t columna = 1;
};

struct WordInfo {
    std::string text;
    size_t line;
    size_t column;
};

class NlTokenizer {
public:
    explicit NlTokenizer(const SchemaMapper& schema);

    std::vector<NlToken> Tokenize(const std::string& input) const;
    std::vector<WordInfo> TokenizeWords(const std::string& input) const;

private:
    const SchemaMapper& schema_;
};

}
