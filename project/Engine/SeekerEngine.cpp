#include "SeekerEngine.h"

void SeekerEngine::Init()
{
	Logger::Init();
	Logger::Write("アプリ開始");

	app_ = std::make_unique<Application>(1280, 720, L"CG2");
	app_->Init();

	// graphicsの初期化
	graphics_ = std::make_unique<Graphics>();
	graphics_->Init(app_->GetHWND(), app_->GetWidth(), app_->GetHeight(), true);

	// DxcCompilerの初期化
	dxcCompiler_.Init();

	//DirectInput初期化
	Input::GetInstance()->Init(app_.get());
	TextureManager::Init(graphics_.get());
	ModelManager::GetInstance()->Init(graphics_.get());

	// デバイスの生成がうまくいかなかったので起動できない
	assert(graphics_->GetDevice() != nullptr);
	// 初期化完了ログ
	Logger::Write("Complete Create D3D12Device!!!");

	// RootSignature作成
	rootSignatureFactory_.Init(graphics_.get());
	rs3D_ = rootSignatureFactory_.Create3D();
	rs2D_ = rootSignatureFactory_.Create2D();

	// SoundCommon作成
	soundCommon_ = std::make_unique<SoundCommon>();
	soundCommon_->Init();

	spriteCommon_ = std::make_unique<SpriteCommon>();
	entityCommon_ = std::make_unique<Entity3DCommon>();

	// スプライト共通部の作成
	spriteCommon_->Init(graphics_.get(), dxcCompiler_, rs2D_.Get());

	// モデル共通部の作成
	entityCommon_->Init(graphics_.get(), dxcCompiler_, rs3D_.Get());
}

void SeekerEngine::Update()
{
	Input::GetInstance()->Update();
}

void SeekerEngine::Shutdown()
{
	ModelManager::GetInstance()->Shutdown();
	TextureManager::Shutdown();

	Input::GetInstance()->Shutdown();

	soundCommon_->Shutdown();

	graphics_->Shutdown();

	Logger::Write("AppのShutdown");
	app_->Shutdown();

	Logger::Write("アプリ終了");
	Logger::Shutdown();
}

void SeekerEngine::BegineFrame()
{
	graphics_->BeginFrame();
}

void SeekerEngine::EndFrame()
{
	graphics_->EndFrame();
}
