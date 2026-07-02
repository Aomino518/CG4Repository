#include "TitleScene.h"
#include "SceneIncludes.h"
#include "Skybox.h"

void TitleScene::Init()
{
    Logger::Write("現在シーンTitleScene");

    tHTex_ = TextureManager::GetInstance()->Load("./resources/rostock_laage_airport_4k.dds");
    tHCircle_ = TextureManager::GetInstance()->Load("./resources/sprites/circle2.png");
    tHGradationLine_ = TextureManager::GetInstance()->Load("./resources/sprites/gradationLine.png");
    Skybox::GetInstance()->SetTexture(tHTex_);

    entity_ = std::make_unique<Entity3D>();
    ModelManager::GetInstance()->LoadModel("ball", "ball.obj");
    ModelManager::GetInstance()->FindModel("ball")->SetEnviromentTexture(tHTex_);
    entity_->Init("ball");
    entity_->SetTranslate(Vector3(0.0f, 0.0f, 0.0f));
    Editor::GetInstance()->RegisterModel("ball", entity_.get());

    modelTerrain_ = std::make_unique<Entity3D>();
    ModelManager::GetInstance()->LoadModel("terrain", "terrain.obj");
    ModelManager::GetInstance()->FindModel("terrain")->SetEnviromentTexture(tHTex_);
    modelTerrain_->Init("terrain");
    modelTerrain_->SetTranslate(Vector3(0.0f, 0.0f, 0.0f));
    Editor::GetInstance()->RegisterModel("terrain", modelTerrain_.get());

    animeCube_ = std::make_unique<Entity3D>();
    ModelManager::GetInstance()->LoadModel("AnimatedCube", "AnimatedCube.gltf");
    ModelManager::GetInstance()->FindModel("AnimatedCube")->SetEnviromentTexture(tHTex_);
    animeCube_->Init("AnimatedCube");
    animeCube_->SetTranslate(Vector3(2.0f, 0.0f, 2.0f));
    Editor::GetInstance()->RegisterModel("AnimatedCube", animeCube_.get());

    simpleSkin_ = std::make_unique<Entity3D>();
    ModelManager::GetInstance()->LoadModel("human", "sneakWalk.gltf");
    ModelManager::GetInstance()->FindModel("sneakWalk")->SetEnviromentTexture(tHTex_);
    simpleSkin_->Init("sneakWalk");
    simpleSkin_->SetTranslate(Vector3{ -2.0f, 0.0f, 2.0f });
    Editor::GetInstance()->RegisterModel("sneakWalk", simpleSkin_.get());

    ParticleManager::GetInstance()->CreateParticleGroup("HitEffect", tHCircle_);
    EmitterManager::GetInstance()->CreateEmitter("HitEffect", hitEffectConfig_);
    Editor::GetInstance()->RegisterParticle("HitEffect");

    ParticleManager::GetInstance()->CreateParticleGroup("RingEffect", tHGradationLine_);
    ParticleManager::GetInstance()->SetModelType("RingEffect", ParticleModelType::Ring);
    EmitterManager::GetInstance()->CreateEmitter("RingEffect", hitRingEffectConfig_);
    Editor::GetInstance()->RegisterParticle("RingEffect");

    ParticleManager::GetInstance()->CreateParticleGroup("CylinderEffect", tHGradationLine_);
    ParticleManager::GetInstance()->SetModelType("CylinderEffect", ParticleModelType::Cylinder);
    EmitterManager::GetInstance()->CreateEmitter("CylinderEffect", cylinderEffectConfig_);
    Editor::GetInstance()->RegisterParticle("CylinderEffect");

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

    if (Input::GetInstance()->IsTrigger(DIK_R)) {
        auto hitEffectTransform = EmitterManager::GetInstance()->GetEmitter("HitEffect")->GetTransform();
        auto hitEffectCount = EmitterManager::GetInstance()->GetEmitter("HitEffect")->GetCount();
        hitEffectConfig_ = EmitterManager::GetInstance()->GetEmitter("HitEffect")->GetConfig();
        auto ringEffectTransform = EmitterManager::GetInstance()->GetEmitter("RingEffect")->GetTransform();
        auto ringEffectCount = EmitterManager::GetInstance()->GetEmitter("RingEffect")->GetCount();
        hitRingEffectConfig_ = EmitterManager::GetInstance()->GetEmitter("RingEffect")->GetConfig();
        ParticleManager::GetInstance()->Emit("HitEffect", hitEffectConfig_, hitEffectTransform.translate, hitEffectCount);
        ParticleManager::GetInstance()->Emit("RingEffect", hitRingEffectConfig_, ringEffectTransform.translate, ringEffectCount);
    }

    Skybox::GetInstance()->Update();
    entity_->SetCamera(camMgr->GetActiveCamera());
    entity_->Update();
    modelTerrain_->SetCamera(camMgr->GetActiveCamera());
    modelTerrain_->Update();
    animeCube_->SetCamera(camMgr->GetActiveCamera());
    animeCube_->Update();
    simpleSkin_->SetCamera(camMgr->GetActiveCamera());
    simpleSkin_->Update();

    EmitterManager::GetInstance()->Update();
    ParticleManager::GetInstance()->Update(camMgr);

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
    modelTerrain_->Draw();
    animeCube_->Draw();
    simpleSkin_->Draw();
    ParticleManager::GetInstance()->Draw();

    simpleSkin_->DrawBorn();
    DebugDraw::Draw();
    ImGuiManager::GetInstance()->Draw();
}

void TitleScene::Shutdown()
{
    ParticleManager::GetInstance()->RemoveParticleGroup("HitEffect");
    EmitterManager::GetInstance()->RemoveEmitter("HitEffect");
    ParticleManager::GetInstance()->RemoveParticleGroup("RingEffect");
    EmitterManager::GetInstance()->RemoveEmitter("RingEffect");
    ParticleManager::GetInstance()->RemoveParticleGroup("CylinderEffect");
    EmitterManager::GetInstance()->RemoveEmitter("CylinderEffect");
    Editor::GetInstance()->Clear();
}
