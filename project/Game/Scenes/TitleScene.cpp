#include "TitleScene.h"
#include "SceneIncludes.h"

void TitleScene::Init()
{
    //===========================
    // Sound
    //===========================
    SoundManager::GetInstance()->Load("bgm1", "sound5.wav");

    //===========================
    // Sprite
    //===========================
    sprite = std::make_unique<Sprite>();
    uint32_t tHChecker = TextureManager::GetInstance()->Load("resources/sprites/uvChecker.png");
    sprite->Create(tHChecker, { 0.0f, 0.0f }, Color::WHITE);
    sprite->SetRotation(0.0f);
    Editor::GetInstance()->RegisterSprite("uvChecker", sprite.get());

    spr_monsterBall = std::make_unique<Sprite>();
    uint32_t tHMonsterBall = TextureManager::GetInstance()->Load("resources/sprites/monsterBall.png");
    spr_monsterBall->Create(tHMonsterBall, { 360.0f, 0.0f }, Color::WHITE);
    spr_monsterBall->SetRotation(0.0f);
    Editor::GetInstance()->RegisterSprite("monsterBall", spr_monsterBall.get());

    ImGuiManager::GetInstance()->LoadScenesJson();
}

void TitleScene::Update()
{
    if (Input::GetInstance()->IsTrigger(DIK_B)) {
        SoundManager::GetInstance()->PlayBGM("bgm1");
    }

    if (Input::GetInstance()->IsTrigger(DIK_N)) {
        SoundManager::GetInstance()->StopBGM();
    }

    if (Input::GetInstance()->IsTrigger(DIK_SPACE)) {
       SoundManager::GetInstance()->StopBGM();
       SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
    }

    SoundManager::GetInstance()->Update();

    sprite->Update();
    spr_monsterBall->Update();

    ImGuiManager::GetInstance()->BeginFrame();
    ImGuiManager::GetInstance()->DrawMainMenuBar();
    ImGuiManager::GetInstance()->DrawEditor();
    ImGuiManager::GetInstance()->DrawLoggerWindow();
    ImGuiManager::GetInstance()->Stats();
    ImGuiManager::GetInstance()->EndFrame();
}

void TitleScene::Draw()
{
    sprite->Draw();
    spr_monsterBall->Draw();
    DebugDraw::DrawLine(Vector2{ 0.0f, 0.0f }, Vector2{ 1280.0f, 720.0f }, Color::MAGENTA);
    DebugDraw::DrawCircle({ 400.0f, 200.0f }, 50.0f, 30.0f, Color::GREEN, DebugDrawMode::Solid);
    DebugDraw::DrawRect({ 200.0f, 600.0f }, { 50.0f, 50.0f }, Color::WHITE, DebugDrawMode::Solid);
    DebugDraw::Draw();
    ImGuiManager::GetInstance()->Draw();
}

void TitleScene::Shutdown()
{
    Editor::GetInstance()->Clear();
}
