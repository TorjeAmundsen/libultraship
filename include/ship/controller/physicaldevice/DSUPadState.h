#pragma once

#include <cstdint>

/** @brief Number of controller slots a DSU (cemuhook) server can report. */
#define DSU_SLOT_COUNT 4

/** @brief Default DSU server port, used by Cemuhook, Dolphin and most DSU servers. */
#define DSU_DEFAULT_PORT 26760

namespace Ship {

/**
 * @brief The state of one DSU controller slot, as the server last reported it.
 *
 * Stick bytes are 0 to 255 with 128 at center. On the Y axes 255 is up. Buttons follow the
 * cemuhook layout: buttons1 is Share, L3, R3, Options, D-Pad Up, Right, Down, Left from bit 0;
 * buttons2 is L2, R2, L1, R1, Triangle, Circle, Cross, Square from bit 0.
 */
struct DSUPadState {
    bool Connected = false;
    uint8_t Buttons1 = 0;
    uint8_t Buttons2 = 0;
    uint8_t Home = 0;
    uint8_t Touch = 0;
    uint8_t LeftX = 128;
    uint8_t LeftY = 128;
    uint8_t RightX = 128;
    uint8_t RightY = 128;
    uint8_t L2 = 0; ///< Analog trigger, 0 to 255.
    uint8_t R2 = 0; ///< Analog trigger, 0 to 255.
};

/**
 * @brief Reads a DSU server's pad data packet.
 *
 * Checks the magic, the length the header gives and the CRC32, then reads the slot and its
 * state. Returns false for anything else (other message types, short or corrupt packets).
 */
bool ParseDSUPadData(const uint8_t* data, int size, uint8_t& slot, DSUPadState& state);

} // namespace Ship
