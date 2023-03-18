#pragma once
#include <string>
#include <vector>
#include <set>

template <typename StringContainer>
std::set<std::string> MakeUniqueNonEmptyStrings(const StringContainer& strings);
void CheckWordSymbols(const std::string& word);
void CheckMinusWord(const std::string& word);
std::vector<std::string> SplitIntoWords(const std::string& text);


/*********************************************************/

template <typename StringContainer>
std::set<std::string> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
    std::set<std::string> non_empty_strings;
    for (const std::string& str : strings) {
        if (!str.empty()) {
            CheckWordSymbols(str);
            non_empty_strings.insert(str);
        }
    }
    return non_empty_strings;
}