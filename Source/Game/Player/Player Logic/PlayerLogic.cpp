#include "PlayerLogic.h"

#include "../../../Helper/Logger.h"
#include <fstream>
#include <sstream>
#include <windows.h>

PlayerLogic::PlayerLogic() {}

PlayerLogic::~PlayerLogic() {}

PlayerState PlayerLogic::GetPlayerState() const { return mState; }

void PlayerLogic::SetPlayerState(PlayerState state) {
	mState = state;
}

void PlayerLogic::Update(float deltaTime, const IInputSystem* const inputSystem) {
	CalculateCurrentState(deltaTime, inputSystem);
}

float PlayerLogic::GetWalkingRunningSpeed(float squareOfMovement) {
	return mState == PlayerState::Walking ? mPlayerNormalWalkingSpeed : mPlayerRunningSpeed;
}

void PlayerLogic::CalculateCurrentState(float deltaTime, const IInputSystem* const inputSystem) {
	// don't do anything if we're backwards dashing
	if (mState == PlayerState::BackwardsDashing) { return; }

	// get data
	GameButtonState actionEastButtonState = inputSystem->GetButtonState(GameButton::ActionEast);
	float leftStickX = inputSystem->GetLeftStickX();
	float leftStickY = inputSystem->GetLeftStickY();
	PlayerState targetState = mState;
	bool isMoving = leftStickX != 0 || leftStickY != 0;

	switch (mState) {
	case PlayerState::Idle:
	case PlayerState::Walking:
	case PlayerState::Running:
		if (actionEastButtonState == GameButtonState::Just_Pressed) {
			if (isMoving) {
				targetState = PlayerState::Running;
			}
			else {
				mTotalTimeActionEastWasHeldDown = 0;
				targetState = PlayerState::PendingActionEast;
			}
		}
		else {
			if (isMoving) {
				targetState = actionEastButtonState == GameButtonState::Held ? PlayerState::Running : PlayerState::Walking;
			}
			else {
				targetState = PlayerState::Idle;
			}
		}
		break;
	case PlayerState::PendingActionEast:
		mTotalTimeActionEastWasHeldDown += deltaTime;

		if (isMoving) {
			targetState = actionEastButtonState == GameButtonState::Held ? PlayerState::Running : PlayerState::Walking;
		}
		else if (mTotalTimeActionEastWasHeldDown >
			mTotalDurationPlayerHasToRunAfterPressingActionEast) {
			targetState = PlayerState::BackwardsDashing;
		}
		break;
	}

	mState = targetState;
}