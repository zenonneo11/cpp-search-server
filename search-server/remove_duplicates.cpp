#include <algorithm>
#include <iostream>
#include <set>
#include "remove_duplicates.h"

using namespace std;

bool map_key_compare(const map<string, double>& lhs, const map<string, double>& rhs) {
    return lhs.size() == rhs.size() && equal(lhs.begin(), lhs.end(), rhs.begin(), 
        [] (auto a, auto b) { return a.first == b.first; });
}


void RemoveDuplicates(SearchServer& search_server) {
  set<int> id_to_erase;
  for (auto it1 = search_server.begin(), ite1 = search_server.end(); next(it1)!=ite1; ++it1) {
      for (auto it2 = next(it1); it2!=ite1; ++it2){
          auto m1 = search_server.GetWordFrequencies(*it1);
          auto m2 = search_server.GetWordFrequencies(*it2);          
          if (map_key_compare(m1,m2)){
              id_to_erase.insert(*it2);

          }      
      }      
    }
    for (auto id: id_to_erase){
        std::cout<<"Found duplicate document id "<<id<<std::endl;
        search_server.RemoveDocument(id);    
    }
    
}