#pragma once

#include "../Engine/EngineCore.h"
#include "Game State/GameState.h"
#include "../Game Timer/GameTimer.h"
#include "Player/Player.h"
#include <memory>
#include <wtypes.h>
#include <string>

class EngineCore;
class Entity;

class Game {
public:
	Game(HINSTANCE hInstance, int clientWidth, int clientHeight, std::wstring caption);
	Game(const Game& rhs) = delete;
	Game& operator=(const Game& rhs) = delete;
	~Game();

	bool Initialize(HWND hwnd);
	int Run();

	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	HWND mhMainHwnd;
	const std::wstring mMainWndCaption;
	UINT mClientWidth;
	UINT mClientHeight;

	Engine::EngineCore mEngineCore;
	GameTimer mTimer;
	GameState mGameState;
	Player mPlayer;

private:
	void CalculateFrameStats();
	void Update();
	void Draw();
};