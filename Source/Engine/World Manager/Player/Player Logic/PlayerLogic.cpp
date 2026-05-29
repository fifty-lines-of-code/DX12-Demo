#include "PlayerLogic.h"

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

float PlayerLogic::GetWalkingRunningSpeed() {
	return mState == PlayerState::Walking ? mPlayerNormalWalkingSpeed : mPlayerRunningSpeed;
}

void PlayerLogic::CalculateCurrentState(float deltaTime, const IInputSystem* const inputSystem) {
	// don't do anything if we're rotating or backwards dashing
	if (mState == PlayerState::Rotating ||
		mState == PlayerState::BackwardsDashing)
	{ return; }

	// get data
	GameButtonState actionEastButtonState = inputSystem->GetButtonState(GameButton::ActionEast);
	float leftStickX = inputSystem->GetLeftStickX();
	float leftStickY = inputSystem->GetLeftStickY();
	PlayerState targetState = mState;
	float movementSqLength = (leftStickX * leftStickX) + (leftStickY * leftStickY);
	bool isMoving = movementSqLength >= 0.01f;

	switch (mState) {
	case PlayerState::Idle:
		if (isMoving) {
			// rotate to camera's movement vector
			targetState = PlayerState::BeginRotating;
			break;
		}
		// fallthrough on purpose
	case PlayerState::BeginRotating:
	case PlayerState::Rotating:
	case PlayerState::EndRotating:
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