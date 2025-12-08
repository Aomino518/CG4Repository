#pragma once
#include "ModelCommon.h"
#include <d3d12.h>
#include <wrl.h>
#include "CreateResorceUtils.h"
#include <string>
#include <vector>
#include <cassert>
#include <unordered_map>
#include <algorithm>
#include "Matrix.h"

class Model
{
public:
	void Init(ModelCommon* modelCommon, const std::string& directoryPath, const std::string& filename);

	void Draw();

	/// <summary>
	/// mtlファイルの読み取り
	/// </summary>
	/// <param name="directoryPath">ディレクトリパス</param>
	/// <param name="filename">ファイル名</param>
	/// <returns>マテリアルデータ</returns>
	static MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

	void LoadObjFile(const std::string& directoryPath, const std::string& filename);

	Vector4& GetMaterial() const { return materialData_->color; }
	bool GetIsLighting() const { return materialData_->enableLighting; }

	void SetMaterial(const Vector4& material) { this->materialData_->color = material; }
	void SetIsLighting(const bool isLighting) { this->materialData_->enableLighting = isLighting; }

private:
	void CreateBufferResources();
	void MaterialInit();

	ModelCommon* modelCommon_;

	// マテリアルデータ
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr;
	Material* materialData_ = nullptr;
	// モデルデータ
	ModelData modelData_ = {};

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	VertexData* vertexData_ = nullptr;
	uint32_t* indexData_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU_;

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList_;
};

