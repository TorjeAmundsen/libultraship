#include "ship/controller/controldevice/controller/mapping/dsu/DSUMapping.h"

#include <algorithm>
#include <chrono>
#include "ship/Context.h"
#include "ship/controller/controldeck/ControlDeck.h"
#include "ship/controller/physicaldevice/DSUClient.h"
#include "ship/utils/StringHelper.h"
#include "ship/window/gui/IconsFontAwesome4.h"

namespace Ship {

bool DSUButtonPressed(const DSUPadState& state, DSUButton button) {
    if (button < DSU_BUTTON_L2) {
        return state.Buttons1 & (1 << button);
    }
    if (button < DSU_BUTTON_HOME) {
        return state.Buttons2 & (1 << (button - DSU_BUTTON_L2));
    }
    if (button == DSU_BUTTON_HOME) {
        return state.Home != 0;
    }
    if (button == DSU_BUTTON_TOUCH) {
        return state.Touch != 0;
    }
    return false;
}

float DSUAxisDirectionValue(const DSUPadState& state, DSUAxis axis, AxisDirection direction) {
    uint8_t stick;
    switch (axis) {
        case DSU_AXIS_LEFTX:
            stick = state.LeftX;
            break;
        case DSU_AXIS_LEFTY:
            stick = state.LeftY;
            break;
        case DSU_AXIS_RIGHTX:
            stick = state.RightX;
            break;
        case DSU_AXIS_RIGHTY:
            stick = state.RightY;
            break;
        case DSU_AXIS_L2:
            return direction == POSITIVE ? state.L2 / 255.0f : 0.0f;
        case DSU_AXIS_R2:
            return direction == POSITIVE ? state.R2 / 255.0f : 0.0f;
        default:
            return 0.0f;
    }
    const int offset = (stick - 128) * direction;
    return std::clamp(offset / 127.0f, 0.0f, 1.0f);
}

DSURawPress DSURawPressForPort(uint8_t portIndex) {
    using Clock = std::chrono::steady_clock;
    constexpr float THRESHOLD = 0.7f;                                 // the same as SDL gamepads
    constexpr auto TRIGGER_WAIT = std::chrono::milliseconds(250);     // for a button after a trigger
    constexpr auto POPUP_CLOSED_GAP = std::chrono::milliseconds(200); // no calls for this long: a new popup

    // A trigger past the threshold, waiting to see if a button follows.
    static DSURawPress sPendingTrigger;
    static Clock::time_point sPendingSince, sLastCall;
    const auto now = Clock::now();
    if (now - sLastCall > POPUP_CLOSED_GAP) {
        sPendingTrigger = {};
    }
    sLastCall = now;

    // The editor can ask while the game isn't reading pads, so refresh the DSU state first.
    const auto controlDeck = Context::GetRawInstance()->GetControlDeck();
    controlDeck->UpdateDSUClient();
    const auto client = controlDeck->GetDSUClient();
    const auto slots = client->GetConnectedSlotsForPort(portIndex);

    for (const uint8_t slot : slots) {
        for (int32_t button = 0; button < DSU_BUTTON_MAX; button++) {
            if (DSUButtonPressed(client->GetLatchedState(slot), static_cast<DSUButton>(button))) {
                sPendingTrigger = {};
                return { DSURawPress::Kind::Button, button, POSITIVE };
            }
        }
    }
    DSURawPress trigger;
    for (const uint8_t slot : slots) {
        for (int32_t axis = 0; axis < DSU_AXIS_MAX; axis++) {
            for (const AxisDirection direction : { NEGATIVE, POSITIVE }) {
                if (DSUAxisDirectionValue(client->GetLatchedState(slot), static_cast<DSUAxis>(axis), direction) <=
                    THRESHOLD) {
                    continue;
                }
                if (axis != DSU_AXIS_L2 && axis != DSU_AXIS_R2) {
                    sPendingTrigger = {};
                    return { DSURawPress::Kind::Axis, axis, direction };
                }
                if (trigger.Type == DSURawPress::Kind::None) {
                    trigger = { DSURawPress::Kind::Axis, axis, direction };
                }
            }
        }
    }

    if (trigger.Type != DSURawPress::Kind::None) {
        if (sPendingTrigger.Type == DSURawPress::Kind::None || sPendingTrigger.Index != trigger.Index) {
            sPendingTrigger = trigger;
            sPendingSince = now;
        }
        if (now - sPendingSince < TRIGGER_WAIT) {
            return {};
        }
    } else if (sPendingTrigger.Type == DSURawPress::Kind::None) {
        return {};
    }
    // Held through the wait, or let go before it with no button: the trigger it is.
    const DSURawPress found = sPendingTrigger;
    sPendingTrigger = {};
    return found;
}

std::string DSUButtonName(DSUButton button) {
    switch (button) {
        case DSU_BUTTON_SHARE:
            return "Share";
        case DSU_BUTTON_L3:
            return "L3";
        case DSU_BUTTON_R3:
            return "R3";
        case DSU_BUTTON_OPTIONS:
            return "Options";
        case DSU_BUTTON_DPAD_UP:
            return StringHelper::Sprintf("D-Pad %s", ICON_FA_ARROW_UP);
        case DSU_BUTTON_DPAD_RIGHT:
            return StringHelper::Sprintf("D-Pad %s", ICON_FA_ARROW_RIGHT);
        case DSU_BUTTON_DPAD_DOWN:
            return StringHelper::Sprintf("D-Pad %s", ICON_FA_ARROW_DOWN);
        case DSU_BUTTON_DPAD_LEFT:
            return StringHelper::Sprintf("D-Pad %s", ICON_FA_ARROW_LEFT);
        case DSU_BUTTON_L2:
            return "L2";
        case DSU_BUTTON_R2:
            return "R2";
        case DSU_BUTTON_L1:
            return "L1";
        case DSU_BUTTON_R1:
            return "R1";
        case DSU_BUTTON_TRIANGLE:
            return "Triangle";
        case DSU_BUTTON_CIRCLE:
            return "Circle";
        case DSU_BUTTON_CROSS:
            return "Cross";
        case DSU_BUTTON_SQUARE:
            return "Square";
        case DSU_BUTTON_HOME:
            return "Home";
        case DSU_BUTTON_TOUCH:
            return "Touchpad";
        default:
            return StringHelper::Sprintf("B%d", button);
    }
}

std::string DSUAxisDirectionName(DSUAxis axis, AxisDirection direction) {
    const char* horizontal = direction == NEGATIVE ? ICON_FA_ARROW_LEFT : ICON_FA_ARROW_RIGHT;
    const char* vertical = direction == NEGATIVE ? ICON_FA_ARROW_DOWN : ICON_FA_ARROW_UP;
    switch (axis) {
        case DSU_AXIS_LEFTX:
            return StringHelper::Sprintf("Left Stick %s", horizontal);
        case DSU_AXIS_LEFTY:
            return StringHelper::Sprintf("Left Stick %s", vertical);
        case DSU_AXIS_RIGHTX:
            return StringHelper::Sprintf("Right Stick %s", horizontal);
        case DSU_AXIS_RIGHTY:
            return StringHelper::Sprintf("Right Stick %s", vertical);
        case DSU_AXIS_L2:
            return "L2 (analog)";
        case DSU_AXIS_R2:
            return "R2 (analog)";
        default:
            return StringHelper::Sprintf("Axis %d %s", axis, direction == NEGATIVE ? ICON_FA_MINUS : ICON_FA_PLUS);
    }
}

} // namespace Ship
