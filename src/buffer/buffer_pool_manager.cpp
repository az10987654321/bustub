//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// buffer_pool_manager.cpp
//
// Identification: src/buffer/buffer_pool_manager.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/buffer_pool_manager.h"

#include <list>
#include <unordered_map>

namespace bustub {

BufferPoolManager::BufferPoolManager(size_t pool_size, DiskManager *disk_manager, LogManager *log_manager)
    : pool_size_(pool_size), disk_manager_(disk_manager), log_manager_(log_manager) {
  // We allocate a consecutive memory space for the buffer pool.
  pages_ = new Page[pool_size_];
  replacer_ = new LRUReplacer(pool_size);

  // Initially, every page is in the free list.
  for (size_t i = 0; i < pool_size_; ++i) {
    free_list_.emplace_back(static_cast<int>(i));
  }
}

BufferPoolManager::~BufferPoolManager() {
  delete[] pages_;
  delete replacer_;
}

Page *BufferPoolManager::FetchPageImpl(page_id_t page_id) {
  // 1.     Search the page table for the requested page (P).
  frame_id_t frame_id;
  Page* pg = nullptr;
  auto target = page_table_.find(page_id); // b. 判断存在与访问数据只用一次查找
  // 1.1    If P exists, pin it and return it immediately.
  if (target != page_table_.end()) {
    frame_id = target->second;
    // c. 通过指针运算获取 frame_id 处存放的 Page 结构体
    pg = pages_ + frame_id;
    pg->pin_count_++;
    this->replacer_->Pin(frame_id); // d. 将对应 page 从“回收站”中捞出
    return pg;
  }
  // 1.2    If P does not exist, find a replacement page (R) from either the free list or the replacer.
  //        Note that pages are always found from the free list first.
  if (!this->free_list_.empty()) {
    frame_id = this->free_list_.back();
    this->free_list_.pop_back();
    pg = this->pages_ + frame_id;
  }
  else {
    if (!this->replacer_->Victim(&frame_id))
      return nullptr;
    pg = this->pages_ + frame_id;
  // 2.     If R is dirty, write it back to the disk.
    if (pg->IsDirty()) {
      disk_manager_->WritePage(pg->GetPageId(), pg->GetData());
      pg->is_dirty_ = false;
    }
    pg->pin_count_ = 0; // g. 将元信息 pin_count_ 清空
  }
  // 3.     Delete R from the page table and insert P.
  this->page_table_.erase(pg->GetPageId()); // h. 时刻注意区分 p->GetPageId() 与 page_id 是否相等，别混用
  this->page_table_[page_id] = frame_id;
  // 4.     Update P's metadata, read in the page content from disk, and then return a pointer to P.
  pg->page_id_ = page_id;
  pg->ResetMemory();
  disk_manager_->ReadPage(page_id, pg->GetData());
  pg->pin_count_++;
  return pg;
}

bool BufferPoolManager::UnpinPageImpl(page_id_t page_id, bool is_dirty) {
  frame_id_t frame_id;
  Page* pg = nullptr;
  auto target = this->page_table_.find(page_id);
  if (target != this->page_table_.end()) {
    frame_id = target->second;
    pg = this->pages_ + frame_id;
    if (pg->pin_count_ <= 0)
      return false;
    pg->pin_count_ = 0;
    pg->is_dirty_ = is_dirty;
    this->replacer_->Unpin(frame_id);
  }
  return true;
}

bool BufferPoolManager::FlushPageImpl(page_id_t page_id) {
  // Make sure you call DiskManager::WritePage!
  frame_id_t frame_id;
  Page* pg = nullptr;
  auto target = this->page_table_.find(page_id);
  if (target == this->page_table_.end())
    return false;
  frame_id = target->second;
  pg = this->pages_ + frame_id;
  this->disk_manager_->WritePage(pg->GetPageId(), pg->GetData());
  pg->pin_count_ = 0; // g. 将元信息 pin_count_ 清空
  return true;
}

Page *BufferPoolManager::NewPageImpl(page_id_t *page_id) {
  // 0.   Make sure you call DiskManager::AllocatePage!
  frame_id_t frame_id;
  Page* pg = nullptr;
  // 1.   If all the pages in the buffer pool are pinned, return nullptr.
  // 2.   Pick a victim page P from either the free list or the replacer. Always pick from the free list first.
  if (!this->free_list_.empty()) {
    frame_id = this->free_list_.back();
    this->free_list_.pop_back();
    pg = this->pages_ + frame_id;
  }
  else {
    if (!this->replacer_->Victim(&frame_id))
      return nullptr;
    pg = this->pages_ + frame_id;
  // 2.     If R is dirty, write it back to the disk.
    if (pg->IsDirty()) {
      disk_manager_->WritePage(pg->GetPageId(), pg->GetData());
      pg->is_dirty_ = false;
    }
    pg->pin_count_ = 0; // g. 将元信息 pin_count_ 清空
  }
  // 3.   Update P's metadata, zero out memory and add P to the page table.
  *page_id = this->disk_manager_->AllocatePage();
  this->page_table_.erase(pg->GetPageId()); // h. 时刻注意区分 p->GetPageId() 与 page_id 是否相等，别混用
  this->page_table_[*page_id] = frame_id;
  // 4.   Set the page ID output parameter. Return a pointer to P.
  pg->page_id_ = *page_id;
  pg->pin_count_++;
  return pg;
}

bool BufferPoolManager::DeletePageImpl(page_id_t page_id) {
  // 0.   Make sure you call DiskManager::DeallocatePage!
  // 1.   Search the page table for the requested page (P).
  // 1.   If P does not exist, return true.
  // 2.   If P exists, but has a non-zero pin-count, return false. Someone is using the page.
  // 3.   Otherwise, P can be deleted. Remove P from the page table, reset its metadata and return it to the free list.
  return false;
}

void BufferPoolManager::FlushAllPagesImpl() {
  // You can do it!
}

}  // namespace bustub
