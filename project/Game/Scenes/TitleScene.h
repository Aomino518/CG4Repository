#pragma once
#include "SceneIncludes.h"
#include "BaseScene.h"
#include "Particle2DEmitter.h"

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

	const char* GetSceneName() const override { return "TITLE"; }

	void UpdatePlayer();
	void ResolveTerrainCollision();
	void UpdateThirdPersonCamera();

private:
	std::unique_ptr<Entity3D> modelTerrain_;
	std::unique_ptr<Entity3D> simpleSkin_;
	uint32_t tHTex_;

	float moveSpeed_ = 0.12f;
	float playerHalfHeight_ = 0.9f;

	// 三人称カメラ
	Vector3 cameraOffset_ = { 0.0f, 3.0f, -7.0f };
	float cameraPitch_ = 0.3f;
};