#pragma once
#include <wrl.h>
#include <d3d12.h>
#include "DxcCompiler.h"
#include "InputLayout.h"
#include "PsoBuilder.h"
#include "BlendStateUtils.h"
#include "Color.h"
#include "CreateResorceUtils.h"
#include "CameraManager.h"
#include "TextureManager.h"
#include <cstdint>

class Skybox
{
public:
	static Skybox* GetInstance();
	void Init(DxcCompiler dxcCompiler, ID3D12RootSignature* rootSignature);
	void Update();
	void Draw();
	void Shutdown();

	void SetTexture(uint32_t textureId);

private:
	Skybox() = default;
	~Skybox() = default;
	Skybox(const Skybox&) = delete;
	Skybox& operator=(const Skybox&) = delete;

	void CreateGraphicPipeline(DxcCompiler dxcCompiler);
	void CreateVertexBuffer();
	void CreateTransformationMatrixResource();

	// 他クラスファイルからの取得変数
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Graphics* graphics_;

	// シェーダーBlob
	Microsoft::WRL::ComPtr<IDxcBlob> vsBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> psBlob_;

	// PSO
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pso_;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList_;
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc_{};

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	VertexData* vertexData_ = nullptr;
	//uint32_t vertexCount_ = 0;

	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	uint32_t* indexData_ = nullptr;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;

	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	Material* materialData_ = nullptr;

	// BlendMode
	D3D12_BLEND_DESC blendDesc_{};
	BlendMode mode_ = kBlendModeNone;

	// TransformationMatrix
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;
	TransformationMatrix* transformationMatrixData_ = nullptr;

	// カメラ
	Camera* camera_ = nullptr;
	DebugCamera* debugCamera_ = nullptr;
	CameraManager* cameraManager_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource = nullptr;
	CameraForGPU* cameraData_ = nullptr;

	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU_{};
	// テクスチャ番号
	uint32_t textureIndex_ = 0;
};

