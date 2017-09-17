//
// Created by wyb on 17-6-27.
//

#ifndef UNLOCKQUEUE_UNLOCK_QUEUE_H
#define UNLOCKQUEUE_UNLOCK_QUEUE_H

#include <stdint.h>
#include <atomic>

class UnlockQueue {
public:
  UnlockQueue(uint32_t size);

  ~UnlockQueue();

  uint32_t Put(const uint8_t *buffer, uint32_t len);

  uint32_t Get(uint8_t *buffer, uint32_t len);

  inline uint32_t size() { return _size; }
  inline uint32_t length() { return (_in - _out); }
  inline bool empty() { return _in <= _out; }

private:
  uint8_t *_buffer;
  uint32_t _size;
  std::atomic<uint32_t> _in;
  std::atomic<uint32_t> _out;
};

#endif //UNLOCKQUEUE_UNLOCK_QUEUE_H
