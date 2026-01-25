#pragma once
#include "SeekerEngine.h"
#include <TitleScene.h>
#include "SceneManager.h" 
#include "BaseScene.h"

class PlayScene : public BaseScene
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
	std::unique_ptr<Entity3D> entity;
	std::unique_ptr<Entity3D> modelTerrain;
	std::unique_ptr<Camera> camera;
	//std::unique_ptr<ParticleEmitter> emitter_;

	//std::unique_ptr<Sound> bgm;
	//std::unique_ptr<Sound> se;
	//SoundData sHAudio1, sHAudio2, sHAudio3, sHAudio4;

	std::unique_ptr<CameraManager> cameraManager;

};

