// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

/*
*__sync_fetch_and_add系列一共有十二个函数，有加/减/与/或/异或/等函数的原子性操作函数,__sync_fetch_and_add,
顾名思义，先fetch，然后自加，返回的是自加以前的值。以count = 4为例，调用__sync_fetch_and_add(&count,1),之后，
返回值是4，然后，count变成了5.有__sync_fetch_and_add,自然也就有__sync_add_and_fetch，呵呵这个的意思就很清楚
了，先自加，在返回。他们哥俩的关系与i++和++i的关系是一样的。被谭浩强他老人家收过保护费的都会清楚了。

bool __sync_bool_compare_and_swap (type*ptr, type oldval, type newval, ...)
type __sync_val_compare_and_swap (type *ptr, type oldval,  type newval, ...)
这两个函数提供原子的比较和交换，如果*ptr == oldval,就将newval写入*ptr,
第一个函数在相等并写入的情况下返回true.
第二个函数在返回操作之前的值。
————————————————
原文链接：https://blog.csdn.net/hzhsan/article/details/25124901
*
*/

#ifndef MUDUO_BASE_ATOMIC_H
#define MUDUO_BASE_ATOMIC_H

#include "muduo/base/noncopyable.h"

#include <stdint.h>

namespace muduo
{

namespace detail
{
template<typename T>
class AtomicIntegerT : noncopyable
{
 public:
  AtomicIntegerT()
    : value_(0)
  {
  }

  // uncomment if you need copying and assignment
  //
  // AtomicIntegerT(const AtomicIntegerT& that)
  //   : value_(that.get())
  // {}
  //
  // AtomicIntegerT& operator=(const AtomicIntegerT& that)
  // {
  //   getAndSet(that.get());
  //   return *this;
  // }

  T get()
  {
    // in gcc >= 4.7: __atomic_load_n(&value_, __ATOMIC_SEQ_CST)
    return __sync_val_compare_and_swap(&value_, 0, 0);
  }

  T getAndAdd(T x)
  {
    // in gcc >= 4.7: __atomic_fetch_add(&value_, x, __ATOMIC_SEQ_CST)
    return __sync_fetch_and_add(&value_, x);
  }

  T addAndGet(T x)
  {
    return getAndAdd(x) + x;
  }

  T incrementAndGet()
  {
    return addAndGet(1);
  }

  T decrementAndGet()
  {
    return addAndGet(-1);
  }

  void add(T x)
  {
    getAndAdd(x);
  }

  void increment()
  {
    incrementAndGet();
  }

  void decrement()
  {
    decrementAndGet();
  }

  T getAndSet(T newValue)
  {
    // in gcc >= 4.7: __atomic_exchange_n(&value_, newValue, __ATOMIC_SEQ_CST)
    return __sync_lock_test_and_set(&value_, newValue);
  }

 private:
  volatile T value_;
};
}  // namespace detail

typedef detail::AtomicIntegerT<int32_t> AtomicInt32;
typedef detail::AtomicIntegerT<int64_t> AtomicInt64;

}  // namespace muduo

#endif  // MUDUO_BASE_ATOMIC_H
