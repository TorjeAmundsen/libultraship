#include "ship/controller/controldevice/controller/mapping/dsu/DSUAxisDirectionToAnyMapping.h"

#include <algorithm>
#include "ship/Context.h"
#include "ship/controller/controldeck/ControlDeck.h"
#include "ship/controller/physicaldevice/DSUClient.h"

namespace Ship {
DSUAxisDirectionToAnyMapping::DSUAxisDirectionToAnyMapping(int32_t dsuAxis, int32_t axisDirection)
    : ControllerInputMapping(PhysicalDeviceType::DSU) {
    mDSUAxis = static_cast<DSUAxis>(dsuAxis);
    mAxisDirection = static_cast<AxisDirection>(axisDirection);
}

DSUAxisDirectionToAnyMapping::~DSUAxisDirectionToAnyMapping() {
}

std::string DSUAxisDirectionToAnyMapping::GetPhysicalInputName() {
    return DSUAxisDirectionName(mDSUAxis, mAxisDirection);
}

std::string DSUAxisDirectionToAnyMapping::GetPhysicalDeviceName() {
    return "DSU Controller";
}

bool DSUAxisDirectionToAnyMapping::AxisIsTrigger() {
    return mDSUAxis == DSU_AXIS_L2 || mDSUAxis == DSU_AXIS_R2;
}

float DSUAxisDirectionToAnyMapping::AxisDirectionValueForPort(uint8_t portIndex) {
    const auto client = Context::GetRawInstance()->GetControlDeck()->GetDSUClient();
    float value = 0.0f;
    for (const uint8_t slot : client->GetConnectedSlotsForPort(portIndex)) {
        value = std::max(value, DSUAxisDirectionValue(client->GetLatchedState(slot), mDSUAxis, mAxisDirection));
    }
    return value;
}
} // namespace Ship
