#pragma once
#include "pch.h"
#include "roundBuffer.h"

RoundBuffer::RoundBuffer(size_t capacity)
    : mBuffer(std::make_unique<std::byte[]>(capacity)),
    mCapacity(capacity), mReadPos(0), mWritePos(0) {
}

size_t RoundBuffer::ReadableSize() const noexcept 
{
    return mWritePos - mReadPos;
}

size_t RoundBuffer::WritableSize() const noexcept 
{
    return mCapacity - mWritePos;
}

size_t RoundBuffer::Capacity() const noexcept 
{
    return mCapacity;
}

std::byte* RoundBuffer::ReadPtr() noexcept 
{
    return mBuffer.get() + mReadPos;
}

std::byte* RoundBuffer::WritePtr() noexcept 
{
    return mBuffer.get() + mWritePos;
}

void RoundBuffer::MoveReadPos(size_t size) 
{
    if (size > ReadableSize())
        throw std::out_of_range("MoveReadPos overflow");
    
    mReadPos += size;
    if (mReadPos == mWritePos) 
        mReadPos = mWritePos = 0;
}

bool RoundBuffer::Peek(std::byte* out, size_t size)
{
    if (size > ReadableSize())
        return false;

    std::memcpy(out, ReadPtr(), size);
    return true;
}

void RoundBuffer::MoveWritePos(size_t size) 
{
    if (size > WritableSize())
        throw std::out_of_range("MoveWritePos overflow");
    
    mWritePos += size;
}

bool RoundBuffer::Write(std::span<const std::byte> data) 
{
    if (data.size() > WritableSize()) return false;
    std::copy(data.begin(), data.end(), WritePtr());
    MoveWritePos(data.size());
    return true;
}

bool RoundBuffer::Read(std::span<std::byte> outBuffer, size_t size) 
{
    if (size > ReadableSize()) 
        return false;

    std::copy_n(ReadPtr(), size, outBuffer.begin());
    //MoveReadPos(size);
    return true;
}

void RoundBuffer::Reset() noexcept 
{
    mReadPos = mWritePos = 0;
}
