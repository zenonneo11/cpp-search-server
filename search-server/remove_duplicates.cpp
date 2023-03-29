#include <algorithm>
#include <iostream>
#include <set>
#include <utility>
#include "remove_duplicates.h"

using namespace std;

void RemoveDuplicates(SearchServer& search_server) {
    set<int> id_to_erase;
    set<set<string>> set_of_docs_words;
    for (auto id: search_server) {
        set<string> doc_words;
        for (const auto& [word,_]: search_server.GetWordFrequencies(id)){
            doc_words.insert(word);     
        }
        auto [_, is_inserted] = set_of_docs_words.insert(move(doc_words));
        if (!is_inserted){
            id_to_erase.insert(id);
        }    
    }
    for (auto id: id_to_erase){
        std::cout<<"Found duplicate document id "<<id<<std::endl;
        search_server.RemoveDocument(id);    
    }
    
}
