#pragma once

#include "Helper.h"
#include <string>
#include <vector>
#include <windows.h>

class Logger {
public:
	static void PRINT(std::wstring msg) {
#if defined(_DEBUG)
		std::wstring output = Logger::DEBUG + L"_PRINT: " + msg;
		OutputDebugString(output.c_str());
#endif
	}

	static void ERR(std::wstring msg) {
#if defined(_DEBUG)
		std::wstring output = Logger::DEBUG + Logger::_ERROR +
			msg +
			L"(File: " + Helper::StringToWideString(__FILE__) +
			L", Line: " + std::to_wstring(__LINE__);
		OutputDebugString(output.c_str());
#endif
	}

private:
	inline static const std::wstring DEBUG = L"DEBUG";
	inline static const std::wstring _ERROR = L"_ERROR: ";
};