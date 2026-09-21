#pragma once

#include "input/api/Controller.h"

// Input state supplied by an embedding UWP/Xbox host.  Keeping this adapter
// free of WinRT types is important: Windows.Gaming.Input objects are
// apartment-affine on Xbox, while Cemu's input threads are not.
class UWPGamepadController final : public ControllerBase
{
public:
	static constexpr uint32 kMaxHostGamepads = 4;
	explicit UWPGamepadController(uint32 playerIndex = 0);

	static void SetHostState(uint32 playerIndex, bool connected, uint32 buttons,
		float leftX, float leftY, float rightX, float rightY,
		float leftTrigger, float rightTrigger);
	static bool IsHostGamepadConnected(uint32 playerIndex = 0);
	static float GetHostRumble(uint32 playerIndex);
	uint32 host_player_index() const { return m_playerIndex; }

	std::string_view api_name() const override { return "Windows.Gaming.Input"; }
	InputAPI::Type api() const override { return InputAPI::WGIGamepad; }
	bool is_connected() override;
	bool has_rumble() override { return true; }
	void start_rumble() override;
	void stop_rumble() override;
	std::string get_button_name(uint64 button) const override;

protected:
	ControllerState raw_state() override;

private:
	uint32 m_playerIndex{};
};
