// -*- C++ -*-
//===----------------------- support/win32/pthread.h ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP_PTHREAD_WIN32_PTHREAD_H
#define _LIBCPP_PTHREAD_WIN32_PTHREAD_H

#include <windows.h>

#if _WIN32_WINNT < 0x0600
#error Win32 threads support requires at least Windows Vista
#endif

static inline int sched_yield() {
    return SwitchToThread() ? 0 : -1;
}

// Threads
typedef HANDLE pthread_t;
typedef struct {} pthread_attr_t;

static inline pthread_t pthread_self() {
    return GetCurrentThread();
}

// Windows uses a DWORD (uint32_t) return from thread functions, but
// pthreads uses a void*. The only caller of pthread_create in libc++ should
// be for std::thread, which ignores the return value.
struct __wthread_proxy_arg_ {
    void *(*__ts_)(void *);
    void *__ta_;
};

static DWORD __stdcall __wthread_proxy_(LPVOID __tp_arg_) {
    auto __wtp_data_ = reinterpret_cast<__wthread_proxy_arg_ *>(__tp_arg_);

    __wtp_data_->__ts_(__wtp_data_->__ta_);
    return 0;
}

static inline int pthread_create(pthread_t *__t_, const pthread_attr_t *__a_,
                                 void *(*__s_)(void *), void *__v_) {
    if (__a_ != nullptr) {
        return EINVAL;
    }

    __wthread_proxy_arg_ __tp_a_ = {__s_, __v_};

    // TODO(danalbert): Add a basic pthread_attr_t shim for CREATE_SUSPENDED?
    *__t_ = CreateThread(NULL, 0, __wthread_proxy_,
                         reinterpret_cast<void *>(&__tp_a_), 0, NULL);
    if (*__t_ == NULL) {
        return GetLastError();
    }

    return 0;
}

static inline int pthread_equal(pthread_t __a_, pthread_t __b_) {
    return __a_ == __b_;
}

static inline int pthread_join(pthread_t __t_, void**) {
    return WaitForSingleObject(__t_, INFINITE) == WAIT_OBJECT_0
               ? 0
               : GetLastError();
}

static inline int pthread_detach(pthread_t __t_) {
    return CloseHandle(__t_) ? 0 : GetLastError();
}

// Mutexes
typedef SRWLOCK pthread_mutex_t;
#define PTHREAD_MUTEX_INITIALIZER SRWLOCK_INIT

static inline int pthread_mutex_lock(pthread_mutex_t *__m_) {
    AcquireSRWLockExclusive(__m_);
    return 0;
}

static inline int pthread_mutex_trylock(pthread_mutex_t *__m_) {
    return TryAcquireSRWLockExclusive(__m_) ? 0 : -1;
}

static inline int pthread_mutex_unlock(pthread_mutex_t *__m_) {
    ReleaseSRWLockExclusive(__m_);
    return 0;
}

static inline int pthread_mutex_destroy(pthread_mutex_t*) {
    return 0;
}

// Condition variables
typedef CONDITION_VARIABLE pthread_cond_t;
#define PTHREAD_COND_INITIALIZER CONDITION_VARIABLE_INIT

static inline int pthread_cond_wait(pthread_cond_t *__c_,
                                    pthread_mutex_t *__m_) {
    return SleepConditionVariableSRW(__c_, __m_, INFINITE, 0) ? 0 : -1;
}

static inline int pthread_cond_timedwait(pthread_cond_t *__c_,
                                         pthread_mutex_t *__m_,
                                         const struct timespec *__ts_) {
    DWORD __ms_ = __ts_->tv_sec * 1000 + __ts_->tv_nsec / 1000000;
    return SleepConditionVariableSRW(__c_, __m_, __ms_, 0) ? 0 : -1;
}

static inline int pthread_cond_signal(pthread_cond_t *__c_) {
    WakeConditionVariable(__c_);
    return 0;
}

static inline int pthread_cond_broadcast(pthread_cond_t *__c_) {
    WakeAllConditionVariable(__c_);
    return 0;
}

static inline int pthread_cond_destroy(pthread_cond_t*) {
    return 0;
}

// TLS
struct __win32_tls {
    DWORD __index_;
    void (*__dstr_)(void *);
};

typedef __win32_tls pthread_key_t;

static inline int pthread_key_create(pthread_key_t *__k_,
                                     void (*__d_)(void *)) {
    DWORD __r_ = TlsAlloc();
    if (__r_ == TLS_OUT_OF_INDEXES) {
        return GetLastError();
    }

    *__k_ = {__r_, __d_};
    return 0;
}

static inline int pthread_key_delete(pthread_key_t __k_) {
    if (__k_.__dstr_) {
        __k_.__dstr_(TlsGetValue(__k_.__index_));
    }

    return TlsFree(__k_.__index_) ? 0 : GetLastError();
}

static inline void* pthread_getspecific(pthread_key_t __k_) {
    return TlsGetValue(__k_.__index_);
}

static inline int pthread_setspecific(pthread_key_t __k_, const void* __v_) {
    return TlsSetValue(__k_.__index_, const_cast<void*>(__v_)) ? 0 : -1;
}

#endif // _LIBCPP_PTHREAD_WIN32_PTHREAD_H
