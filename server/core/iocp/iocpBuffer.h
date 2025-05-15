#pragma once
#include "pch.h"

class IocpBuffer
{
public:
    explicit IocpBuffer(size_t capacity);
    ~IocpBuffer() = default;

    bool Write(const char* data, size_t size);

    bool Read(char* dest, size_t size);
    void Read(size_t size);

    void Clear();

    char* GetWritePtr();
    char* GetReadPtr();

    size_t GetContinuousWriteSize() const;
    size_t GetStoredSize() const;
    size_t GetRemainSize() const;

    void MoveWritePos(size_t size);
    void MoveReadPos(size_t size);

    size_t GetReadPos() const;
    size_t GetWritePos() const;

private:
    size_t GetReadPosWithoutMod() const;

    size_t GetWritePosWithoutMod() const;
private:
    std::unique_ptr<char[]> mBuffer;
    size_t mCapacity;
    size_t mReadPos;
    size_t mWritePos;
};