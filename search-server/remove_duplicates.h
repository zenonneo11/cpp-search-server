#pragma once
#include <map>
#include <string>
#include "search_server.h"

bool map_key_compare(const std::map<std::string, double>& lhs, const std::map<std::string, double>& rhs);

void RemoveDuplicates(SearchServer& search_server);