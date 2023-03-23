#pragma once

#include <ntddk.h>

#include "../Common/Log.hpp"

//
// Types
//

///
/// Static types
///
using u8  = UINT8;
using u16 = UINT16;
using u32 = UINT32;
using u64 = UINT64;

using i8  = INT8;
using i16 = INT16;
using i32 = INT32;
using i64 = INT64;

using usize = SIZE_T;

#ifndef countof
#define countof(arr) ((sizeof(arr)) / (sizeof(arr[0])))
#endif


///
/// Compile-time types
///


template<typename C, usize S = sizeof(C)>
class GenericBuffer
{
public:
    constexpr GenericBuffer(const C* str) noexcept
    {
        auto i       = 0;
        const C* ptr = str;
        for ( ptr = str, i = 0; i < S + 1; i++ )
        {
            m_buffer[i] = ptr[i];
        }
    }

    constexpr usize
    size() const noexcept
    {
        return m_size;
    }

    constexpr
    operator C*() noexcept
    {
        return m_buffer;
    }

    constexpr operator const C*() const noexcept
    {
        return m_buffer;
    }

    const C*
    get() const noexcept
    {
        return m_buffer;
    }


private:
    C m_buffer[S + 1] = {0};
    usize m_size      = S;
};

using basic_string  = GenericBuffer<char>;
using basic_wstring = GenericBuffer<wchar_t>;


namespace Utils
{


template<typename T>
class KLock
{
    KLock(T& lock) : _lock(lock)
    {
        _lock.Lock();
    }
    ~KLock()
    {
        _lock.Unlock();
    }

private:
    T& _lock;
};


class KMutex
{
public:
    KMutex();
    void
    Lock();
    void
    Unlock();

private:
    KMUTEX _mutex;
};


template<typename T, typename DTOR_FUNC>
class ScopedWrapper
{
public:
    ScopedWrapper(T& f, DTOR_FUNC d) : _f(f), _d(d)
    {
    }

    ~ScopedWrapper()
    {
        _d();
    }

    T
    get() const
    {
        return _f;
    }

private:
    T _f;
    DTOR_FUNC _d;
};


template<typename T>
class KAlloc
{
public:
    KAlloc(const usize sz = 0, const u32 tag = 0) : _tag(tag), _sz(sz), _mem(nullptr)
    {
        if ( !sz || sz >= MAXUSHORT )
            return;

        auto p = ::ExAllocatePoolWithTag(PagedPool, _sz, _tag);
        if ( p )
        {
            _mem = reinterpret_cast<T>(p);
            ::RtlSecureZeroMemory((PUCHAR)_mem, _sz);
        }
    }

    ~KAlloc()
    {
        __free();
    }

    virtual void
    __free()
    {
        if ( _mem != nullptr )
        {
            ::RtlSecureZeroMemory((PUCHAR)_mem, _sz);
            ::ExFreePoolWithTag(_mem, _tag);
            _mem = nullptr;
            _tag = 0;
            _sz  = 0;
        }
    }

    KAlloc(const KAlloc&) = delete;

    KAlloc&
    operator=(const KAlloc&) = delete;

    KAlloc&
    operator=(KAlloc&& other) noexcept
    {
        if ( this != &other )
        {
            _mem       = other._mem;
            _tag       = other._tag;
            _sz        = other._sz;
            other._mem = nullptr;
            other._tag = 0;
            other._sz  = 0;
        }
        return *this;
    }

    const T
    get() const
    {
        return _mem;
    }

    const size_t
    size() const
    {
        return _sz;
    }

    operator bool() const
    {
        return _mem != nullptr && _sz > 0;
    }

    friend bool
    operator==(KAlloc const& lhs, KAlloc const& rhs)
    {
        return lhs._mem == rhs._mem && lhs._tag == rhs._tag;
    }

protected:
    T _mem;
    size_t _sz;
    u32 _tag;
};


class KUnicodeString : public Utils::KAlloc<PUNICODE_STRING>
{
public:
    KUnicodeString(size_t sz = 0, const wchar_t* content = nullptr, u32 tag = 0) :
        KAlloc(sizeof(UNICODE_STRING) + sz + sizeof(WCHAR), tag)
    {
        _mem->Length        = (u16)_sz;
        _mem->MaximumLength = (u16)sz + 1;
        _mem->Buffer        = (PWCH)((ULONG_PTR)(_mem) + sizeof(UNICODE_STRING));
        if ( content && _sz )
        {
            ::RtlCopyMemory(_mem->Buffer, content, _sz);
        }
    }

    const PUNICODE_STRING
    get() const
    {
        return _mem;
    }

    const PWCH
    c_str() const
    {
        return _mem->Buffer;
    }
};
} // namespace Utils
