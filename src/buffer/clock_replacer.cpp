//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// clock_replacer.cpp
//
// Identification: src/buffer/clock_replacer.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/clock_replacer.h"

namespace bustub {

ClockReplacer::ClockReplacer(size_t num_pages) {
    max_page_num = num_pages;
    clock_hand = 0;
}

ClockReplacer::~ClockReplacer() = default;

auto ClockReplacer::Victim(frame_id_t *frame_id) -> bool {
    struct frame *frame_n;
    for ( ; ; ){
        frame_n = &frame_list[clock_hand];
        if (frame_n->in_clock == true){
            if (frame_n->ref_flag == false){
                clock_hand = (clock_hand + 1) % frame_list.size();
                break;
            }
            else
                frame_n->ref_flag = false;
        }
        clock_hand = (clock_hand + 1) % frame_list.size();
    }
    frame_n->in_clock = false;
    *frame_id = frame_n->id;
    return true;
}

void ClockReplacer::Pin(frame_id_t frame_id) {
    for (auto& frame_n : frame_list){
        if (frame_n.id == frame_id && frame_n.in_clock == true)
            frame_n.ref_flag = true;
    }
}

void ClockReplacer::Unpin(frame_id_t frame_id) {
    for (auto frame_n : frame_list){
        if (frame_n.id == frame_id)
            return;
    }
    if (frame_list.size() < max_page_num){
        struct frame new_frame;
        new_frame.id = frame_id;
        new_frame.in_clock = true;
        new_frame.ref_flag = false;
        frame_list.push_back(new_frame);
    }
}

auto ClockReplacer::Size() -> size_t {
    size_t frame_count = 0;
    for (auto& frame_n : frame_list){
        if (frame_n.in_clock == true && frame_n.ref_flag == false)
            frame_count++;
    }
    return frame_count;
}

}  // namespace bustub
