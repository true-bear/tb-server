#pragma once
#include "pch.h"

class RoundBuffer {
public:
    explicit RoundBuffer(const size_t capacity);

    bool Write(std::span<const std::byte> data);
    bool Read(std::span<std::byte> dest);
    bool Peek(std::span<std::byte> dest) const;

    bool CommitRead(const size_t size);
	bool CommitWrite(const size_t size);

    void Clear();

    size_t GetStoredSize() const;
    size_t GetFreeSize() const;
    size_t WritableSize() const;


    std::byte* GetWritePtr();


private:
    std::vector<std::byte> mBuffer;
    size_t mCapacity;
    size_t mReadPos = 0;
    size_t mWritePos = 0;
};