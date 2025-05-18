#pragma once
namespace RIONetwork
{
    bool Init(int port, int maxSession);
    void Run();
    void Stop();
}

class RIOFns
{
public:
    static bool Init();
    static RIO_EXTENSION_FUNCTION_TABLE Table;

private:
    static bool sInitialized;
};
