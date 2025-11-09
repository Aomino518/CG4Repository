#include "ModelManager.h"

ModelManager* ModelManager::instance_ = nullptr;

ModelManager* ModelManager::GetInstance()
{
	if (instance_ == nullptr) {
		instance_ = new ModelManager;
	}
	return instance_;
}

void ModelManager::Shutdown()
{
	delete instance_;
	instance_ = nullptr;
}

void ModelManager::Init(Graphics* graphics)
{
	modelCommon_->Init(graphics);
}

void ModelManager::LoadModel(const std::string& filePath)
{
	if (models_.contains(filePath)) {
		return;
	}

	// モデル生成とファイル読み込み、初期化
	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->Init(modelCommon_.get(), "resources", filePath);

	// モデルをマップコンテナに格納する
	models_.insert(std::make_pair(filePath, std::move(model)));
}

Model* ModelManager::FindModel(const std::string& filePath)
{
	// 読み込みモデルを検索
	if (models_.contains(filePath)) {
		return models_.at(filePath).get();
	}
	return nullptr;
}
