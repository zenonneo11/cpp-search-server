#include <iostream>
#include <stdexcept>
#include "string_processing.h"

using namespace std;

void CheckWordSymbols(const string& word) {
    for (char c: word){
        int symbol = static_cast<int>(c);
        if (symbol >= 0&&symbol<=31){              
            throw invalid_argument("invalid symbol");
        }                
    }
}

void CheckMinusWord(const string& word) {
    if ((word.size() == 1 && word[0] == '-') ||                //check empty word after "-"
        (word.size() > 1 && word[0] == '-' && word[1] == '-')) {    //check "--" in the begining of the word
        throw invalid_argument("invalid minus argument");
    }
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
