#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <assert.h>
#include "Application.h"
#include <wrl.h>

class Input
{
public:
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
	static Input* GetInstance();

	void Init(Application* app);
	void Shutdown();

	void Update();

	bool IsDown(int dik) const;
	bool IsPressed(int dik) const;
	bool IsReleased(int dik) const;

private:
	// 内部用
	Input() = default;
	~Input() = default;
	Input(const Input&) = delete;
	Input& operator=(const Input&) = delete;

	ComPtr<IDirectInput8> directInput_ = nullptr;
	ComPtr<IDirectInputDevice8> keyboard_ = nullptr;
	BYTE key[256] = {};
	BYTE preKey[256] = {};
	Application* app_ = nullptr;
};

