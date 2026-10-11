#pragma once

#include "ship/controller/controldevice/controller/mapping/ControllerAxisDirectionMapping.h"
#include "ship/controller/controldevice/controller/mapping/dsu/DSUAxisDirectionToAnyMapping.h"

namespace Ship {

/**
 * @brief Maps a DSU axis direction to a virtual stick direction.
 */
class DSUAxisDirectionToAxisDirectionMapping final : public ControllerAxisDirectionMapping,
                                                     public DSUAxisDirectionToAnyMapping {
  public:
    /**
     * @param portIndex     The controller port index.
     * @param stickIndex    Which analog stick this mapping targets.
     * @param direction     The stick direction to drive.
     * @param dsuAxis       The DSUAxis to read.
     * @param axisDirection The axis half to bind (NEGATIVE or POSITIVE). POSITIVE is up on Y axes.
     */
    DSUAxisDirectionToAxisDirectionMapping(uint8_t portIndex, StickIndex stickIndex, Direction direction,
                                           int32_t dsuAxis, int32_t axisDirection);

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
