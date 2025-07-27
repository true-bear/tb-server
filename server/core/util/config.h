#pragma once
#include "../pch.h"


namespace Config
{
	static int Load(const wchar_t* category, const wchar_t* optName)
	{
		wchar_t buf[100] = { 0, };

		auto result = GetPrivateProfileString(category, optName, L"-1", buf, 100, L"..\\network.ini");
		std::wstring ResultString(buf, result);

		return std::stoi(ResultString);
	}
}

