#pragma once

class SceneManager;
class BaseScene
{
public:
	virtual ~BaseScene() = default;

	// 初期化
	virtual void Init() = 0;

	// 更新
	virtual void Update() = 0;

	// 描画
	virtual void Draw() = 0;

	// 終了処理
	virtual void Shutdown() = 0;

	void SetSceneManager(SceneManager* mgr) { sceneManager_ = mgr; }

protected:
	SceneManager* sceneManager_ = nullptr;
};

