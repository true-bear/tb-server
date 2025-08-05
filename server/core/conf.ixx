module;
#include <windows.h>

export module util.conf;
import <string>;

export namespace Config
{
	inline int Load(const wchar_t* category, const wchar_t* optName)
	{
		wchar_t buf[100] = { 0 };

		DWORD result = GetPrivateProfileStringW(category, optName, L"-1", buf, 100, L"..\\network.ini");
		std::wstring ResultString(buf, result);

		return std::stoi(ResultString);
	}
}
