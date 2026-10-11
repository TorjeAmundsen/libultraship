#pragma once

#include "ship/controller/controldevice/controller/mapping/ControllerAxisDirectionMapping.h"
#include "ship/controller/controldevice/controller/mapping/dsu/DSUButtonToAnyMapping.h"

namespace Ship {

/**
 * @brief Maps a DSU button to a virtual stick direction at full deflection.
 */
class DSUButtonToAxisDirectionMapping final : public ControllerAxisDirectionMapping, public DSUButtonToAnyMapping {
  public:
    /**
     * @param portIndex  The controller port index.
     * @param stickIndex Which analog stick this mapping targets.
     * @param direction  The stick direction to push while the button is held.
     * @param dsuButton  The DSUButton to read.
     */
    DSUButtonToAxisDirectionMapping(uint8_t portIndex, StickIndex stickIndex, Direction direction, int32_t dsuButton);

    /** @brief Returns the deflection for this direction, 0 to MAX_AXIS_RANGE. */
    float GetNormalizedAxisDirectionValue() override;

    /** @brief Returns the unique string identifier for this mapping. */
    std::string GetAxisDirectionMappingId() override;

    /** @brief Returns the mapping type identifier. */
    int8_t GetMappingType() override;

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
