#include "SEFramework.h"
#include <SceneFactory.h>
#include "SceneManager.h"
#include "Application.h"

void SEFramework::Init()
{
}

void SEFramework::Shutdown()
{
}

void SEFramework::Update()
{
}

void SEFramework::Run()
{
	engine_.Init();

	Init();

	sceneFactory_ = std::make_unique<SceneFactory>();
	SceneManager::GetInstance()->SetSceneFactory(std::move(sceneFactory_));
	SceneManager::GetInstance()->ChangeScene("TITLE");

	// ウィンドウの×ボタンが押されるまでループ
	while (engine_.GetApp()->ProcessMessage()) {
		engine_.Update();
		Update();
		SceneManager::GetInstance()->Update();

		if (IsEndRequst()) {
			break;
		}

		/*-- 描画処理 --*/
		engine_.BegineFrame();
		Draw();
		SceneManager::GetInstance()->Draw();
		engine_.EndFrame();
	}

	Shutdown();
	engine_.Shutdown();
}
