#pragma once
#include "pch.h"

class RioBuffer
{
public:
    RioBuffer(size_t size);
    ~RioBuffer();

    RIO_BUFFERID GetBufferId() const { return mBufferId; }
    char* GetRawBuffer() const { return mBuffer; }
    size_t GetTotalSize() const { return mSize; }

    size_t GetReadableSize() const;
    size_t GetWritableSize() const;

    size_t GetReadOffset() const { return mReadPos; }
    size_t GetWriteOffset() const { return mWritePos; }

    char* GetReadPointer() const { return mBuffer + mReadPos; }
    char* GetWritePointer() const { return mBuffer + mWritePos; }

    void CommitWrite(size_t size);
    void CommitRead(size_t size);
    void Reset();

private:
    char* mBuffer = nullptr;
    size_t mSize = 0;

    size_t mReadPos = 0;
    size_t mWritePos = 0;

    RIO_BUFFERID mBufferId = RIO_INVALID_BUFFERID;
};
