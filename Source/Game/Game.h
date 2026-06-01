#pragma once

#include "../Engine/EngineCore.h"
#include "Game State/GameState.h"
#include "Game Timer/GameTimer.h"
#include <memory>
#include <string>
#include <wtypes.h>

class Entity;

class Game {
public:
	Game(HINSTANCE hInstance, int windowedClientWidth, int windowedClientHeight, std::wstring caption);
	Game(const Game& rhs) = delete;
	Game& operator=(const Game& rhs) = delete;
	~Game();

	bool Initialize(HWND hwnd);
	int Run();

	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	Engine::EngineCore mEngineCore;
	GameTimer mTimer;
	const std::wstring mMainWndCaption;
	GameState mGameState;
	HWND mhMainWnd;

private:
	void CalculateFullscreenDimensions();
	void SetFullscreen();
	void SetWindowed();
	void CalculateFrameStats();
	void Update();
	void Draw();
};