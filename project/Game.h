#pragma once
#include "SeekerEngine.h"
#include "SEFramework.h"

class Game : public SEFramework
{
public:
	void Init() override;
	void Shutdown() override;
	void Update() override;
	void Draw() override;

private:
    std::unique_ptr<Sprite> sprite;
    std::unique_ptr<Entity3D> entity;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<Particle3DCommon> paticleCommon_;
 
    std::unique_ptr<Sound> bgm;
    std::unique_ptr<Sound> se;
    SoundData sHAudio1, sHAudio2, sHAudio3, sHAudio4;

    std::unique_ptr<CameraManager> cameraManager;

    ImGuiManager imgui;

};

