#include "SeekerEngine.h"

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	SeekerEngine engine;
	engine.Init();

	// ImGui初期化
	ImGuiManager imgui;
	imgui.Init(engine.GetApp(), engine.GetGraphics());

	// XAudio2の初期化
	std::unique_ptr<Sound> bgm = std::make_unique<Sound>();
	std::unique_ptr<Sound> se = std::make_unique<Sound>();
	bgm->SetCommon(engine.GetSoundCommon());
	se->SetCommon(engine.GetSoundCommon());
	// 音声読み込み
	SoundData sHAudio1 = bgm->SoundLoad("resources/c21.mp3");
	SoundData sHAudio2 = bgm->SoundLoad("resources/koharubiyori.mp3");
	SoundData sHAudio3 = se->SoundLoad("resources/gold.mp3");
	SoundData sHAudio4 = se->SoundLoad("resources/se_itemget.wav"); 

	// Sprite
	Vector2 positoin = {0.0f, 0.0f};
	float rotation = 0.0f;
	std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();
	uint32_t tHChecker = TextureManager::Load("resources/uvChecker.png");
	sprite->Create(tHChecker, positoin, Color::WHITE);
	sprite->SetRotation(rotation);

	// Model
	std::unique_ptr<Entity3D> entity = std::make_unique<Entity3D>();
	ModelManager::GetInstance()->LoadModel("axis.obj");
	entity->Init(engine.GetEntityCommon());
	entity->SetModel("axis.obj");
	entity->SetTranslate(Vector3(0.0f, 0.0f, 0.0f));

	// カメラ
	std::unique_ptr<Camera> camera = std::make_unique<Camera>();
	Vector3 cameraPos = { 0.0f, 0.0f, -10.0f };
	Vector3 cameraRotate = { 0.0f, 0.0f, 0.0f };
	camera->SetRotate(cameraRotate);
	camera->SetTranslate(cameraPos);
	engine.GetEntityCommon()->SetDefaultCamera(camera.get());

	// ウィンドウの×ボタンが押されるまでループ
	while (engine.GetApp()->ProcessMessage()) {
		engine.Update();

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

		camera->SetRotate(cameraRotate);
		camera->SetTranslate(cameraPos);
		camera->Update();

		sprite->Update();

		entity->SetCamera(camera.get());
		entity->Update();

		imgui.BegineFrame();
		imgui.BegineInspector();
		imgui.CameraSetting(cameraPos, cameraRotate);
		imgui.SpriteSetting("uvChecker", sprite.get());
		imgui.ModelSetting("axis.obj", entity.get());
		imgui.EndInspector();
		imgui.Stats();
		imgui.EndFrame();

		/*-- 描画処理 --*/
		engine.BegineFrame();

		engine.GetEntityCommon()->DrawCommon();
		entity->Draw();

		engine.GetSpriteCommon()->DrawCommon();
		sprite->Draw();

		imgui.Draw();
		engine.EndFrame();
	}
	
	imgui.Shutdown();

	engine.Shutdown();
	return 0;
}