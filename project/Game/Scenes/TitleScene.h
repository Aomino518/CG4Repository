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

private:
	std::unique_ptr<Entity3D> entity_;
	std::unique_ptr<Entity3D> modelTerrain_;
	uint32_t tHTex_;
	uint32_t tHCircle_;
	uint32_t tHGradationLine_;

	// パーティクル設定
	ParticleConfig hitEffectConfig_;
	ParticleConfig hitRingEffectConfig_;
	ParticleConfig cylinderEffectConfig_;
};