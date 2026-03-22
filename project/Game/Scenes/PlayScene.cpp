#include "PlayScene.h"

void PlayScene::Init()
{
   //===========================
   // Sound
   //===========================
   // SoundManager::GetInstance()->Load("bgm1", "sound5.wav");
    SoundManager::GetInstance()->Load("bgm2", "koharubiyori.mp3");
    SoundManager::GetInstance()->Load("se1", "gold.mp3");
    SoundManager::GetInstance()->Load("se2", "se_itemget.wav");

    //===========================
    // Camera
    //===========================
    auto camMgr = CameraManager::GetInstance();
    camMgr->CreateCamera("EntranceCamera");
    Entity3DCommon::GetInstance()->SetCameraManager(camMgr);
    Entity3DCommon::GetInstance()->SetDebugCamera(camMgr->GetDebugCamera());

    //===========================
    // Sprite
    //===========================
    sprite = std::make_unique<Sprite>();
    uint32_t tHChecker = TextureManager::GetInstance()->Load("resources/sprites/circle.png");
    sprite->Create(tHChecker, { 0.0f, 0.0f }, Color::WHITE);
    sprite->SetRotation(0.0f);

    //===========================
    // Model
    //===========================
    entity = std::make_unique<Entity3D>();
    ModelManager::GetInstance()->LoadModel("ball.obj");
    entity->Init();
    entity->SetModel("ball");
    entity->SetTranslate(Vector3(0.0f, 0.0f, 0.0f));
    //entity->SetIsLighting(false);

    modelTerrain = std::make_unique<Entity3D>();
    ModelManager::GetInstance()->LoadModel("terrain.obj");
    modelTerrain->Init();
    modelTerrain->SetModel("terrain");
    modelTerrain->SetTranslate(Vector3(0.0f, 0.0f, 0.0f));
    //modelTerrain->SetIsLighting(false);

    //===========================
    // Particle
    //===========================
    ParticleManager::GetInstance()->CreateParticleGroup("Smoke", tHChecker);
    emitter_ = std::make_unique<ParticleEmitter>(
        "Smoke", 
        10,
        Color::YELLOW,
        Color::RED,
        Vector3{ 0.5f, 0.5f, 0.5f },
        Vector3{ 0.0f, 0.0f, 0.0f },
        0.1f,
        -0.1f,
        0.1f);
	
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

    SoundManager::GetInstance()->Update();

    emitter_->Update();
    auto camMgr = CameraManager::GetInstance();
	ParticleManager::GetInstance()->Update(camMgr);

    sprite->Update();

    entity->SetCamera(camMgr->GetActiveCamera());
    entity->Update();

    modelTerrain->SetCamera(camMgr->GetActiveCamera());
    modelTerrain->Update();

    ImGuiManager::GetInstance()->BeginFrame();
    ImGuiManager::GetInstance()->BeginInspector();
    ImGuiManager::GetInstance()->DrawCameraWindow(camMgr);
    ImGuiManager::GetInstance()->DrawSpriteInspector("uvChecker", sprite.get());
    //ImGuiManager::GetInstance()->ModelSetting("plane", entity.get());
    //ImGuiManager::GetInstance()->ModelSetting("terrain", modelTerrain.get());
    ImGuiManager::GetInstance()->DrawParticleInspector("Smoke", emitter_.get());
    ImGuiManager::GetInstance()->EndInspector();
    ImGuiManager::GetInstance()->Stats();
    ImGuiManager::GetInstance()->DrawLightWindow();
	ImGuiManager::GetInstance()->DrawSoundWindow();
    ImGuiManager::GetInstance()->EndFrame();
}

void PlayScene::Draw()
{
    /*-- 描画処理 --*/
    Entity3DCommon::GetInstance()->DrawCommon();
    entity->Draw();
    modelTerrain->Draw();
   
    ParticleManager::GetInstance()->Draw();

    //sprite->Draw();

    ImGuiManager::GetInstance()->Draw();
}

void PlayScene::Shutdown()
{
    ParticleManager::GetInstance()->RemoveParticleGroup("Smoke");
}
