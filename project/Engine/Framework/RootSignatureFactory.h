#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <d3dcompiler.h>
#include "Graphics.h"
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxcompiler.lib")

class RootSignatureFactory
{
public:
	void Init(Graphics* graphics);

	Microsoft::WRL::ComPtr<ID3D12RootSignature> Create2D();

	Microsoft::WRL::ComPtr<ID3D12RootSignature> Create3D();

private:

	Graphics* graphics_;
};

