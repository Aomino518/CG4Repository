#include "Game.h"

void Game::Init()
{
	SEFramework::Init();

    //===========================
    // ImGui
    //===========================
    imgui.Init(engine_.GetApp(), engine_.GetGraphics());

    //===========================
    // Sound
    //===========================
    bgm = std::make_unique<Sound>();
    se = std::make_unique<Sound>();

    bgm->SetCommon(engine_.GetSoundCommon());
    se->SetCommon(engine_.GetSoundCommon());

    sHAudio1 = bgm->SoundLoad("resources/c21.mp3");
    sHAudio2 = bgm->SoundLoad("resources/koharubiyori.mp3");
    sHAudio3 = se->SoundLoad("resources/gold.mp3");
    sHAudio4 = se->SoundLoad("resources/se_itemget.wav");

    //===========================
    // Camera
    //===========================
	cameraManager = std::make_unique<CameraManager>();
	cameraManager->Init();
	cameraManager->CreateCamera("EntranceCamera");
	engine_.GetEntityCommon()->SetCameraManager(cameraManager.get());
	engine_.GetEntityCommon()->SetDebugCamera(cameraManager->GetDebugCamera());

    //===========================
    // Sprite
    //===========================
    sprite = std::make_unique<Sprite>();
    uint32_t tHChecker = TextureManager::GetInstance()->Load("resources/uvChecker.png");
    sprite->Create(engine_.GetSpriteCommon(), tHChecker, {0.0f, 0.0f}, Color::WHITE);
    sprite->SetRotation(0.0f);

    //===========================
    // Model
    //===========================
    entity = std::make_unique<Entity3D>();
    ModelManager::GetInstance()->LoadModel("fence");
    entity->Init(engine_.GetEntityCommon());
    entity->SetModel("fence");
    entity->SetTranslate(Vector3(0.0f, 0.0f, 0.0f));

	uint32_t tHParticle = TextureManager::GetInstance()->Load("resources/circle.png");
	engine_.GetPaticleCommon()->SetTexture(tHParticle);
}

void Game::Shutdown()
{
	imgui.Shutdown();

	SEFramework::Shutdown();
}

void Game::Update()
{
	SEFramework::Update();

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

	cameraManager->Update();

	engine_.GetPaticleCommon()->UpdateInstanceData(cameraManager.get());
	sprite->Update();

	entity->SetCamera(cameraManager->GetActiveCamera());
	entity->Update();

	imgui.BegineFrame();
	imgui.BegineInspector();
	imgui.CameraSetting(cameraManager.get());
	imgui.ParticleSetting("testParticle", engine_.GetPaticleCommon());
	//imgui.SpriteSetting("uvChecker", sprite.get());
	//imgui.ModelSetting("fence.obj", entity.get());
	imgui.EndInspector();
	imgui.Stats();
	imgui.EndFrame();
}

void Game::Draw()
{
	/*-- 描画処理 --*/
	engine_.GetEntityCommon()->DrawCommon();
	//entity->Draw();

	engine_.GetPaticleCommon()->Draw();

	engine_.GetSpriteCommon()->DrawCommon();
	//sprite->Draw();

	imgui.Draw();
}
