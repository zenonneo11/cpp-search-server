#include <stdexcept>
#include <numeric>
#include "search_server.h"

using namespace std;

SearchServer::SearchServer(const string& stop_words_text)
    :SearchServer(string_view(stop_words_text)) {}

SearchServer::SearchServer(string_view stop_words_text)
    : SearchServer(SplitIntoWords(stop_words_text)) {}  // Invoke delegating constructor from string_view container

void SearchServer::AddDocument(int document_id, string_view document, DocumentStatus status, const vector<int>& ratings) {
    if (documents_.count(document_id) != 0) throw invalid_argument("document id already exists");//check document id
    if (document_id < 0) throw invalid_argument("negative document id");  //check document id

    const vector<string_view> words = SplitIntoWordsNoStop(document);
    const double inv_word_count = 1.0 / words.size();
    map<string_view, double>& word_to_freq = document_id_to_word_freqs_[document_id];
    for (string_view word : words) {
        auto word_in_storage_it = storage.insert(string(word)).first;
        word_to_document_freqs_[*word_in_storage_it][document_id] += inv_word_count;
        word_to_freq[*word_in_storage_it] += inv_word_count;
    }

    all_doc_id_.insert(document_id);
    documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
}

void SearchServer::RemoveDocument(int document_id) {
    RemoveDocument(execution::seq, document_id);
}

using It = std::set<int>::const_iterator;
It SearchServer::begin() {
    return all_doc_id_.begin();
}
It SearchServer::end() {
    return all_doc_id_.end();
}
const map<string_view, double>& SearchServer::GetWordFrequencies(int document_id) const {
    if (document_id_to_word_freqs_.count(document_id))
        return document_id_to_word_freqs_.at(document_id);
    else
        return words_to_freq_empty_map_;
}

int SearchServer::GetDocumentCount() const {
    return documents_.size();
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(std::execution::parallel_policy policy, std::string_view raw_query, int document_id) const {
    Query query = ParseQuery(policy, raw_query);

    const auto& word_to_freq = GetWordFrequencies(document_id);
    vector<string_view> doc_words;
    doc_words.reserve(word_to_freq.size());
    for (const auto& [word, _] : word_to_freq) {
        doc_words.push_back(word);
    }

    vector<string_view> matched_words;
    vector<string_view>& minus = query.minus_words;

    for (const auto& word : minus) {
        if (binary_search(doc_words.begin(), doc_words.end(), word)) {
            return { matched_words, documents_.at(document_id).status };
        }
    }


    vector<string_view>& plus = query.plus_words;
    matched_words.resize(min(doc_words.size(), plus.size()));


    sort(policy, plus.begin(), plus.end());
    plus.erase(unique(plus.begin(), plus.end()), plus.end());
    auto it = set_intersection(policy, plus.begin(), plus.end(), doc_words.begin(), doc_words.end(), matched_words.begin());
    matched_words.resize(it - matched_words.begin());
    return { matched_words, documents_.at(document_id).status };

}

tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(execution::sequenced_policy policy, string_view raw_query, int document_id) const {
    const Query query = ParseQuery(policy, raw_query);
    vector<string_view> matched_words;

    for (string_view word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            return { matched_words, documents_.at(document_id).status };
        }
    }

    for (string_view word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }

    sort(matched_words.begin(), matched_words.end());
    matched_words.erase(unique(matched_words.begin(), matched_words.end()), matched_words.end());
    return { matched_words, documents_.at(document_id).status };
}

tuple<vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(std::string_view raw_query, int document_id) const {
    return SearchServer::MatchDocument(execution::seq, raw_query, document_id);
}

bool SearchServer::IsStopWord(const string_view word) const {
    return stop_words_.count(word) > 0;
}

vector<string_view> SearchServer::SplitIntoWordsNoStop(string_view text) const {
    vector<string_view> words;
    for (string_view word : SplitIntoWords(text)) {
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


SearchServer::QueryWord SearchServer::ParseQueryWord(std::string_view text) const {
    CheckWordSymbols(text);
    CheckMinusWord(text);
    bool is_minus = false;
    // Word shouldn't be empty
    if (text[0] == '-') {
        is_minus = true;
        text = text.substr(1);
    }
    return { text, is_minus, IsStopWord(text) };
}


SearchServer::Query SearchServer::ParseQuery(std::execution::sequenced_policy policy, string_view text) const {
    Query query;
    for (string_view word : SplitIntoWords(text)) {
        QueryWord query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                query.minus_words.push_back(move(query_word.data));
            }
            else {
                query.plus_words.push_back(move(query_word.data));
            }
        }
    }
    sort(query.plus_words.begin(), query.plus_words.end());
    query.plus_words.erase(unique(query.plus_words.begin(), query.plus_words.end()), query.plus_words.end());
    return query;   
}

SearchServer::Query SearchServer::ParseQuery(std::execution::parallel_policy policy, string_view text) const {
    Query query;
    for (string_view word : SplitIntoWords(text)) {
        QueryWord query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                query.minus_words.push_back(move(query_word.data));
            }
            else {
                query.plus_words.push_back(move(query_word.data));
            }
        }
    }
    return query;


}

SearchServer::Query SearchServer::ParseQuery(string_view text) const {
    return ParseQuery(execution::seq, text);
}

// Existence required
double SearchServer::ComputeWordInverseDocumentFreq(string_view word) const {
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}
