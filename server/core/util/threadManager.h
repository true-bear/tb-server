#pragma once
#include "pch.h"

class ThreadManager 
{
public:
	void Run(std::function<void(void)> callback)
	{
		std::lock_guard guard(mLock);
		mThreads.emplace_back([=]() {
			try {
				callback();
			}
			catch (const std::exception& ex) {
				std::cerr << "[ThreadManager] ����: " << ex.what() << std::endl;
			}
			catch (...) {
				std::cerr << "[ThreadManager] �� �� ���� ���� �߻�" << std::endl;
			}
			});
	}

	void Stop()
	{
		mIsRunning = false;
	}

	void Join()
	{
		std::lock_guard guard(mLock);
		for (std::thread& t : mThreads)
		{
			if (t.joinable())
			{
				try {
					t.join();
				}
				catch (...) {
					std::cerr << "[ThreadManager] join �� ���� �߻�" << std::endl;
				}
			}
		}
		mThreads.clear();
	}

	bool IsRunning() const { return mIsRunning; }

	size_t GetThreadCount() const
	{
		std::lock_guard guard(mLock);
		return mThreads.size();
	}

private:
	mutable std::mutex mLock;
	std::vector<std::thread> mThreads;
	std::atomic<bool> mIsRunning{ true };
};
