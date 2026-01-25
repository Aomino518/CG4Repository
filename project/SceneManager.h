#pragma once
#include "SeekerEngine.h"
#include "BaseScene.h"
#include "TitleScene.h"
#include "PlayScene.h"
#include "AbstractSceneFactory.h"

class SceneManager
{
public:
	static SceneManager* GetInstance();

	void SetNextScene(std::unique_ptr<BaseScene> nextScene) { nextScene_ = std::move(nextScene); }

	void Update();

	void Draw();

	// setter
	void SetSceneFactory(std::unique_ptr<AbstractSceneFactory> sceneFactory) { this->sceneFactory_ = std::move(sceneFactory); }

	SceneManager();
	~SceneManager();

private:

	std::unique_ptr<BaseScene> scene_;
	std::unique_ptr<BaseScene> nextScene_;
	std::unique_ptr<AbstractSceneFactory> sceneFactory_ = nullptr;
};

