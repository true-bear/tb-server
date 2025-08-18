export module util.roundbuffer;

import <memory>;
import <cstddef>;
import <span>;
import <stdexcept>;
import <cstring>;

export class RoundBuffer 
{
public:
    [[nodiscard]] explicit RoundBuffer(size_t capacity)
        : mBuffer(std::make_unique<std::byte[]>(capacity)),
        mCapacity(capacity), mReadPos(0), mWritePos(0) 
    {
    }

    [[nodiscard]] size_t ReadableSize() const noexcept 
    {
        return mWritePos - mReadPos;
    }

    [[nodiscard]] size_t WritableSize() const noexcept 
    {
        return mCapacity - mWritePos;
    }

    [[nodiscard]] size_t Capacity() const noexcept 
    {
        return mCapacity;
    }

    [[nodiscard]] std::byte* ReadPtr() noexcept 
    {
        return mBuffer.get() + mReadPos;
    }

    [[nodiscard]] std::byte* WritePtr() noexcept 
    {
        return mBuffer.get() + mWritePos;
    }

    void MoveReadPos(size_t size) 
    {
        if (size > ReadableSize())
            throw std::out_of_range("MoveReadPos overflow");
        
        mReadPos += size;
        if (mReadPos == mWritePos) 
            mReadPos = mWritePos = 0;
    }

    bool Peek(std::byte* out, size_t size) 
    {
        if (size > ReadableSize())
            return false;

        std::memcpy(out, ReadPtr(), size);
        return true;
    }

    void MoveWritePos(size_t size) 
    {
        if (size > WritableSize())
            throw std::out_of_range("MoveWritePos overflow");

        mWritePos += size;
    }

    bool Write(std::span<const std::byte> data) 
    {
        if (data.size() > WritableSize())
            return false;

        std::memcpy(WritePtr(), data.data(), data.size());
        MoveWritePos(data.size());
        return true;
    }

    bool Read(std::span<std::byte> outBuffer, size_t size) 
    {
        if (size > ReadableSize())
            return false;

        std::memcpy(outBuffer.data(), ReadPtr(), size);
        MoveReadPos(size);
        return true;
    }

    void Reset() noexcept 
    {
        mReadPos = 0;
        mWritePos = 0;
    }

private:
    std::unique_ptr<std::byte[]> mBuffer;
    size_t mCapacity;
    size_t mReadPos;
    size_t mWritePos;
};
