#include "pch.h"
#include "rioBuffer.h"
#include "rio.h"


RioBuffer::RioBuffer(size_t size)
    : mSize(size)
{
    mBuffer = new char[size];
    mBufferId = RIOFns::Table.RIORegisterBuffer(mBuffer, static_cast<DWORD>(size));
    assert(mBufferId != RIO_INVALID_BUFFERID);
}

RioBuffer::~RioBuffer()
{
    if (mBufferId != RIO_INVALID_BUFFERID)
    {
        RIOFns::Table.RIODeregisterBuffer(mBufferId);
    }

    delete[] mBuffer;
}

size_t RioBuffer::GetReadableSize() const
{
    return mWritePos - mReadPos;
}

size_t RioBuffer::GetWritableSize() const
{
    return mSize - mWritePos;
}

void RioBuffer::CommitWrite(size_t size)
{
    assert(mWritePos + size <= mSize);
    mWritePos += size;
}

void RioBuffer::CommitRead(size_t size)
{
    assert(mReadPos + size <= mWritePos);
    mReadPos += size;

    if (mReadPos == mWritePos)
    {
        mReadPos = mWritePos = 0;
    }
}

void RioBuffer::Reset()
{
    mReadPos = mWritePos = 0;
}
