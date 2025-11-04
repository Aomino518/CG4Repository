#pragma once
#include "ModelCommon.h"
#include "Model.h"
#include <unordered_map>
#include <string>
#include <cassert>
#include <wrl.h>
#include <vector>
#include <map>
#include "Graphics.h"

class ModelManager
{
public:
	// シングルトンインスタンスの取得
	static ModelManager* GetInstance();
	// 終了
	void Shutdown();

	void Init(Graphics* graphics);

	/// <summary>
	/// モデルファイルの読み込み
	/// </summary>
	/// <param name="filePath">ファイルパス</param>
	void LoadModel(const std::string& filePath);

	/// <summary>
	/// モデル検索
	/// </summary>
	/// <param name="filePath">ファイルパス</param>
	/// <returns>モデル</returns>
	Model* FindModel(const std::string& filePath);

private:
	static ModelManager* instance_;
	
	ModelManager() = default;
	~ModelManager() = default;
	ModelManager(const ModelManager&) = delete;
	ModelManager& operator=(const ModelManager&) = delete;

	// モデルデータ
	std::map<std::string, std::unique_ptr<Model>> models_;
	std::unique_ptr<ModelCommon> modelCommon_ = std::make_unique<ModelCommon>();
};

