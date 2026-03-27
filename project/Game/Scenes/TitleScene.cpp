#include "TitleScene.h"
#include "SceneManager.h"

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

    spr_monsterBall = std::make_unique<Sprite>();
    uint32_t tHMonsterBall = TextureManager::GetInstance()->Load("resources/sprites/monsterBall.png");
    spr_monsterBall->Create(tHMonsterBall, { 360.0f, 0.0f }, Color::WHITE);
    spr_monsterBall->SetRotation(0.0f);
}

void TitleScene::Update()
{
    if (Input::GetInstance()->IsPressed(DIK_M)) {
        SoundManager::GetInstance()->PlayBGM("bgm1");
    }

    if (Input::GetInstance()->IsPressed(DIK_N)) {
        SoundManager::GetInstance()->StopBGM();
    }

    if (Input::GetInstance()->IsPressed(DIK_SPACE)) {
       SoundManager::GetInstance()->StopBGM();
       SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
    }

    SoundManager::GetInstance()->Update();

    sprite->Update();
    spr_monsterBall->Update();

    ImGuiManager::GetInstance()->BeginFrame();
    ImGuiManager::GetInstance()->DrawMainMenuBar();
    ImGuiManager::GetInstance()->BeginInspector();
    ImGuiManager::GetInstance()->DrawSpriteInspector("uvChecker", sprite.get());
    ImGuiManager::GetInstance()->DrawSpriteInspector("monsterBall", spr_monsterBall.get());
    ImGuiManager::GetInstance()->EndInspector();
    ImGuiManager::GetInstance()->Stats();
    ImGuiManager::GetInstance()->EndFrame();
}

void TitleScene::Draw()
{
    sprite->Draw();
    spr_monsterBall->Draw();
    ImGuiManager::GetInstance()->Draw();
}

void TitleScene::Shutdown()
{

}
