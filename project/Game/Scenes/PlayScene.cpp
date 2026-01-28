#include "PlayScene.h"

void PlayScene::Init()
{
   //===========================
   // Sound
   //===========================
    SoundManager::GetInstance()->Load("bgm1", "resources/c21.mp3");
    SoundManager::GetInstance()->Load("bgm2", "resources/koharubiyori.mp3");
    SoundManager::GetInstance()->Load("se1", "resources/gold.mp3");
    SoundManager::GetInstance()->Load("se2", "resources/se_itemget.wav");

    //===========================
    // Camera
    //===========================
    cameraManager = std::make_unique<CameraManager>();
    cameraManager->Init();
    cameraManager->CreateCamera("EntranceCamera");
    Entity3DCommon::GetInstance()->SetCameraManager(cameraManager.get());
    Entity3DCommon::GetInstance()->SetDebugCamera(cameraManager->GetDebugCamera());

    //===========================
    // Sprite
    //===========================
    sprite = std::make_unique<Sprite>();
    uint32_t tHChecker = TextureManager::GetInstance()->Load("resources/uvChecker.png");
    sprite->Create(tHChecker, { 0.0f, 0.0f }, Color::WHITE);
    sprite->SetRotation(0.0f);

    //===========================
    // Model
    //===========================
    entity = std::make_unique<Entity3D>();
    ModelManager::GetInstance()->LoadModel("ball");
    entity->Init();
    entity->SetModel("ball");
    entity->SetTranslate(Vector3(0.0f, 0.0f, 0.0f));

    modelTerrain = std::make_unique<Entity3D>();
    ModelManager::GetInstance()->LoadModel("terrain");
    modelTerrain->Init();
    modelTerrain->SetModel("terrain");
    modelTerrain->SetTranslate(Vector3(0.0f, 0.0f, 0.0f));

    //===========================
    // Particle
    //===========================
    ParticleManager::GetInstance()->CreateParticleGroup("Smoke", tHChecker);
    emitter_ = std::make_unique<ParticleEmitter>("Smoke", 10, 0.1f);
	
}

void PlayScene::Update()
{
    /*-- 更新処理 --*/
    if (Input::GetInstance()->IsPressed(DIK_S)) {
        SoundManager::GetInstance()->PlayBGM("bgm1");
    }

    if (Input::GetInstance()->IsPressed(DIK_N)) {
        SoundManager::GetInstance()->PlayBGM("bgm2");
    }

    if (Input::GetInstance()->IsPressed(DIK_M)) {
        SoundManager::GetInstance()->PlaySE("se1");
    }

    if (Input::GetInstance()->IsPressed(DIK_V)) {
        SoundManager::GetInstance()->PlaySE("se2");
    }

    if (Input::GetInstance()->IsPressed(DIK_B)) {
        SoundManager::GetInstance()->StopBGM();
    }

    if (Input::GetInstance()->IsPressed(DIK_SPACE)) {
        SoundManager::GetInstance()->StopBGM();
        SoundManager::GetInstance()->StopSE();
        SceneManager::GetInstance()->ChangeScene("TITLE");
    }

    cameraManager->Update();

    emitter_->Update();
	ParticleManager::GetInstance()->Update(cameraManager.get());

    sprite->Update();

    entity->SetCamera(cameraManager->GetActiveCamera());
    entity->Update();

    modelTerrain->SetCamera(cameraManager->GetActiveCamera());
    modelTerrain->Update();

    ImGuiManager::GetInstance()->BegineFrame();
    ImGuiManager::GetInstance()->BegineInspector();
    ImGuiManager::GetInstance()->CameraSetting(cameraManager.get());
    ImGuiManager::GetInstance()->SpriteSetting("uvChecker", sprite.get());
    ImGuiManager::GetInstance()->ModelSetting("ball", entity.get());
    ImGuiManager::GetInstance()->ModelSetting("terrain", modelTerrain.get());
    ImGuiManager::GetInstance()->ParticleSetting("Smoke", emitter_.get());
    ImGuiManager::GetInstance()->EndInspector();
    ImGuiManager::GetInstance()->Stats();
    ImGuiManager::GetInstance()->LightSetting();
	ImGuiManager::GetInstance()->SoundSetting();
    ImGuiManager::GetInstance()->EndFrame();
}

void PlayScene::Draw()
{
    /*-- 描画処理 --*/
    Entity3DCommon::GetInstance()->DrawCommon();
    entity->Draw();
    modelTerrain->Draw();
   
    ParticleManager::GetInstance()->Draw();

    SpriteCommon::GetInstance()->DrawCommon();
    sprite->Draw();

    ImGuiManager::GetInstance()->Draw();
}

void PlayScene::Shutdown()
{
    ParticleManager::GetInstance()->RemoveParticleGroup("Smoke");
    SoundManager::GetInstance()->Unload("bgm1");
    SoundManager::GetInstance()->Unload("bgm2");
    SoundManager::GetInstance()->Unload("se1");
    SoundManager::GetInstance()->Unload("se2");
}
