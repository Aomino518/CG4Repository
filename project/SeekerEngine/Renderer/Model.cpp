#include "Model.h"
#include "TextureManager.h"
#include <fstream>
#include "Logger.h"
#include <filesystem>

void Model::Init(ModelCommon* modelCommon, const std::string& directoryPath, const std::string& filename)
{
	modelCommon_ = modelCommon;
	cmdList_ = modelCommon->GetGraphics()->GetCmdList();
	LoadObjFile(directoryPath, filename);
	CreateBufferResources();
	MaterialInit();
	modelData_.material.textureIndex = TextureManager::GetInstance()->Load(modelData_.material.textureFilePath);
	textureSrvHandleGPU_ = TextureManager::GetInstance()->GetGPUHandle(modelData_.material.textureIndex);
}

void Model::Draw()
{
	cmdList_->IASetVertexBuffers(0, 1, &vertexBufferView_); // VBVを設定
	cmdList_->IASetIndexBuffer(&indexBufferView_);
	// 形状を設定。PSOに設定しているものとはまた別。同じものを設定する。
	cmdList_->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
	cmdList_->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU_);
	// 描画 (DrawCall)。
	cmdList_->DrawIndexedInstanced(UINT(modelData_.indices.size()), 1, 0, 0, 0);
}

MaterialData Model::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename)
{
	// 必要な変数宣言とファイルを開く
	MaterialData materialData; // 構築するMaterialData
	std::string line; // ファイルから読んだ1行を格納するもの
	std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
	assert(file.is_open()); // 開けなかったら止める

	// ファイルを読み、MaterialDataを構築
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		// identifierに応じた処理
		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			// 連結してファイルパスにする
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}

	return materialData;
}

void Model::LoadObjFile(const std::string& directoryPath, const std::string& filename)
{
	std::vector<Vector4> positions; // 位置
	std::vector<Vector3> normals; // 法線
	std::vector<Vector2> texcoords; // テクスチャ座標
	std::string line; // ファイルから読んだ1行を格納するもの
	std::filesystem::path objPath = std::filesystem::path(directoryPath) / (filename + ".obj");

	if (!std::filesystem::exists(objPath)) {
		objPath = std::filesystem::path(directoryPath) / filename / (filename + ".obj");
	}

	assert(std::filesystem::exists(objPath));

	// OBJ のディレクトリ
	std::string modelDir = objPath.parent_path().string();

	std::ifstream file(objPath); // ファイルを開く
	assert(file.is_open());

	std::unordered_map<TripletKey, uint32_t, TripletHash> lut;

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier; // 先頭の識別子を読む

		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			positions.push_back(position);
		} else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoords.push_back(texcoord);
		} else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);
		} else if (identifier == "f") {
			//VertexData triangle[3];

			struct FaceElm { uint32_t v, t, n; } f[3]{};

			// 面は三角形限定。その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				std::replace(vertexDefinition.begin(), vertexDefinition.end(), '/', ' ');
				// 頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してIndexを取得する
				std::istringstream v(vertexDefinition);

				v >> f[faceVertex].v >> f[faceVertex].t >> f[faceVertex].n;
				f[faceVertex].v--;
				f[faceVertex].t--;
				f[faceVertex].n--;
			}

			for (int order : {2, 1, 0}) {
				TripletKey key{ f[order].v, f[order].t, f[order].n };
				auto it = lut.find(key);
				uint32_t idx;
				if (it == lut.end()) {
					// 頂点生成
					Vector4 p = positions[key.v];
					Vector2 t = texcoords[key.vt];
					Vector3 n = normals[key.vn];

					p.x *= -1.0f;
					t.y = 1.0f - t.y;
					n.x *= -1.0f;

					VertexData vtx{ p, t, n };
					idx = (uint32_t)modelData_.vertices.size();
					modelData_.vertices.push_back(vtx);
					lut.emplace(key, idx);
				} else {
					idx = it->second;
				}
				modelData_.indices.push_back(idx);
			}

		} else if (identifier == "mtllib") {
			// materialTemplateLibraryファイルの名前を取得する
			std::string materialFilename;
			s >> materialFilename;
			// 基本的にobjファイルと同一層にmtlは存在させるので、ディレクトリ名とファイル名を残す
			modelData_.material = LoadMaterialTemplateFile(modelDir, materialFilename);
		}
	}
}

void Model::CreateBufferResources()
{
	// 頂点リソース
	vertexResource_ = CreateBufferResource(Graphics::GetDevice(), sizeof(VertexData) * modelData_.vertices.size());
	// 頂点バッファビューを作成する
	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点3つ分のサイズ
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData_.vertices.size());
	// 1頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
	Logger::Write("モデルのVertexResource生成完了");

	indexResource_ = CreateBufferResource(Graphics::GetDevice(), sizeof(uint32_t) * modelData_.indices.size());
	D3D12_INDEX_BUFFER_VIEW indexBufferViewModel{};
	// リソースの先頭のアドレスから使う
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = UINT(sizeof(uint32_t) * modelData_.indices.size());
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
	Logger::Write("モデルのindexResource生成完了");

	// モデル用の頂点リソースにデータを書き込む
	// 書き込むためのアドレスを取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	std::memcpy(vertexData_, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
	vertexResource_->Unmap(0, nullptr);
	vertexData_ = nullptr;
	Logger::Write("モデルのVertexData書き込み完了");

	// モデル用の頂点リソースにデータを書き込む
	// 書き込むためのアドレスを取得
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));
	std::memcpy(indexData_, modelData_.indices.data(), sizeof(uint32_t) * modelData_.indices.size());
	indexResource_->Unmap(0, nullptr);
	indexData_ = nullptr;
	Logger::Write("モデルのindexDataに書き込み完了");
}

void Model::MaterialInit()
{
	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
	materialResource_ = CreateBufferResource(Graphics::GetDevice(), sizeof(Material));
	// 書き込むためのアドレスを取得
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData_->uvTransform = MakeIdentity4x4();
	materialData_->enableLighting = true;
}
