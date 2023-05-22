#include "process_queries.h"
#include <algorithm>
#include <execution>
using namespace std;

vector<vector<Document>> ProcessQueries(
    const SearchServer& search_server,
    const vector<string>& queries) {
    vector<std::vector<Document>> results(queries.size());

    transform(execution::par, queries.begin(), queries.end(), results.begin(),
        [&search_server](const string& query) { return search_server.FindTopDocuments(query); });
    return results;
}


vector<Document> ProcessQueriesJoined(
    const SearchServer& search_server,
    const std::vector<std::string>& queries) {
    vector<Document> linearised;
    for (auto&& res : ProcessQueries(search_server, queries))
        linearised.insert(linearised.end(), res.begin(), res.end());
    return linearised;
}