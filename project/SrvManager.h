#pragma once
#include "Graphics.h"
#include <stack>

class SrvManager
{
public:
	// 最大SRV数 (最大テクスチャ枚数)
	static constexpr uint32_t kMaxSRVCount = 4096;

	// シングルトンインスタンスの取得
	static SrvManager* GetInstance();

	void Init(Graphics* graphics);

	uint32_t Allocate();
	void Free(uint32_t index);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t index);

	// SRV生成テクスチャ用
	void CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT format, UINT mipLevels);
	// SRV生成Structured Buffer用
	void CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride);

	void PreDraw();

	void SetGraphicsRootDescriptorTable(UINT rootPramatorIndex, uint32_t srvIndex);

	ID3D12DescriptorHeap* GetDiscriptorHeap() { return descriptorHeap_.Get(); }

	void Shutdown();

private:
	static SrvManager* instance_;
	SrvManager() = default;
	~SrvManager() = default;
	SrvManager(const SrvManager&) = delete;
	SrvManager& operator=(const SrvManager&) = delete;

	Graphics* graphics_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Device> device_ = nullptr;
	// SRV用ディスクリプタ―サイズ
	uint32_t descriptorSize_;
	// SRV用ディスクリプターヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_;
	// 次に使用するSRVインデックス
	uint32_t useIndex_ = 1;
	std::stack<uint32_t> freeList_;
};

