//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_replacer.cpp
//
// Identification: src/buffer/lru_replacer.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/lru_replacer.h"

namespace bustub {

LRUReplacer::LRUReplacer(size_t num_pages) {
    this->capacity = num_pages;
}

LRUReplacer::~LRUReplacer() = default;

auto LRUReplacer::Victim(frame_id_t *frame_id) -> bool {
    if(lru.empty())
        return false;
    *frame_id = *lru.begin();
    lru.pop_front();
    return true;
}

void LRUReplacer::Pin(frame_id_t frame_id) {
    auto it = lru.begin();
    for(; it != lru.end(); it++){
        if(*it == frame_id)
            break;
    }
    if(it != lru.end())
        lru.erase(it);
}

void LRUReplacer::Unpin(frame_id_t frame_id) {
    for(auto it = lru.begin(); it != lru.end(); it++){
        if(*it == frame_id)
            return;
    }
    if(lru.size() == this->capacity)
        lru.pop_front();
    lru.push_back(frame_id);
}

auto LRUReplacer::Size() -> size_t {
     return lru.size();
}

}  // namespace bustub
