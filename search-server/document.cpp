#include "document.h"
using namespace std;

Document::Document(int id, double relevance, int rating)
    : id(id)
    , relevance(relevance)
    , rating(rating) {
}

ostream& operator<<(ostream& os, const Document& doc) {
    auto [document_id, relevance, rating]=doc; 
    os<<"{ document_id = "<<document_id<<", relevance = "<<relevance<<", rating = "<<rating<<" }";    
    return os;
}