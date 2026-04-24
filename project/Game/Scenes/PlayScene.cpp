#include "PlayScene.h"
#include "SceneIncludes.h"

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
    Editor::GetInstance()->RegisterSprite("uvChecker", sprite.get());

    //===========================
    // Model
    //===========================
    entity = std::make_unique<Entity3D>();
    ModelManager::GetInstance()->LoadModel("ball.obj");
    entity->Init();
    entity->SetModel("ball");
    entity->SetTranslate(Vector3(0.0f, 0.0f, 0.0f));
    Editor::GetInstance()->RegisterModel("ball", entity.get());

    modelTerrain = std::make_unique<Entity3D>();
    ModelManager::GetInstance()->LoadModel("terrain.obj");
    modelTerrain->Init();
    modelTerrain->SetModel("terrain");
    modelTerrain->SetTranslate(Vector3(0.0f, 0.0f, 0.0f));
    Editor::GetInstance()->RegisterModel("terrain", modelTerrain.get());
   
    //===========================
    // Particle
    //===========================
    ParticleConfig particleConfig;
    ParticleManager::GetInstance()->CreateParticleGroup("Smoke", tHChecker);
    EmitterManager::GetInstance()->CreateEmitter("Smoke", particleConfig);
    Editor::GetInstance()->RegisterParticle("Smoke");
    WorldFieldManager::GetInstance()->CreateWorldField("TestWind_001");
    WorldFieldManager::GetInstance()->CreateWorldField("TestWind_002");
    
    LightManager::GetInstance()->CreatePointLight("PointLight1");
    LightManager::GetInstance()->CreatePointLight("PointLight2");
    LightManager::GetInstance()->CreateSpotLight("SpotLight1");

    ImGuiManager::GetInstance()->LoadScenesJson();
}

void PlayScene::Update()
{
    /*-- 更新処理 --*/
    if (Input::GetInstance()->IsTrigger(DIK_C)) {
        SoundManager::GetInstance()->PlayBGM("bgm1");
    }

    if (Input::GetInstance()->IsTrigger(DIK_N)) {
        SoundManager::GetInstance()->PlayBGM("bgm2");
    }

    if (Input::GetInstance()->IsTrigger(DIK_M)) {
        SoundManager::GetInstance()->PlaySE("se1");
    }

    if (Input::GetInstance()->IsTrigger(DIK_V)) {
        SoundManager::GetInstance()->PlaySE("se2");
    }

    if (Input::GetInstance()->IsTrigger(DIK_B)) {
        SoundManager::GetInstance()->StopBGM();
    }

    if (Input::GetInstance()->IsTrigger(DIK_SPACE)) {
        SoundManager::GetInstance()->StopBGM();
        SoundManager::GetInstance()->StopSE();
        SceneManager::GetInstance()->ChangeScene("TITLE");
    }

    SoundManager::GetInstance()->Update();

    EmitterManager::GetInstance()->Update();
    auto camMgr = CameraManager::GetInstance();
	ParticleManager::GetInstance()->Update(camMgr);

    sprite->Update();

    entity->SetCamera(camMgr->GetActiveCamera());
    entity->Update();

    modelTerrain->SetCamera(camMgr->GetActiveCamera());
    modelTerrain->Update();

    ImGuiManager::GetInstance()->BeginFrame();
    ImGuiManager::GetInstance()->DrawMainMenuBar();
    ImGuiManager::GetInstance()->DrawCameraWindow(camMgr);
    ImGuiManager::GetInstance()->DrawEditor();
    ImGuiManager::GetInstance()->Stats();
	ImGuiManager::GetInstance()->DrawSoundWindow();
    ImGuiManager::GetInstance()->DrawLoggerWindow();
    ImGuiManager::GetInstance()->EndFrame();
}

void PlayScene::Draw()
{
    /*-- 描画処理 --*/
    entity->Draw();
    modelTerrain->Draw();
   
    ParticleManager::GetInstance()->Draw();

    //sprite->Draw();

    WorldFieldManager::GetInstance()->DrawDebug();
    EmitterManager::GetInstance()->DrawDebug();
    DebugDraw::Draw();

    ImGuiManager::GetInstance()->Draw();
}

void PlayScene::Shutdown()
{
    ParticleManager::GetInstance()->RemoveParticleGroup("Smoke");
    Editor::GetInstance()->Clear();
}
