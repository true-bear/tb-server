#include "pch.h"
#include "iocpBuffer.h"

RoundBuffer::RoundBuffer(size_t capacity)
    : mBuffer(capacity), mCapacity(capacity) {
}

bool RoundBuffer::Write(std::span<const std::byte> data)
{
    if (GetFreeSize() < data.size())
        return false;

    size_t firstChunk = std::min(data.size(), mCapacity - mWritePos);
    std::memcpy(mBuffer.data() + mWritePos, data.data(), firstChunk);

    if (firstChunk < data.size())
        std::memcpy(mBuffer.data(), data.data() + firstChunk, data.size() - firstChunk);

    mWritePos = (mWritePos + data.size()) % mCapacity;
    return true;
}

bool RoundBuffer::Peek(std::span<std::byte> dest) const
{
    if (GetStoredSize() < dest.size())
        return false;

    size_t firstChunk = std::min(dest.size(), mCapacity - mReadPos);
    std::memcpy(dest.data(), mBuffer.data() + mReadPos, firstChunk);

    if (firstChunk < dest.size())
        std::memcpy(dest.data() + firstChunk, mBuffer.data(), dest.size() - firstChunk);

    return true;
}

bool RoundBuffer::Read(std::span<std::byte> dest)
{
    if (!Peek(dest)) return false;
    mReadPos = (mReadPos + dest.size()) % mCapacity;
    return true;
}

void RoundBuffer::Clear()
{
    mReadPos = 0;
    mWritePos = 0;
}

size_t RoundBuffer::GetStoredSize() const
{
    if (mWritePos >= mReadPos)
        return mWritePos - mReadPos;
    else
        return mCapacity - (mReadPos - mWritePos);
}

size_t RoundBuffer::GetFreeSize() const
{
    return mCapacity - GetStoredSize() - 1;
}

bool RoundBuffer::CommitRead(const size_t size)
{
    if (GetStoredSize() < size) 
        return false;

    mReadPos = (mReadPos + size) % mCapacity;
    return true;
}

bool RoundBuffer::CommitWrite(const size_t size)
{
    if (GetFreeSize() < size) 
        return false;

    mWritePos = (mWritePos + size) % mCapacity;
    return true;
}

std::byte* RoundBuffer::GetWritePtr()
{
    return mBuffer.data() + mWritePos;
}

size_t RoundBuffer::WritableSize() const
{
    if (mWritePos >= mReadPos)
        return (mReadPos == 0) ? mCapacity - mWritePos : mCapacity - mWritePos;
    else
        return mReadPos - mWritePos;
}