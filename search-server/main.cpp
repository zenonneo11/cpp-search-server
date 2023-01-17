#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <cmath>
using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    int id;
    double relevance;
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        ++document_count_;
        const vector<string> words = SplitIntoWordsNoStop(document);
        for (const auto& word:words){
                word_to_document_freqs_[word][document_id]+=(1.0/words.size());
               
        }
        
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const Query query_words= ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 return lhs.relevance > rhs.relevance;
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:

    struct Query{
        set<string> plus_words;
        set<string> minus_words;
    };
    
    map<string, map<int,double>> word_to_document_freqs_;
    set<string> stop_words_;
    int document_count_=0;
    
    
    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    Query ParseQuery(const string& text) const {
        set<string> plus_words;
        set<string> minus_words;
        for (const string& word : SplitIntoWords(text)) {
            if (word[0]!='-'){
                if(!IsStopWord(word)){
                    plus_words.insert(word);    
                }
                    
            }else{
                string minus_word=word.substr(1);
                if(!IsStopWord(minus_word)){
                    minus_words.insert(minus_word);
                }
            }
            
        }
        return {plus_words,minus_words};
    }

    vector<Document> FindAllDocuments(const Query& query_words) const {
        
        map<int,double> id_to_relevance;
        for (const auto& query_word : query_words.plus_words) {
           if (word_to_document_freqs_.count(query_word)>0){
               double word_idf=log(1.0*document_count_/(word_to_document_freqs_.at(query_word).size()));          
               for (const auto& [doc_id,term_freq] :word_to_document_freqs_.at(query_word)){
                        id_to_relevance[doc_id]+=(word_idf*term_freq);
                   }              
               }                                     
            }
     
        set<int> minus_id;
        for (const auto& query_word : query_words.minus_words) {
           if (word_to_document_freqs_.count(query_word)>0){
               for (auto [id,_] :word_to_document_freqs_.at(query_word)){
                   minus_id.insert(id);
               }                                     
            }
        }
        
        vector<Document> matched_documents;
        for (const auto& [id,relevance]:id_to_relevance){
            if (minus_id.count(id)==0)
                matched_documents.push_back({id,relevance});
        }
        
        return matched_documents;
    }    
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());
    const int document_count = ReadLineWithNumber();
    
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }
    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }
}