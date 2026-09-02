#pragma once

#include <string>
#include <vector>
#include "SchemaMapper.h"

namespace nl2sql {

struct JoinStep {
    std::string tipo;
    std::string tablaDestino;
    std::string condicion;
};

class JoinResolver {
public:
    explicit JoinResolver(const SchemaMapper& schema);

    std::vector<JoinStep> FindJoinPath(const std::string& fromTable, const std::string& toTable) const;

private:
    const SchemaMapper& schema_;
};

}
