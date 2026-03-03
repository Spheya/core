#pragma once

#include <cstdlib>
#include <string>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <d3d11.h>
#include <dcomp.h>
#include <dxgi.h>
#include <wrl/client.h>

#include "logger.hpp"

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

inline void fatalError(const char* message) {
	logger::error("{}", message);
	std::wstring wmessage = std::wstring(message, message + strlen(message));
	MessageBox(nullptr, wmessage.c_str(), L"Fatal Error", MB_OK | MB_ICONERROR);
	__debugbreak();
	std::abort();
}

inline void handleFatalError(HRESULT result, const char* message) {
	if(FAILED(result)) fatalError(std::format("DX11 Error: {:#08x}. \"{}\"", (unsigned long)result, message).c_str());
}

inline std::string wcharPtrToStr(wchar_t* wstr) {
	if(!wstr) return "";
	int size = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
	std::string result(size - 1, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, result.data(), size, nullptr, nullptr);
	return result;
}
