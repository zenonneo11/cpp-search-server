#include <stdexcept>
#include <numeric>
#include "search_server.h"

using namespace std;

SearchServer::SearchServer(const string& stop_words_text)
    : SearchServer(SplitIntoWords(stop_words_text)){}  // Invoke delegating constructor from string container
    
void SearchServer::AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) {
    if (documents_.count(document_id) != 0) throw invalid_argument("document id already exists");//check document id
    if (document_id < 0) throw invalid_argument("negative document id");  //check document id
    
    const vector<string> words = SplitIntoWordsNoStop(document);
    const double inv_word_count = 1.0 / words.size();
    map<string, double>& word_to_freq = document_id_to_word_freqs_[document_id];
    for (const string& word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
        word_to_freq[word]+=inv_word_count;
    }

    all_doc_id_.insert(document_id);
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status}); 
}

void SearchServer::RemoveDocument(int document_id){
     auto& word_to_freq = GetWordFrequencies(document_id);
     for (const auto& [word,_]: word_to_freq){
         word_to_document_freqs_[word].erase(document_id);
         if (word_to_document_freqs_[word].empty())
             word_to_document_freqs_.erase(word);    
         }
     document_id_to_word_freqs_.erase(document_id);
     all_doc_id_.erase(document_id);
     documents_.erase(document_id);    
}

using It = std::set<int>::const_iterator;
It SearchServer::begin(){
    return all_doc_id_.begin();
}
It SearchServer::end(){
    return all_doc_id_.end();
}
const map<string, double>& SearchServer::GetWordFrequencies(int document_id) const{
    
    if (document_id_to_word_freqs_.count(document_id))
        return document_id_to_word_freqs_.at(document_id);
    else
        return words_to_freq_empty_map_;
}

vector<Document> SearchServer::FindTopDocuments(const string& raw_query, DocumentStatus status) const {
    return FindTopDocuments(
        raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
            return document_status == status;
        });
}

vector<Document> SearchServer::FindTopDocuments(const string& raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

int SearchServer::GetDocumentCount() const {
    return documents_.size();
}

tuple<vector<string>, DocumentStatus> SearchServer::MatchDocument(const string& raw_query, int document_id) const {
    const Query query = ParseQuery(raw_query);
    vector<string> matched_words;
    for (const string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }
    for (const string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.clear();
            break;
        }
    }
    return make_tuple(matched_words, documents_.at(document_id).status);
}

bool SearchServer::IsStopWord(const string& word) const {
    return stop_words_.count(word) > 0;
}

vector<string> SearchServer::SplitIntoWordsNoStop(const string& text) const {
    vector<string> words;
    for (const string& word : SplitIntoWords(text)) {
        CheckWordSymbols(word);
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    int rating_sum = accumulate(ratings.begin(), ratings.end(), 0);
    return rating_sum / static_cast<int>(ratings.size());
}


SearchServer::QueryWord SearchServer::ParseQueryWord(string text) const {
    CheckWordSymbols(text);
    CheckMinusWord(text);
    bool is_minus = false;
    // Word shouldn't be empty
    if (text[0] == '-') {
        is_minus = true;
        text = text.substr(1);
    }
    return {text, is_minus, IsStopWord(text)};
}


SearchServer::Query SearchServer::ParseQuery(const string& text) const {
    Query query;
    for (const string& word : SplitIntoWords(text)) {
        const QueryWord query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                query.minus_words.insert(query_word.data);
            } else {
                query.plus_words.insert(query_word.data);
            }
        }
    }
    return query;
}

// Existence required
double SearchServer::ComputeWordInverseDocumentFreq(const string& word) const {
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}
