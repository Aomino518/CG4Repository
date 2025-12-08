#include "SEFramework.h"

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

	// ウィンドウの×ボタンが押されるまでループ
	while (engine_.GetApp()->ProcessMessage()) {
		engine_.Update();
		Update();

		if (IsEndRequst()) {
			break;
		}

		/*-- 描画処理 --*/
		engine_.BegineFrame();
		Draw();
		engine_.EndFrame();
	}

	Shutdown();
	engine_.Shutdown();
}
