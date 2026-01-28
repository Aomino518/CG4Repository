#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <cassert>
#ifdef USE_IMGUI
#include "externals/imgui/imgui_impl_win32.h"
#endif

class Application
{
public:
	static Application* GetInstance();

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

	void Init(int width, int height, const wchar_t* title); // ウィンドウ作成
	bool ProcessMessage();  // メッセージポンプ
	void Shutdown();		// 明示終了

	// アクセッサ
	HINSTANCE GetHInstance() const { return hInstance_; }
	HWND GetHWND() const { return hwnd_; }
	static int GetWidth() { return clientWidth_; }
	static int GetHeight() { return clientHeight_; }

private:
	Application() = default;
	~Application() = default;
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

	HINSTANCE hInstance_ = nullptr;
	HWND hwnd_ = nullptr;
	WNDCLASS wndclass{};
	std::wstring className_ = L"CG2WindowClass";
	std::wstring title_ = L"CG2";
	static int clientWidth_;
	static int clientHeight_;
	DWORD style_ = WS_OVERLAPPEDWINDOW;
};

