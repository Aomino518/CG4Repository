#pragma once
#include "BaseScene.h"

class ClearScene : public BaseScene
{
	// 初期化
	void Init() override;

	// 更新
	void Update() override;

	// 描画
	void Draw() override;

	// 終了
	void Shutdown() override;

private:

};

