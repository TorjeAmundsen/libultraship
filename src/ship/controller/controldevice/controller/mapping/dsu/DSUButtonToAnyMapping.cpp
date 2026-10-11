#include "ship/controller/controldevice/controller/mapping/dsu/DSUButtonToAnyMapping.h"

#include "ship/Context.h"
#include "ship/controller/controldeck/ControlDeck.h"
#include "ship/controller/physicaldevice/DSUClient.h"

namespace Ship {
DSUButtonToAnyMapping::DSUButtonToAnyMapping(int32_t dsuButton) : ControllerInputMapping(PhysicalDeviceType::DSU) {
    mDSUButton = static_cast<DSUButton>(dsuButton);
}

DSUButtonToAnyMapping::~DSUButtonToAnyMapping() {
}

std::string DSUButtonToAnyMapping::GetPhysicalInputName() {
    return DSUButtonName(mDSUButton);
}

std::string DSUButtonToAnyMapping::GetPhysicalDeviceName() {
    return "DSU Controller";
}

bool DSUButtonToAnyMapping::ButtonPressedForPort(uint8_t portIndex) {
    const auto client = Context::GetRawInstance()->GetControlDeck()->GetDSUClient();
    for (const uint8_t slot : client->GetConnectedSlotsForPort(portIndex)) {
        if (DSUButtonPressed(client->GetLatchedState(slot), mDSUButton)) {
            return true;
        }
    }
    return false;
}
} // namespace Ship
