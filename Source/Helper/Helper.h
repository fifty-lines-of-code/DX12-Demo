#pragma once

#define WIN32_LEAN_AND_MEAN 

#include <windows.h>
#include <string>

#ifndef ThrowException
#define ThrowException(hr__)                                        \
{                                                                     \
    std::wstring wfn = Helper::StringToWideString(__FILE__);          \
    throw DxException(hr__, L#hr__, wfn, __LINE__);                   \
}
#endif

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = Helper::StringToWideString(__FILE__);          \
    if(FAILED(hr__)) { throw DxException(x, L#x, wfn, __LINE__); } \
}                                                                     
#endif

class Helper {
public:
    static std::wstring StringToWideString(const std::string& s) {
        if (s.empty()) { return std::wstring(); }

        int sLength = (int)s.length() + 1;

        if (sLength < 512) {
            // use fast local buffer
            WCHAR stackBuffer[512];
            MultiByteToWideChar(CP_ACP, 0, s.c_str(), sLength, stackBuffer, 512);
            return std::wstring(stackBuffer);
        }

        // use slower heap
        int len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), sLength, 0, 0);
        std::wstring heapBuffer(len, L'\0');
        MultiByteToWideChar(CP_ACP, 0, s.c_str(), sLength, &heapBuffer[0], len);
        return heapBuffer;
    }
};

class DxException {
public: 
    DxException() = default;
    DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& filename, int lineNumber);

    std::wstring ToString()const;

    HRESULT ErrorCode = S_OK;
    std::wstring FunctionName;
    std::wstring FileName;
    int LineNumber = -1;
};
