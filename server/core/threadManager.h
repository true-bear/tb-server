#pragma once
#include "pch.h"

class ThreadManager
{
public:
	void Run(std::function<void(std::stop_token)> callback)
	{
		std::lock_guard lock(mLock);
		mThreads.emplace_back(std::jthread([callback](std::stop_token st)
		{
			callback(st);
		}));
	}

	void Stop() 
	{
		std::lock_guard lock(mLock);
		for (auto& jt : mThreads) 
		{
			jt.request_stop();
		}
	}

	void Clear()
	{
		std::lock_guard guard(mLock);
		mThreads.clear();
	}

	size_t GetThreadCount() const
	{
		std::lock_guard guard(mLock);
		return mThreads.size();
	}

private:
	mutable std::mutex mLock;
	std::vector<std::jthread> mThreads;
};
