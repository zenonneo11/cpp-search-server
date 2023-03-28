#pragma once
#include <cassert>
#include <vector>
#include <string>
#include "search_server.h"
void AddDocument(SearchServer& ss, int id, const std::string& str, DocumentStatus ds, const std::vector<int> rting);