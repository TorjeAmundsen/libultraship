#include "ship/controller/controldevice/controller/mapping/dsu/DSUButtonToAxisDirectionMapping.h"
#include "ship/utils/StringHelper.h"
#include "ship/config/ConsoleVariable.h"
#include "ship/Context.h"
#include "ship/controller/controldeck/ControlDeck.h"

namespace Ship {
DSUButtonToAxisDirectionMapping::DSUButtonToAxisDirectionMapping(uint8_t portIndex, StickIndex stickIndex,
                                                                 Direction direction, int32_t dsuButton)
    : ControllerInputMapping(PhysicalDeviceType::DSU),
      ControllerAxisDirectionMapping(PhysicalDeviceType::DSU, portIndex, stickIndex, direction),
      DSUButtonToAnyMapping(dsuButton) {
}

float DSUButtonToAxisDirectionMapping::GetNormalizedAxisDirectionValue() {
    if (Context::GetRawInstance()->GetControlDeck()->GamepadGameInputBlocked()) {
        return 0.0f;
    }
    return ButtonPressedForPort(mPortIndex) ? MAX_AXIS_RANGE : 0.0f;
}

std::string DSUButtonToAxisDirectionMapping::GetAxisDirectionMappingId() {
    return StringHelper::Sprintf("P%d-S%d-D%d-DSUB%d", mPortIndex, mStickIndex, mDirection, mDSUButton);
}

void DSUButtonToAxisDirectionMapping::SaveToConfig() {
    const std::string key = CVAR_PREFIX_CONTROLLERS ".AxisDirectionMappings." + GetAxisDirectionMappingId();
    const auto cvars = Ship::Context::GetRawInstance()->GetConsoleVariables();
    cvars->SetString(StringHelper::Sprintf("%s.AxisDirectionMappingClass", key.c_str()).c_str(),
                     "DSUButtonToAxisDirectionMapping");
    cvars->SetInteger(StringHelper::Sprintf("%s.Stick", key.c_str()).c_str(), mStickIndex);
    cvars->SetInteger(StringHelper::Sprintf("%s.Direction", key.c_str()).c_str(), mDirection);
    cvars->SetInteger(StringHelper::Sprintf("%s.DSUButton", key.c_str()).c_str(), mDSUButton);
    cvars->Save();
}

void DSUButtonToAxisDirectionMapping::EraseFromConfig() {
    const std::string key = CVAR_PREFIX_CONTROLLERS ".AxisDirectionMappings." + GetAxisDirectionMappingId();
    const auto cvars = Ship::Context::GetRawInstance()->GetConsoleVariables();
    cvars->ClearVariable(StringHelper::Sprintf("%s.AxisDirectionMappingClass", key.c_str()).c_str());
    cvars->ClearVariable(StringHelper::Sprintf("%s.Stick", key.c_str()).c_str());
    cvars->ClearVariable(StringHelper::Sprintf("%s.Direction", key.c_str()).c_str());
    cvars->ClearVariable(StringHelper::Sprintf("%s.DSUButton", key.c_str()).c_str());
    cvars->Save();
}

int8_t DSUButtonToAxisDirectionMapping::GetMappingType() {
    return MAPPING_TYPE_GAMEPAD;
}

std::string DSUButtonToAxisDirectionMapping::GetPhysicalDeviceName() {
    return DSUButtonToAnyMapping::GetPhysicalDeviceName();
}

std::string DSUButtonToAxisDirectionMapping::GetPhysicalInputName() {
    return DSUButtonToAnyMapping::GetPhysicalInputName();
}
} // namespace Ship
