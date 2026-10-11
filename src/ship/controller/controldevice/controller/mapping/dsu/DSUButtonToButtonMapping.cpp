#include "ship/controller/controldevice/controller/mapping/dsu/DSUButtonToButtonMapping.h"
#include "ship/utils/StringHelper.h"
#include "ship/config/ConsoleVariable.h"
#include "ship/Context.h"
#include "ship/controller/controldeck/ControlDeck.h"

namespace Ship {
DSUButtonToButtonMapping::DSUButtonToButtonMapping(uint8_t portIndex, CONTROLLERBUTTONS_T bitmask, int32_t dsuButton)
    : ControllerInputMapping(PhysicalDeviceType::DSU),
      ControllerButtonMapping(PhysicalDeviceType::DSU, portIndex, bitmask), DSUButtonToAnyMapping(dsuButton) {
}

void DSUButtonToButtonMapping::UpdatePad(CONTROLLERBUTTONS_T& padButtons) {
    if (Context::GetRawInstance()->GetControlDeck()->GamepadGameInputBlocked()) {
        return;
    }
    if (ButtonPressedForPort(mPortIndex)) {
        padButtons |= mBitmask;
    }
}

std::string DSUButtonToButtonMapping::GetButtonMappingId() {
    return StringHelper::Sprintf("P%d-B%d-DSUB%d", mPortIndex, mBitmask, mDSUButton);
}

void DSUButtonToButtonMapping::SaveToConfig() {
    const std::string key = CVAR_PREFIX_CONTROLLERS ".ButtonMappings." + GetButtonMappingId();
    const auto cvars = Ship::Context::GetRawInstance()->GetConsoleVariables();
    cvars->SetString(StringHelper::Sprintf("%s.ButtonMappingClass", key.c_str()).c_str(), "DSUButtonToButtonMapping");
    cvars->SetInteger(StringHelper::Sprintf("%s.Bitmask", key.c_str()).c_str(), mBitmask);
    cvars->SetInteger(StringHelper::Sprintf("%s.DSUButton", key.c_str()).c_str(), mDSUButton);
    cvars->Save();
}

void DSUButtonToButtonMapping::EraseFromConfig() {
    const std::string key = CVAR_PREFIX_CONTROLLERS ".ButtonMappings." + GetButtonMappingId();
    const auto cvars = Ship::Context::GetRawInstance()->GetConsoleVariables();
    cvars->ClearVariable(StringHelper::Sprintf("%s.ButtonMappingClass", key.c_str()).c_str());
    cvars->ClearVariable(StringHelper::Sprintf("%s.Bitmask", key.c_str()).c_str());
    cvars->ClearVariable(StringHelper::Sprintf("%s.DSUButton", key.c_str()).c_str());
    cvars->Save();
}

int8_t DSUButtonToButtonMapping::GetMappingType() {
    return MAPPING_TYPE_GAMEPAD;
}

std::string DSUButtonToButtonMapping::GetPhysicalDeviceName() {
    return DSUButtonToAnyMapping::GetPhysicalDeviceName();
}

std::string DSUButtonToButtonMapping::GetPhysicalInputName() {
    return DSUButtonToAnyMapping::GetPhysicalInputName();
}
} // namespace Ship
