#pragma once
#include "Application.h"
#include "Logger.h"
#include "Matrix.h"
#include "DebugCamera.h"
#include "StringUtil.h"
#include "Input.h"
#include "Sound.h"
#include "DxcCompiler.h"
#include "RootSignatureFactory.h"
#include "InputLayout.h"
#include "PsoBuilder.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "TextureManager.h"
#include "Entity3DCommon.h"
#include "Entity3D.h"
#include "ModelManager.h"
#include "ImGuiManager.h"
#include "Camera.h"
#include "StartupManager.h"
#include "SrvManager.h"
#include "ParticleManager.h"
#include "ParticleEmitter.h"
#include "LightManager.h"
#include "SceneManager.h"
#include "SoundManager.h"
#include "CameraManager.h"
#include "EmitterManager.h"
#include "WorldFieldManager.h"

class SeekerEngine
{
public:
	void Init();
	void Update();
	void Shutdown();

	void BegineFrame();
	void EndFrame();

	// Getter
	Application* GetApp() const { return Application::GetInstance(); }
	Graphics* GetGraphics() const { return Graphics::GetInstance(); }
	DxcCompiler GetDxcCompiler() const { return dxcCompiler_; }
	RootSignatureFactory GetRootSig() const { return rootSignatureFactory_; }

private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rs3D_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rs2D_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rsParticle_;

	DxcCompiler dxcCompiler_;
	RootSignatureFactory rootSignatureFactory_;
	InputLayout inputLayout_;
	PsoBuilder psoBuilder_;

};