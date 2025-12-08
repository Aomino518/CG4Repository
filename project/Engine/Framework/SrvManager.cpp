#include "SrvManager.h"
#include <assert.h>

SrvManager* SrvManager::instance_ = nullptr;

SrvManager* SrvManager::GetInstance()
{
	if (instance_ == nullptr) {
		instance_ = new SrvManager;
	}
	return instance_;
}

void SrvManager::Init(Graphics* graphics)
{
	this->graphics_ = graphics;
	device_ = graphics->GetDevice();
	// ディスクリプターヒープの生成
	descriptorHeap_ = graphics->CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMaxSRVCount, true);
	descriptorSize_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

uint32_t SrvManager::Allocate()
{
	assert(useIndex_ < kMaxSRVCount);

	if (!freeList_.empty()) {
		uint32_t index = freeList_.top();
		freeList_.pop();
		return index;
	}

	return useIndex_++;
}

void SrvManager::Free(uint32_t index)
{
	freeList_.push(index);
}

D3D12_CPU_DESCRIPTOR_HANDLE SrvManager::GetCPUDescriptorHandle(uint32_t index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize_ * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE SrvManager::GetGPUDescriptorHandle(uint32_t index)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap_->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize_ * index);
	return handleGPU;
}

void SrvManager::CreateSRVforTexture2D(uint32_t srvIndex, ID3D12Resource* pResource, DXGI_FORMAT format, UINT mipLevels)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(mipLevels);

	device_->CreateShaderResourceView(
		pResource,
		&srvDesc,
		GetCPUDescriptorHandle(srvIndex)
	);
}

void SrvManager::CreateSRVforStructuredBuffer(uint32_t srvIndex, ID3D12Resource* pResource, UINT numElements, UINT structureByteStride)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;  // StructuredBuffer は UNKNOWN
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;

	srvDesc.Buffer.NumElements = numElements;           // 要素数
	srvDesc.Buffer.StructureByteStride = structureByteStride; // １要素のサイズ
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.FirstElement = 0;

	device_->CreateShaderResourceView(
		pResource,
		&srvDesc,
		GetCPUDescriptorHandle(srvIndex)
	);
}

void SrvManager::PreDraw()
{
	// 描画用のDescriptorHeapの設定
	ID3D12DescriptorHeap* heaps[] = { descriptorHeap_.Get() };
	Graphics::GetCmdList()->SetDescriptorHeaps(_countof(heaps), heaps);
}

void SrvManager::SetGraphicsRootDescriptorTable(UINT rootPramatorIndex, uint32_t srvIndex)
{
	Graphics::GetCmdList()->SetGraphicsRootDescriptorTable(rootPramatorIndex, GetGPUDescriptorHandle(srvIndex));
}

void SrvManager::Shutdown()
{
	device_.Reset();
	descriptorHeap_.Reset();
	delete instance_;
	instance_ = nullptr;
}
