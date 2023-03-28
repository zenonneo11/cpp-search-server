#include <iostream>
#include <string>

#include "test_example_functions.h"
#include "search_server.h"

using namespace std;

void AddDocument(SearchServer& ss, int id, const string& str, DocumentStatus ds, const vector<int> rting){    
    ss.AddDocument(id, str, ds, rting);
}