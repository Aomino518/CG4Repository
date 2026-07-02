#include "ModelManager.h"
#include "StringUtil.h"

ModelManager* ModelManager::GetInstance()
{
	static ModelManager instance;
	return &instance;
}

void ModelManager::Shutdown()
{
	models_.clear();
	Logger::Write("ModelManager Shutdown");
}

void ModelManager::Init()
{

}

void ModelManager::LoadModel(const std::string& fileName, const std::string& filePath)
{
	if (models_.contains(filePath)) {
		return;
	}

	std::vector<std::string> filePaths = Split(filePath, '.');
	
	// モデル生成とファイル読み込み、初期化
	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->Init("resources/models", fileName + "/" + filePaths[0], filePaths[1]);

	// モデルをマップコンテナに格納する
	models_.insert(std::make_pair(filePaths[0], std::move(model)));
}

Model* ModelManager::FindModel(const std::string& filePath)
{
	// 読み込みモデルを検索
	if (models_.contains(filePath)) {
		return models_.at(filePath).get();
	}
	return nullptr;
}

void ModelManager::SetIsLighting(bool isLighting) {
	isModelLighting_ = isLighting;
	for (auto& model : models_) {
		if (model.second) {
			model.second->SetIsLighting(isModelLighting_);
		}
	}
}