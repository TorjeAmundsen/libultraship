#pragma once

#include "ship/controller/controldevice/controller/mapping/dsu/DSUMapping.h"
#include "ship/controller/controldevice/controller/mapping/ControllerInputMapping.h"

namespace Ship {

/**
 * @brief Base class for mappings that bind a DSU (cemuhook) button to any controller input.
 *
 * Reads the button from every DSU slot the port is using, like SDL mappings do with gamepads.
 */
class DSUButtonToAnyMapping : virtual public ControllerInputMapping {
  public:
    /**
     * @brief Constructs a DSU button mapping.
     * @param dsuButton The DSUButton to read.
     */
    DSUButtonToAnyMapping(int32_t dsuButton);

    /** @brief Destructor. */
    virtual ~DSUButtonToAnyMapping();

    /** @brief Returns the human-readable name of the bound button. */
    std::string GetPhysicalInputName() override;

    /** @brief Returns the human-readable name of the DSU device. */
    std::string GetPhysicalDeviceName() override;

  protected:
    /** @brief True if the button is held on any DSU slot the port is using. */
    bool ButtonPressedForPort(uint8_t portIndex);

    DSUButton mDSUButton;
};
} // namespace Ship
