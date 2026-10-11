#pragma once

#include "ship/controller/controldevice/controller/mapping/dsu/DSUMapping.h"
#include "ship/controller/controldevice/controller/mapping/ControllerInputMapping.h"

namespace Ship {

/**
 * @brief Base class for mappings that bind a DSU (cemuhook) axis direction to any controller input.
 *
 * Reads the axis from every DSU slot the port is using, like SDL mappings do with gamepads.
 */
class DSUAxisDirectionToAnyMapping : virtual public ControllerInputMapping {
  public:
    /**
     * @brief Constructs a DSU axis-direction mapping.
     * @param dsuAxis       The DSUAxis to read.
     * @param axisDirection The axis half to bind (NEGATIVE or POSITIVE). POSITIVE is up on Y axes.
     */
    DSUAxisDirectionToAnyMapping(int32_t dsuAxis, int32_t axisDirection);

    /** @brief Destructor. */
    virtual ~DSUAxisDirectionToAnyMapping();

    /** @brief Returns the human-readable name of the bound axis and direction. */
    std::string GetPhysicalInputName() override;

    /** @brief Returns the human-readable name of the DSU device. */
    std::string GetPhysicalDeviceName() override;

    /** @brief Tests whether this axis is an analog trigger (L2 / R2). */
    bool AxisIsTrigger();

  protected:
    /** @brief The largest deflection in this direction, 0 to 1, across the DSU slots the port is using. */
    float AxisDirectionValueForPort(uint8_t portIndex);

    DSUAxis mDSUAxis;
    AxisDirection mAxisDirection;
};
} // namespace Ship
