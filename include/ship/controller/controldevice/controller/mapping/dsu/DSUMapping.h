#pragma once

#include <cstdint>
#include <string>
#include "ship/controller/controldevice/controller/mapping/AxisDirection.h"
#include "ship/controller/physicaldevice/DSUPadState.h"

namespace Ship {

/** @brief Buttons a DSU (cemuhook) server reports, in its DualShock 4 layout. */
enum DSUButton {
    DSU_BUTTON_SHARE,
    DSU_BUTTON_L3,
    DSU_BUTTON_R3,
    DSU_BUTTON_OPTIONS,
    DSU_BUTTON_DPAD_UP,
    DSU_BUTTON_DPAD_RIGHT,
    DSU_BUTTON_DPAD_DOWN,
    DSU_BUTTON_DPAD_LEFT,
    DSU_BUTTON_L2,
    DSU_BUTTON_R2,
    DSU_BUTTON_L1,
    DSU_BUTTON_R1,
    DSU_BUTTON_TRIANGLE,
    DSU_BUTTON_CIRCLE,
    DSU_BUTTON_CROSS,
    DSU_BUTTON_SQUARE,
    DSU_BUTTON_HOME,
    DSU_BUTTON_TOUCH,
    DSU_BUTTON_MAX
};

/** @brief Axes a DSU server reports. On the Y axes, POSITIVE is up. Triggers only go POSITIVE. */
enum DSUAxis {
    DSU_AXIS_LEFTX,
    DSU_AXIS_LEFTY,
    DSU_AXIS_RIGHTX,
    DSU_AXIS_RIGHTY,
    DSU_AXIS_L2,
    DSU_AXIS_R2,
    DSU_AXIS_MAX
};

/** @brief Whether a button is held in a slot's state. */
bool DSUButtonPressed(const DSUPadState& state, DSUButton button);

/**
 * @brief How far an axis is pushed in one direction, 0 to 1.
 *
 * Stick bytes have 127 steps each way from 128, so 1 and 255 are full deflection and 0 reads like
 * 1. That keeps both directions the same, and every in-game stick value reachable.
 */
float DSUAxisDirectionValue(const DSUPadState& state, DSUAxis axis, AxisDirection direction);

/** @brief The DSU input press-to-bind found: a button, an axis direction, or nothing yet. */
struct DSURawPress {
    enum class Kind { None, Button, Axis };
    Kind Type = Kind::None;
    int32_t Index = 0; ///< A DSUButton or a DSUAxis.
    AxisDirection Direction = POSITIVE;
};

/**
 * @brief The DSU input being pressed on a port's slots, for press-to-bind. Call every frame while waiting.
 *
 * Buttons win. A stick past the threshold (the same as SDL gamepads) counts right away, but a trigger
 * only after a short wait with no button coming in: a Gamecube trigger's analog half passes the
 * threshold just before its digital click at the bottom, and binding the click should be easy.
 */
DSURawPress DSURawPressForPort(uint8_t portIndex);

/** @brief Display names, e.g. "Cross" or "Left Stick" plus an arrow. */
std::string DSUButtonName(DSUButton button);
std::string DSUAxisDirectionName(DSUAxis axis, AxisDirection direction);

} // namespace Ship
