#include "JoinResolver.h"
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace nl2sql {

JoinResolver::JoinResolver(const SchemaMapper& schema) : schema_(schema) {}

std::vector<JoinStep> JoinResolver::FindJoinPath(const std::string& fromTable, const std::string& toTable) const {
    std::vector<JoinStep> path;
    if (fromTable.empty() || toTable.empty() || fromTable == toTable) return path;

    struct Edge { std::string neighbor; JoinStep step; };
    std::unordered_map<std::string, std::vector<Edge>> adjacency;
    for (const auto& rel : schema_.Relations()) {
        adjacency[rel.origen].push_back(Edge{ rel.destino, JoinStep{ rel.tipoDefault, rel.destino, rel.condicionJoin } });
        adjacency[rel.destino].push_back(Edge{ rel.origen, JoinStep{ rel.tipoDefault, rel.origen, rel.condicionJoin } });
    }

    std::unordered_set<std::string> visited;
    std::unordered_map<std::string, std::pair<std::string, JoinStep>> cameFrom;
    std::queue<std::string> frontier;
    frontier.push(fromTable);
    visited.insert(fromTable);

    bool found = false;
    while (!frontier.empty() && !found) {
        std::string current = frontier.front();
        frontier.pop();
        auto it = adjacency.find(current);
        if (it == adjacency.end()) continue;
        for (const auto& edge : it->second) {
            if (visited.count(edge.neighbor)) continue;
            visited.insert(edge.neighbor);
            cameFrom[edge.neighbor] = { current, edge.step };
            if (edge.neighbor == toTable) { found = true; break; }
            frontier.push(edge.neighbor);
        }
    }

    if (!found) return path;

    std::vector<JoinStep> reversed;
    std::string node = toTable;
    while (node != fromTable) {
        auto it = cameFrom.find(node);
        if (it == cameFrom.end()) return {};
        reversed.push_back(it->second.second);
        node = it->second.first;
    }
    path.assign(reversed.rbegin(), reversed.rend());
    return path;
}

}
