#include "SeekerEngine.h"

void SeekerEngine::Init()
{
	StartupManager::Init();
	Logger::Init();
	Logger::Write("アプリ開始");

	app_ = std::make_unique<Application>(1280, 720, L"CG2");
	app_->Init();

	// graphicsの初期化
	graphics_ = std::make_unique<Graphics>();
	graphics_->Init(app_->GetHWND(), app_->GetWidth(), app_->GetHeight(), true);

	SrvManager::GetInstance()->Init(graphics_.get());

	// DxcCompilerの初期化
	dxcCompiler_.Init();

	//DirectInput初期化
	Input::GetInstance()->Init(app_.get());
	TextureManager::GetInstance()->Init(graphics_.get());
	ModelManager::GetInstance()->Init(graphics_.get());

	// RootSignature作成
	rootSignatureFactory_.Init(graphics_.get());
	rs3D_ = rootSignatureFactory_.Create3D();
	rs2D_ = rootSignatureFactory_.Create2D();
	rsParticle_ = rootSignatureFactory_.CreateParticle3D();

	// SoundCommon作成
	soundCommon_ = std::make_unique<SoundCommon>();
	soundCommon_->Init();

	spriteCommon_ = std::make_unique<SpriteCommon>();
	entityCommon_ = std::make_unique<Entity3DCommon>();

	// スプライト共通部の作成
	spriteCommon_->Init(graphics_.get(), dxcCompiler_, rs2D_.Get());

	// モデル共通部の作成
	entityCommon_->Init(graphics_.get(), dxcCompiler_, rs3D_.Get());

	particleCommon_ = std::make_unique<Particle3DCommon>();
	particleCommon_->Init(graphics_.get(), dxcCompiler_, rsParticle_.Get());
}

void SeekerEngine::Update()
{
	Input::GetInstance()->Update();
}

void SeekerEngine::Shutdown()
{
	ModelManager::GetInstance()->Shutdown();
	TextureManager::GetInstance()->Shutdown();

	Input::GetInstance()->Shutdown();

	soundCommon_->Shutdown();

	SrvManager::GetInstance()->Shutdown();
	graphics_->Shutdown();

	Logger::Write("AppのShutdown");
	app_->Shutdown();

	Logger::Write("アプリ終了");
	Logger::Shutdown();
	StartupManager::Shutdown();
}

void SeekerEngine::BegineFrame()
{
	graphics_->BeginFrame();
}

void SeekerEngine::EndFrame()
{
	graphics_->EndFrame();
}