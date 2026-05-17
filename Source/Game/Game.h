#pragma once

#include "../Game Timer/GameTimer.h"
#include <memory>
#include <wtypes.h>
#include <string>

class Engine;
class Entity;
class GameState;
class Player;

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
	HWND mMainHwnd;
	const std::wstring mMainWndCaption;
	UINT mClientWidth;
	UINT mClientHeight;

	std::unique_ptr<Engine> mEngine;
	GameTimer mTimer;
	std::unique_ptr<GameState> mGameState;
	std::unique_ptr<Player> mPlayer;

private:
	bool InitializePlayer(Entity* entity);
	void CalculateFrameStats();
	void Update();
	void Draw();
};