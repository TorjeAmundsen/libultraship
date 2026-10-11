#pragma once

#include "ship/controller/controldevice/controller/mapping/ControllerButtonMapping.h"
#include "ship/controller/controldevice/controller/mapping/dsu/DSUAxisDirectionToAnyMapping.h"

namespace Ship {

/**
 * @brief Maps a DSU axis direction to a virtual controller button, pressed past the axis threshold.
 */
class DSUAxisDirectionToButtonMapping final : public ControllerButtonMapping, public DSUAxisDirectionToAnyMapping {
  public:
    /**
     * @param portIndex     The controller port index.
     * @param bitmask       The button bitmask to set while the axis is pushed far enough.
     * @param dsuAxis       The DSUAxis to read.
     * @param axisDirection The axis half to bind (NEGATIVE or POSITIVE).
     */
    DSUAxisDirectionToButtonMapping(uint8_t portIndex, CONTROLLERBUTTONS_T bitmask, int32_t dsuAxis,
                                    int32_t axisDirection);

    /** @brief Sets the button bitmask bit while the DSU input is held. */
    void UpdatePad(CONTROLLERBUTTONS_T& padButtons) override;

    /** @brief Returns the mapping type identifier. */
    int8_t GetMappingType() override;

    /** @brief Returns the unique string identifier for this mapping. */
    std::string GetButtonMappingId() override;

    /** @brief Persists this mapping to the application configuration. */
    void SaveToConfig() override;

    /** @brief Removes this mapping from the application configuration. */
    void EraseFromConfig() override;

    /** @brief Returns the human-readable name of the DSU device. */
    std::string GetPhysicalDeviceName() override;

    /** @brief Returns the human-readable name of the bound input. */
    std::string GetPhysicalInputName() override;
};
} // namespace Ship
