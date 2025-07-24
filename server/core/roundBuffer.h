#pragma once
#include "pch.h"

class RoundBuffer {
public:
    explicit RoundBuffer(size_t capacity);

    [[nodiscard]] size_t ReadableSize() const noexcept;
    [[nodiscard]] size_t WritableSize() const noexcept;
    [[nodiscard]] size_t Capacity() const noexcept;

    [[nodiscard]] std::byte* ReadPtr() noexcept;
    [[nodiscard]] std::byte* WritePtr() noexcept;

    void MoveReadPos(size_t size);
    bool Peek(std::byte* out, size_t size);
    void MoveWritePos(size_t size);

    bool Write(std::span<const std::byte> data);
    bool Read(std::span<std::byte> outBuffer, size_t size);

    void Reset() noexcept;

private:
    std::unique_ptr<std::byte[]> mBuffer;
    size_t mCapacity;
    size_t mReadPos;
    size_t mWritePos;
};