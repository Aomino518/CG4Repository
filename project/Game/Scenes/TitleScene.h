#pragma once
#include "SeekerEngine.h"
#include "BaseScene.h"

class TitleScene : public BaseScene
{
public:
	// 初期化
	void Init() override;

	// 更新
	void Update() override;

	// 描画
	void Draw() override;

	void Shutdown() override;

private:
	std::unique_ptr<Sprite> sprite;
};

