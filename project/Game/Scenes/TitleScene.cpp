#include "TitleScene.h"
#include "SceneIncludes.h"
#include "Skybox.h"

void TitleScene::Init()
{
    Logger::Write("現在シーンTitleScene");

    tHTex_ = TextureManager::GetInstance()->Load("./resources/rostock_laage_airport_4k.dds");
    tHCircle_ = TextureManager::GetInstance()->Load("./resources/sprites/circle2.png");
    Skybox::GetInstance()->SetTexture(tHTex_);

    entity_ = std::make_unique<Entity3D>();
    ModelManager::GetInstance()->LoadModel("ball.obj");
    ModelManager::GetInstance()->FindModel("ball")->SetEnviromentTexture(tHTex_);
    entity_->Init();
    entity_->SetModel("ball");
    entity_->SetTranslate(Vector3(0.0f, 0.0f, 0.0f));
    Editor::GetInstance()->RegisterModel("ball", entity_.get());

    ParticleConfig hitEffectConfig;
    ParticleManager::GetInstance()->CreateParticleGroup("HitEffect", tHCircle_);
    EmitterManager::GetInstance()->CreateEmitter("HitEffect", hitEffectConfig);
    Editor::GetInstance()->RegisterParticle("HitEffect");

    ImGuiManager::GetInstance()->LoadScenesJson();
}

void TitleScene::Update()
{
    auto camMgr = CameraManager::GetInstance();

    if (Input::GetInstance()->IsTrigger(DIK_ESCAPE)) {
        EndRequset();
    }

    if (Input::GetInstance()->IsTrigger(DIK_SPACE)) {
        SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
    }

    Skybox::GetInstance()->Update();
    entity_->SetCamera(camMgr->GetActiveCamera());
    entity_->Update();

    ParticleManager::GetInstance()->Update(camMgr);
    EmitterManager::GetInstance()->Update();

    ImGuiManager::GetInstance()->BeginFrame();
    ImGuiManager::GetInstance()->DrawMainMenuBar();
    ImGuiManager::GetInstance()->DrawCameraWindow(camMgr);
    ImGuiManager::GetInstance()->DrawEditor();
    ImGuiManager::GetInstance()->Stats();
    ImGuiManager::GetInstance()->DrawSoundWindow();
    ImGuiManager::GetInstance()->DrawLoggerWindow();
    ImGuiManager::GetInstance()->EndFrame();
}

void TitleScene::Draw()
{
    Skybox::GetInstance()->Draw();
    entity_->Draw();
    ParticleManager::GetInstance()->Draw();
    ImGuiManager::GetInstance()->Draw();
}

void TitleScene::Shutdown()
{
    ParticleManager::GetInstance()->RemoveParticleGroup("HitEffect");
    EmitterManager::GetInstance()->RemoveEmitter("HitEffect");
    Editor::GetInstance()->Clear();
}
