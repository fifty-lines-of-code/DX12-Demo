#include "DX12Demo.h"
#include <cassert>
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
    PSTR cmdLine, int showCmd)
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    try
    {
        HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (FAILED(hr)) {
            // Handle failure if COM completely fails to spin up
            return -1;
        }
        auto theDemo = std::make_unique<DX12Demo>(hInstance);
        if (!theDemo->Initialize())
            return 0;

        theDemo->Run();
        CoUninitialize();
        return 0;
    }
    catch (DxException& e)
    {
        MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
        return 0;
    }
}