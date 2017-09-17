//
// Created by wyb on 17-6-27.
//

#include "unlock_queue.h"
#include <algorithm>
#include <cstring>

inline bool is_power_of_2(uint32_t num) {
  return (num != 0 && (num & (num-1)) == 0);
}

inline uint32_t hightest_one_bit(uint32_t num) {
  num |= (num >> 1);
  num |= (num >> 2);
  num |= (num >> 4);
  num |= (num >> 8);
  num |= (num >> 16);
  return num - (num >> 1);
}

inline uint32_t roundup_pow_of_two(uint32_t num) {
  return num > 1 ? hightest_one_bit((num - 1) << 1) : 1;
}

UnlockQueue::UnlockQueue(uint32_t size) : _in(0), _out(0) {
  _size = roundup_pow_of_two(size);
  _buffer = new uint8_t[_size];
}

UnlockQueue::~UnlockQueue() {
  if (!_buffer)
    delete[] _buffer;
}

uint32_t UnlockQueue::Put(const uint8_t *buffer, uint32_t len) {
  len = std::min(len, _size - (_in - _out.load(std::memory_order_acq_rel)));
  //len = std::min(len, _size - (_in - _out));
  // 通用内存屏障，保证out读到正确的值，可能另外一个线程在修改out
  // 用smp_mb是因为上面读out，下面写_buffer
  // smp_mb();

  uint32_t l = std::min(len, _size - (_in & (_size - 1)));
  memcpy(_buffer + (_in & (_size - 1)), buffer, l);
  memcpy(_buffer, buffer + l, len - l);
  // 写内存屏障，保证先写完_buffer，再修改in
  // smp_wmb();
  _in.fetch_add(len, std::memory_order_release);
  //_in += len;

  return len;
}

uint32_t UnlockQueue::Get(uint8_t *buffer, uint32_t len) {
  len = std::min(len, _in.load(std::memory_order_acquire) - _out);
  // 读内存屏障，保证读到正确的in，可能另外一个线程正在修改in
  // 用smp_rmb是因为上面读in，下面读_buffer
  //smp_rmb();

  uint32_t l = std::min(len, _size - (_out & (_size - 1)));
  memcpy(buffer, _buffer + (_out & (_size - 1)), l);
  memcpy(buffer + l, _buffer, len - l);
  // 通用内存屏障，保证先把buffer读出去，再修改out
  // 上面读_bufer，下面写out
  // smp_mb();
  _out.fetch_add(len, std::memory_order_acq_rel);
  //_out += len;

  return len;
}