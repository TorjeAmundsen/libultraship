#include "ship/controller/controldevice/controller/mapping/dsu/DSUAxisDirectionToAxisDirectionMapping.h"
#include "ship/utils/StringHelper.h"
#include "ship/config/ConsoleVariable.h"
#include "ship/Context.h"
#include "ship/controller/controldeck/ControlDeck.h"

namespace Ship {
DSUAxisDirectionToAxisDirectionMapping::DSUAxisDirectionToAxisDirectionMapping(uint8_t portIndex, StickIndex stickIndex,
                                                                               Direction direction, int32_t dsuAxis,
                                                                               int32_t axisDirection)
    : ControllerInputMapping(PhysicalDeviceType::DSU),
      ControllerAxisDirectionMapping(PhysicalDeviceType::DSU, portIndex, stickIndex, direction),
      DSUAxisDirectionToAnyMapping(dsuAxis, axisDirection) {
}

float DSUAxisDirectionToAxisDirectionMapping::GetNormalizedAxisDirectionValue() {
    if (Context::GetRawInstance()->GetControlDeck()->GamepadGameInputBlocked()) {
        return 0.0f;
    }
    return AxisDirectionValueForPort(mPortIndex) * MAX_AXIS_RANGE;
}

std::string DSUAxisDirectionToAxisDirectionMapping::GetAxisDirectionMappingId() {
    return StringHelper::Sprintf("P%d-S%d-D%d-DSUA%d-AD%s", mPortIndex, mStickIndex, mDirection, mDSUAxis,
                                 mAxisDirection == POSITIVE ? "P" : "N");
}

void DSUAxisDirectionToAxisDirectionMapping::SaveToConfig() {
    const std::string key = CVAR_PREFIX_CONTROLLERS ".AxisDirectionMappings." + GetAxisDirectionMappingId();
    const auto cvars = Ship::Context::GetRawInstance()->GetConsoleVariables();
    cvars->SetString(StringHelper::Sprintf("%s.AxisDirectionMappingClass", key.c_str()).c_str(),
                     "DSUAxisDirectionToAxisDirectionMapping");
    cvars->SetInteger(StringHelper::Sprintf("%s.Stick", key.c_str()).c_str(), mStickIndex);
    cvars->SetInteger(StringHelper::Sprintf("%s.Direction", key.c_str()).c_str(), mDirection);
    cvars->SetInteger(StringHelper::Sprintf("%s.DSUAxis", key.c_str()).c_str(), mDSUAxis);
    cvars->SetInteger(StringHelper::Sprintf("%s.AxisDirection", key.c_str()).c_str(), mAxisDirection);
    cvars->Save();
}

void DSUAxisDirectionToAxisDirectionMapping::EraseFromConfig() {
    const std::string key = CVAR_PREFIX_CONTROLLERS ".AxisDirectionMappings." + GetAxisDirectionMappingId();
    const auto cvars = Ship::Context::GetRawInstance()->GetConsoleVariables();
    cvars->ClearVariable(StringHelper::Sprintf("%s.AxisDirectionMappingClass", key.c_str()).c_str());
    cvars->ClearVariable(StringHelper::Sprintf("%s.Stick", key.c_str()).c_str());
    cvars->ClearVariable(StringHelper::Sprintf("%s.Direction", key.c_str()).c_str());
    cvars->ClearVariable(StringHelper::Sprintf("%s.DSUAxis", key.c_str()).c_str());
    cvars->ClearVariable(StringHelper::Sprintf("%s.AxisDirection", key.c_str()).c_str());
    cvars->Save();
}

int8_t DSUAxisDirectionToAxisDirectionMapping::GetMappingType() {
    return MAPPING_TYPE_GAMEPAD;
}

std::string DSUAxisDirectionToAxisDirectionMapping::GetPhysicalDeviceName() {
    return DSUAxisDirectionToAnyMapping::GetPhysicalDeviceName();
}

std::string DSUAxisDirectionToAxisDirectionMapping::GetPhysicalInputName() {
    return DSUAxisDirectionToAnyMapping::GetPhysicalInputName();
}
} // namespace Ship
