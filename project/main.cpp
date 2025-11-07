#include "Application.h"
#include "Logger.h"
#include <format>
#include <dbghelp.h>
#include <strsafe.h>
#include <sstream>
#include "Matrix.h"
#include "DebugCamera.h"
#define _USE_MATH_DEFINES 
#include <math.h>
#include "StringUtil.h"
#include "Input.h"
#include "SoundCommon.h"
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
#include <algorithm>
#pragma comment(lib, "Dbghelp.lib")

#pragma region 自作関数
static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception) {
	// 時刻を取得して、時刻をなめに入れたファイルを作成、Dumpsディレクトリ以下に出力
	SYSTEMTIME time;
	GetLocalTime(&time);
	wchar_t filePath[MAX_PATH] = { 0 };
	CreateDirectory(L"./Dumps", nullptr);
	StringCchPrintfW(filePath, MAX_PATH, L"./Dumps/%04d-%02d%02d-%02d%02d.dmp", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMilliseconds);
	HANDLE dumpFileHandle = CreateFile(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
	// processId(このexeのId)とクラッシュ(例外)の発生したthreadIdを取得
	DWORD processId = GetCurrentProcessId();
	DWORD threadId = GetCurrentThreadId();
	// 設定情報入力
	MINIDUMP_EXCEPTION_INFORMATION minidumpInfomation{ 0 };
	minidumpInfomation.ThreadId = threadId;
	minidumpInfomation.ExceptionPointers = exception;
	minidumpInfomation.ClientPointers = TRUE;
	// Dumpを出力。MiniDumpNormalは最低限の情報を出力するフラグ
	MiniDumpWriteDump(GetCurrentProcess(), processId, dumpFileHandle, MiniDumpNormal, &minidumpInfomation, nullptr, nullptr);
	// 他に関連づけられているSEH例外ハンドラがあれば実行。通常はプロセスを終了する。
	return EXCEPTION_EXECUTE_HANDLER;
}

void WriteSphereVertices(const uint32_t subdivision, VertexData* vertexData) {
	const float kLonEvery = 2.0f * float(M_PI) / float(subdivision); // 経度
	const float kLatEvery = float(M_PI) / float(subdivision); // 緯度
	// 緯度の方向に分割 -π/2 ~ π/2
	for (uint32_t latIndex = 0; latIndex <= subdivision; ++latIndex) {
		float lat = -float(M_PI) / 2.0f + kLatEvery * latIndex;

		// 経度の方向に分割 0 ~ 2π
		for (uint32_t lonIndex = 0; lonIndex <= subdivision; ++lonIndex) {
			float lon = lonIndex * kLonEvery; // 現在の経度kLonEvery

			Vector4 pos = {
				cos(lat) * cos(lon),
				sin(lat),
				cos(lat) * sin(lon),
				1.0f
			};

			// UV座標
			Vector2 uv = {
				lon / (2.0f * float(M_PI)),
				1.0f - (lat + float(M_PI) / 2.0f) / float(M_PI)
			};

			uint32_t index = latIndex * (subdivision + 1) + lonIndex;
			vertexData[index] = {
				pos,
				uv,
				Normalize(Vector3(pos.x, pos.y, pos.z))
			};

		}
	}
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	D3DResourceLeakChecker leakCheck;
	CoInitializeEx(0, COINIT_MULTITHREADED);
	SetUnhandledExceptionFilter(ExportDump);
	Logger::Init();
	Logger::Write("アプリ開始");

	std::unique_ptr<Application> app = std::make_unique<Application>(1280, 720, L"CG2");
	app->Init();
	if (!app) {
		Logger::Write("App 初期化失敗");
		Logger::Shutdown();
		return false;
	}

	Graphics graphics;
	DxcCompiler dxcCompiler;
	RootSignatureFactory rootSignatureFactory;
	InputLayout inputLayout;
	PsoBuilder psoBuilder;
	ImGuiManager imgui;
	
	// graphicsの初期化
	graphics.Init(app->GetHWND(), app->GetWidth(), app->GetHeight(), true);
	imgui.Init(app.get(), &graphics);

	TextureManager::Init(&graphics);
	ModelManager::GetInstance()->Init(&graphics);

	// XAudio2の初期化
	std::unique_ptr<SoundCommon> soundCommon = std::make_unique<SoundCommon>();
	soundCommon->Init();
	std::unique_ptr<Sound> bgm = std::make_unique<Sound>();
	std::unique_ptr<Sound> se = std::make_unique<Sound>();
	bgm->SetCommon(soundCommon.get());
	se->SetCommon(soundCommon.get());

	//DirectInput初期化
	Input::GetInstance()->Init(app.get());

	// デバイスの生成がうまくいかなかったので起動できない
	assert(graphics.GetDevice() != nullptr);
	// 初期化完了ログ
	Logger::Write("Complete Create D3D12Device!!!");

	// DxcCompilerの初期化
	dxcCompiler.Init();

	// RootSignature作成
	rootSignatureFactory.Init(&graphics);
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rs3D = rootSignatureFactory.Create3D();
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rs2D = rootSignatureFactory.Create2D();

	std::unique_ptr<SpriteCommon> spriteCommon = std::make_unique<SpriteCommon>();
	std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();

	std::unique_ptr<Entity3DCommon> entityCommon = std::make_unique<Entity3DCommon>();
	std::unique_ptr<Entity3D> entity = std::make_unique<Entity3D>();
	ModelManager::GetInstance()->LoadModel("axis.obj");

	std::unique_ptr<Camera> camera = std::make_unique<Camera>();
	Vector3 cameraPos = { 0.0f, 0.0f, -10.0f };
	Vector3 cameraRotate = { 0.0f, 0.0f, 0.0f };
	camera->SetRotate(cameraRotate);
	camera->SetTranslate(cameraPos);
	entityCommon->SetDefaultCamera(camera.get());

	// スプライト共通部の作成
	spriteCommon->Init(&graphics, dxcCompiler, rs2D.Get());

	// モデル共通部の作成
	entityCommon->Init(&graphics, dxcCompiler, rs3D.Get());
	entity->Init(entityCommon.get());
	entity->SetModel("axis.obj");
	entity->SetTranslate(Vector3(0.0f, 0.0f, 0.0f));

	Vector4 spriteMaterial = { 1.0f, 1.0f, 1.0f, 1.0f };
	Vector2 positoin = {0.0f, 0.0f};
	Vector2 scale = { 100.0f, 100.0f };
	float rotation = 0.0f;

	uint32_t tHChecker = TextureManager::Load("resources/uvChecker.png");


	/*const uint32_t kSubdivision = 16; // 16分割

	//uint32_t vertexNum = (kSubdivision + 1) * (kSubdivision + 1);
	//uint32_t indexNum = kSubdivision * kSubdivision * 6;*/

	/*--Index用リソース作成--*/
	//Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceModel = CreateBufferResource(graphics.GetDevice(), sizeof(uint32_t) * 6);

	//uint32_t index = 0;
	//uint32_t* indexDataModel = nullptr;
	//indexResourceModel->Map(0, nullptr, reinterpret_cast<void**>(&indexDataModel));
	//std::memcpy(indexDataModel, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());

	/*for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			uint32_t ld = lonIndex + latIndex * (kSubdivision + 1);
			uint32_t lt = lonIndex + (latIndex + 1) * (kSubdivision + 1);
			uint32_t rd = (lonIndex + 1) + latIndex * (kSubdivision + 1);
			uint32_t rt = (lonIndex + 1) + (latIndex + 1) * (kSubdivision + 1);

			indexDataModel[index++] = rt;
			indexDataModel[index++] = ld;
			indexDataModel[index++] = lt;

			indexDataModel[index++] = rd;
			indexDataModel[index++] = ld;
			indexDataModel[index++] = rt;
		}
	}*/

	Transform uvTransformSprite = {
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f},
	};

	// 初期色 (RGBA)
	float modelColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	// 音声読み込み
	SoundData sHAudio1 = bgm->SoundLoad("resources/c21.mp3");
	SoundData sHAudio2 = bgm->SoundLoad("resources/koharubiyori.mp3");
	SoundData sHAudio3 = se->SoundLoad("resources/gold.mp3");
	SoundData sHAudio4 = se->SoundLoad("resources/se_itemget.wav");
	Logger::Write("音声読み込み");
	DebugCamera debugCamera;
	debugCamera.Initialize();

	sprite->Create(tHChecker, positoin, Color::WHITE);
	
	sprite->SetRotation(rotation);
	Vector4 materialColor = sprite->GetColor();
	float rotateModel = 0.0f;

	// ウィンドウの×ボタンが押されるまでループ
	while (app->ProcessMessage()) {
		imgui.BegineFrame();
		Input::GetInstance()->Update();

		/*-- 更新処理 --*/
		if (Input::GetInstance()->IsPressed(DIK_SPACE)) {
			bgm->SoundPlay(sHAudio1, false);
		}

		if (Input::GetInstance()->IsPressed(DIK_N)) {
			bgm->SoundPlay(sHAudio2, false);
		}

		if (Input::GetInstance()->IsPressed(DIK_M)) {
			se->SoundPlay(sHAudio3, false);
		}

		if (Input::GetInstance()->IsPressed(DIK_V)) {
			se->SoundPlay(sHAudio4, false);
		}

		if (Input::GetInstance()->IsPressed(DIK_B)) {
			bgm->SoundStop();
			se->SoundStop();
		}

		debugCamera.Update();

		camera->SetRotate(cameraRotate);
		camera->SetTranslate(cameraPos);
		camera->Update();

		sprite->SetPosition(positoin);
		sprite->SetColor(materialColor);
		sprite->SetSize(scale);
		sprite->SetRotation(rotation);
		sprite->Update();

		rotateModel += 0.01f;

		entity->SetRotate(Vector3{0.0f, rotateModel, 0.0f});
		entity->Update();

		//sprite->SetMaterial(spriteMaterial);
		//sprite->SetTexture(textureSrvHandleGPU);
		//sprite->SetUvTransform(uvTransformSprite);

		// 開発用のUIの処理、実際に開発用のUIを出す場合はここをゲーム固有の処理に置き換える
		/*ImGui::SliderAngle("SphereRotateX", &transform.rotate.x);
		ImGui::SliderAngle("SphereRotateY", &transform.rotate.y);
		ImGui::SliderAngle("SphereRotateZ", &transform.rotate.z);
		ImGui::ColorEdit4("modelColor", modelColor);
		ImGui::Checkbox("enableLighting", (bool*)&materialData->enableLighting);*/

		imgui.BegineInspector();
		imgui.CameraSetting(cameraPos, cameraRotate);
		imgui.SpriteSetting("uvChecker",materialColor, positoin, rotation, scale);
		imgui.EndInspector();

		imgui.Stats();

		//ImGui::ColorEdit4("modelColor", (float*)&materialColor);
		//ImGui::SliderFloat2("translateSprite", (float*)&positoin, 0.0f, 1000.0f, "%.3f");
		//ImGui::SliderFloat3("rotateSprite", (float*)&transformSprite.rotate, 0.0f, 10.0f, "%.3f");
		//ImGui::SliderFloat3("scaleSprite", (float*)&transformSprite.scale, 0.0f, 10.0f, "%.3f");
		//ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
		//ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
		//ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);

		// ImGuiの内部コマンドを生成する
		imgui.EndFrame();

		/*-- 描画処理 --*/
		graphics.BeginFrame();

		entityCommon->DrawCommon();
		entity->Draw();

		spriteCommon->DrawCommon();
		sprite->Draw();

		imgui.Draw();
		graphics.EndFrame();
	}
	ModelManager::GetInstance()->Shutdown();
	TextureManager::Shutdown();

	Input::GetInstance()->Shutdown();

	soundCommon->Shutdown();

	imgui.Shutdown();

	graphics.Shutdown();

	Logger::Write("AppのShutdown");
	app->Shutdown();

	Logger::Write("アプリ終了");
	Logger::Shutdown();
	return 0;
}