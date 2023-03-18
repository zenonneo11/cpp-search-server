#pragma once
#include <iostream>
#include <vector>
#include <iterator>
#include <algorithm>

template <typename It>
struct Range{
    Range(It range_begin, It range_end):range_begin_(range_begin), range_end_(range_end){
        }
        
    It range_begin_;
    It range_end_;
    
    It begin() const {
        return range_begin_;
    }
    
    It end() const {
        return range_end_;
    }

    size_t size()const {
        return std::distance(range_begin_, range_end_);
    }
};

template <typename It>
std::ostream& operator<<(std::ostream& os,const Range<It>& range){
    for (auto el : range) {
        os <<el;
    }
    return os;
}

template <typename It>
struct Paginator{
    Paginator(It range_begin, It range_end, size_t page_size) {
        while(range_begin != range_end) {
            size_t dist = std::distance(range_begin, range_end);
            pages_.push_back(Range(range_begin, std::next(range_begin, std::min(page_size,dist ))));
            std::advance(range_begin, std::min(page_size, dist));
        }
    }

    auto begin() const {
        return pages_.begin();
    }
    
    auto end() const {
        return pages_.end();
    }

    std::vector <Range<It>> pages_;
};

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator<typename Container::const_iterator>(std::begin(c), std::end(c), page_size);
}