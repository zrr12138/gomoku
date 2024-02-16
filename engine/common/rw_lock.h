/*
 *  Copyright (c) 2020 NetEase Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/*
 * Project: curve
 * Created Date: 18-10-11
 * Author: wudemiao
 */

#ifndef SRC_COMMON_CONCURRENT_RW_LOCK_H_
#define SRC_COMMON_CONCURRENT_RW_LOCK_H_

#include <assert.h>
#include <glog/logging.h>
#include <pthread.h>
#include <sys/types.h>  // gettid

#include "common/curve_compiler_specific.h"
#include "common/uncopyable.h"

// Due to the mixed use of bthread and pthread in some cases, acquiring another
// bthread lock(mutex/rwlock) after acquiring a write lock on a pthread rwlock
// may result in switching the bthread coroutine, and then the operation of
// releasing the previous write lock in the other pthread will not take effect
// (implying that the write lock is still held), thus causing a deadlock.

// Check pthread rwlock tid between wrlock and unlock

#define CURVE_CHECK_PTHREAD_WRLOCK_TID 0


namespace common {

class RWLockBase : public Uncopyable {
 public:
    virtual void WRLock() = 0;
    virtual int TryWRLock() = 0;
    virtual void RDLock() = 0;
    virtual int TryRDLock() = 0;
    virtual void Unlock() = 0;

 protected:
    RWLockBase() = default;
    virtual ~RWLockBase() = default;
};

class PthreadRWLockBase : public RWLockBase {
 public:
    void WRLock() override {
        int ret = pthread_rwlock_wrlock(&rwlock_);
        CHECK(0 == ret) << "wlock failed: " << ret << ", " << strerror(ret);
#if CURVE_CHECK_PTHREAD_WRLOCK_TID
        tid_ = gettid();
#endif
    }

    int TryWRLock() override {
        int ret = pthread_rwlock_trywrlock(&rwlock_);
        if (CURVE_UNLIKELY(ret != 0)) {
            return ret;
        }

#if CURVE_CHECK_PTHREAD_WRLOCK_TID
        tid_ = gettid();
#endif
        return 0;
    }

    void RDLock() override {
        int ret = pthread_rwlock_rdlock(&rwlock_);
        CHECK(0 == ret) << "rlock failed: " << ret << ", " << strerror(ret);
    }

    int TryRDLock() override {
        return pthread_rwlock_tryrdlock(&rwlock_);
    }

    void Unlock() override {
#if CURVE_CHECK_PTHREAD_WRLOCK_TID
        if (tid_ != 0) {
            const pid_t current = gettid();
            // If CHECK here is triggered, please look at the comments at the
            // beginning of the file.
            // In the meantime, the simplest solution might be to use
            // `BthreadRWLock` locks everywhere.
            CHECK(tid_ == current)
                << ", tid has changed, previous tid: " << tid_
                << ", current tid: " << current;
            tid_ = 0;
        }
#endif
        pthread_rwlock_unlock(&rwlock_);
    }

 protected:
    PthreadRWLockBase() = default;
    virtual ~PthreadRWLockBase() = default;

    pthread_rwlock_t rwlock_;
    pthread_rwlockattr_t rwlockAttr_;

#if CURVE_CHECK_PTHREAD_WRLOCK_TID
    pid_t tid_ = 0;
#endif
};

#undef CURVE_CHECK_PTHREAD_WRLOCK_TID

class RWLock : public PthreadRWLockBase {
 public:
    RWLock() {
        pthread_rwlock_init(&rwlock_, nullptr);
    }

    ~RWLock() {
        pthread_rwlock_destroy(&rwlock_);
    }
};  // RWLock class


class ReadLockGuard : public Uncopyable {
 public:
    explicit ReadLockGuard(RWLockBase &rwlock) : rwlock_(rwlock) {
        rwlock_.RDLock();
    }

    ~ReadLockGuard() {
        rwlock_.Unlock();
    }

 private:
    RWLockBase &rwlock_;
};  // ReadLockGuard class

class WriteLockGuard : public Uncopyable {
 public:
    explicit WriteLockGuard(RWLockBase &rwlock) : rwlock_(rwlock) {
        rwlock_.WRLock();
    }

    ~WriteLockGuard() {
        rwlock_.Unlock();
    }

 private:
    RWLockBase &rwlock_;
};  // WriteLockGuard class

}  // namespace common


#endif  // SRC_COMMON_CONCURRENT_RW_LOCK_H_
