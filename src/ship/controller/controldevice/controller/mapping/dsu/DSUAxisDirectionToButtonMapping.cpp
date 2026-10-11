#include "ship/controller/controldevice/controller/mapping/dsu/DSUAxisDirectionToButtonMapping.h"
#include "ship/utils/StringHelper.h"
#include "ship/config/ConsoleVariable.h"
#include "ship/Context.h"
#include "ship/controller/controldeck/ControlDeck.h"

namespace Ship {
DSUAxisDirectionToButtonMapping::DSUAxisDirectionToButtonMapping(uint8_t portIndex, CONTROLLERBUTTONS_T bitmask,
                                                                 int32_t dsuAxis, int32_t axisDirection)
    : ControllerInputMapping(PhysicalDeviceType::DSU),
      ControllerButtonMapping(PhysicalDeviceType::DSU, portIndex, bitmask),
      DSUAxisDirectionToAnyMapping(dsuAxis, axisDirection) {
}

void DSUAxisDirectionToButtonMapping::UpdatePad(CONTROLLERBUTTONS_T& padButtons) {
    if (Context::GetRawInstance()->GetControlDeck()->GamepadGameInputBlocked()) {
        return;
    }
    // The same thresholds as SDL gamepads.
    const auto settings = Context::GetRawInstance()->GetControlDeck()->GetGlobalSDLDeviceSettings();
    const int32_t thresholdPercentage =
        AxisIsTrigger() ? settings->GetTriggerAxisThresholdPercentage() : settings->GetStickAxisThresholdPercentage();
    if (AxisDirectionValueForPort(mPortIndex) * 100.0f > thresholdPercentage) {
        padButtons |= mBitmask;
    }
}

std::string DSUAxisDirectionToButtonMapping::GetButtonMappingId() {
    return StringHelper::Sprintf("P%d-B%d-DSUA%d-AD%s", mPortIndex, mBitmask, mDSUAxis,
                                 mAxisDirection == POSITIVE ? "P" : "N");
}

void DSUAxisDirectionToButtonMapping::SaveToConfig() {
    const std::string key = CVAR_PREFIX_CONTROLLERS ".ButtonMappings." + GetButtonMappingId();
    const auto cvars = Ship::Context::GetRawInstance()->GetConsoleVariables();
    cvars->SetString(StringHelper::Sprintf("%s.ButtonMappingClass", key.c_str()).c_str(),
                     "DSUAxisDirectionToButtonMapping");
    cvars->SetInteger(StringHelper::Sprintf("%s.Bitmask", key.c_str()).c_str(), mBitmask);
    cvars->SetInteger(StringHelper::Sprintf("%s.DSUAxis", key.c_str()).c_str(), mDSUAxis);
    cvars->SetInteger(StringHelper::Sprintf("%s.AxisDirection", key.c_str()).c_str(), mAxisDirection);
    cvars->Save();
}

void DSUAxisDirectionToButtonMapping::EraseFromConfig() {
    const std::string key = CVAR_PREFIX_CONTROLLERS ".ButtonMappings." + GetButtonMappingId();
    const auto cvars = Ship::Context::GetRawInstance()->GetConsoleVariables();
    cvars->ClearVariable(StringHelper::Sprintf("%s.ButtonMappingClass", key.c_str()).c_str());
    cvars->ClearVariable(StringHelper::Sprintf("%s.Bitmask", key.c_str()).c_str());
    cvars->ClearVariable(StringHelper::Sprintf("%s.DSUAxis", key.c_str()).c_str());
    cvars->ClearVariable(StringHelper::Sprintf("%s.AxisDirection", key.c_str()).c_str());
    cvars->Save();
}

int8_t DSUAxisDirectionToButtonMapping::GetMappingType() {
    return MAPPING_TYPE_GAMEPAD;
}

std::string DSUAxisDirectionToButtonMapping::GetPhysicalDeviceName() {
    return DSUAxisDirectionToAnyMapping::GetPhysicalDeviceName();
}

std::string DSUAxisDirectionToButtonMapping::GetPhysicalInputName() {
    return DSUAxisDirectionToAnyMapping::GetPhysicalInputName();
}
} // namespace Ship
