#pragma once
#include <string>
#include <vector>
#include <d3d12.h>
#include <wrl.h>
#include <cassert>  
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix.h"
#include <unordered_map>
#include <algorithm>
#include "CreateResorceUtils.h"

class Entity3DCommon;
class TextureManager;

class Entity3D
{
public:
	void Init(Entity3DCommon* entity3DCommon, const std::string& directoryPath, const std::string& filename);

	void Update();

	void Draw();

	/// <summary>
	/// mtlファイルの読み取り
	/// </summary>
	/// <param name="directoryPath">ディレクトリパス</param>
	/// <param name="filename">ファイル名</param>
	/// <returns>マテリアルデータ</returns>
	static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

	void LoadObjFile(const std::string& directoryPath, const std::string& filename);

private:
	void CreateBufferResources();
	void ModelResourcesSetting();

	Transform transform_;
	Transform cameraTransform_;

	Entity3DCommon* entity3DCommon_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr;
	Material* materialData_ = nullptr;
	ModelData modelData_ = {};

	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_ = nullptr;
	TransformationMatrix* wvpData_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_ = nullptr;
	DirectionalLight* directionalLightData_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	VertexData* vertexData_ = nullptr;
	uint32_t* indexData_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU_;

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList_;
};

