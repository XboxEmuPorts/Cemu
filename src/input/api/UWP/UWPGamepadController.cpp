#include "input/api/UWP/UWPGamepadController.h"

#include <algorithm>
#include <array>
#include <mutex>
#include <string>

namespace
{
struct HostGamepadState
{
	bool connected = false;
	uint32 buttons = 0;
	float leftX = 0.0f;
	float leftY = 0.0f;
	float rightX = 0.0f;
	float rightY = 0.0f;
	float leftTrigger = 0.0f;
	float rightTrigger = 0.0f;
};

std::mutex s_hostGamepadMutex;
std::array<HostGamepadState, UWPGamepadController::kMaxHostGamepads> s_hostGamepadStates{};

float ClampAxis(float value)
{
	return (std::max)(-1.0f, (std::min)(value, 1.0f));
}
}

UWPGamepadController::UWPGamepadController(uint32 playerIndex)
	: ControllerBase(playerIndex == 0 ? "host-wgi-gamepad" :
		std::string("host-wgi-gamepad-") + std::to_string(playerIndex),
		"Xbox Gamepad"),
	m_playerIndex(playerIndex)
{
}

void UWPGamepadController::SetHostState(uint32 playerIndex, bool connected, uint32 buttons,
	float leftX, float leftY, float rightX, float rightY,
	float leftTrigger, float rightTrigger)
{
	if (playerIndex >= kMaxHostGamepads)
		return;
	std::scoped_lock lock(s_hostGamepadMutex);
	s_hostGamepadStates[playerIndex] = {
		connected,
		buttons,
		ClampAxis(leftX), ClampAxis(leftY),
		ClampAxis(rightX), ClampAxis(rightY),
		(std::max)(0.0f, (std::min)(leftTrigger, 1.0f)),
		(std::max)(0.0f, (std::min)(rightTrigger, 1.0f))
	};
}

bool UWPGamepadController::IsHostGamepadConnected(uint32 playerIndex)
{
	if (playerIndex >= kMaxHostGamepads)
		return false;
	std::scoped_lock lock(s_hostGamepadMutex);
	return s_hostGamepadStates[playerIndex].connected;
}

bool UWPGamepadController::is_connected()
{
	return IsHostGamepadConnected(m_playerIndex);
}

std::string UWPGamepadController::get_button_name(uint64 button) const
{
	return ControllerBase::get_button_name(button);
}

ControllerState UWPGamepadController::raw_state()
{
	HostGamepadState state;
	{
		std::scoped_lock lock(s_hostGamepadMutex);
		if (m_playerIndex >= kMaxHostGamepads)
			return {};
		state = s_hostGamepadStates[m_playerIndex];
	}

	ControllerState result{};
	if (!state.connected)
		return result;

	for (uint32 button = 0; button < 16; ++button)
	{
		if ((state.buttons & (1u << button)) != 0)
			result.buttons.SetButtonState(button, true);
	}
	result.axis = { state.leftX, state.leftY };
	result.rotation = { state.rightX, state.rightY };
	result.trigger = { state.leftTrigger, state.rightTrigger };
	return result;
}
