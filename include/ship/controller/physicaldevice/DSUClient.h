#pragma once

#include <array>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "ship/controller/physicaldevice/DSUPadState.h"

namespace Ship {

/** @brief What the DSU client is doing, for status lines. */
enum class DSUStatus {
    Off,          ///< Not started, or stopped.
    HostNotFound, ///< The host name doesn't resolve. Retrying every second.
    Waiting,      ///< No pad data in the last 2 seconds: no server, wrong port, or nothing connected to it.
    Receiving,    ///< Pad data is arriving.
};

/**
 * @brief Receives controller data from a DSU (cemuhook) server over UDP.
 *
 * DSU servers (input tools, motion apps, external controller adapters) stream controller state
 * to clients that subscribe. This client subscribes to every slot, keeps the latest state for
 * each on a background thread, and lets the game thread take a consistent copy once per frame
 * with LatchState(). Mappings then read the latched copy, so reading never waits on the network.
 *
 * Like SDL gamepads, each slot can be ignored per port. By default every slot drives port 1 only.
 */
class DSUClient {
  public:
    DSUClient();
    ~DSUClient();

    /**
     * @brief Starts receiving from the server at host:port, restarting if already running.
     *
     * Returns right away, and so does Stop(): the receiver thread resolves the host and opens the
     * socket on its own, retrying every second until that works, and a stopped thread finishes by
     * itself (a host name lookup can take seconds). If no pad data arrives for a few seconds, it
     * resolves and reconnects again, so a changed address is picked up.
     */
    void Start(const std::string& host, uint16_t port);

    /** @brief Stops receiving. Every slot reads as disconnected afterwards. */
    void Stop();

    /** @brief True between Start() and Stop(), unless networking failed to start. */
    bool IsRunning() const;

    /** @brief What the client is doing right now. */
    DSUStatus GetStatus() const;

    /** @brief The server as "host:port" ("[host]:port" for IPv6), or empty when stopped. */
    std::string GetServer() const;

    /**
     * @brief Copies the latest received state for the game thread. Call once per frame.
     *
     * Servers send pad data continuously, neutral or not. A slot that has sent nothing for half a
     * second reads neutral, so a server that goes away mid-press doesn't leave the input held, and
     * after 2 seconds it reads disconnected.
     */
    void LatchState();

    /** @brief The latched state of a slot. Disconnected if the slot is out of range. */
    const DSUPadState& GetLatchedState(uint8_t slot) const;

    /** @brief Latched slots that are connected, whichever ports use them. */
    std::vector<uint8_t> GetConnectedSlots() const;

    /** @brief Latched slots that are connected and not ignored by the port. */
    std::vector<uint8_t> GetConnectedSlotsForPort(uint8_t portIndex) const;

    bool PortIsIgnoringSlot(uint8_t portIndex, uint8_t slot) const;
    void IgnoreSlotForPort(uint8_t portIndex, uint8_t slot);
    void UnignoreSlotForPort(uint8_t portIndex, uint8_t slot);

  private:
    struct Run; ///< One Start(): shared with its receiver thread, which outlives Stop() if it's busy.

    /** @brief The receiver thread. It only touches its Run, so it can finish after Stop() and the client are gone. */
    static void ReceiveLoop(std::shared_ptr<Run> run);

    std::shared_ptr<Run> mRun;
    std::string mServer;
    std::array<DSUPadState, DSU_SLOT_COUNT> mLatched = {}; ///< Game thread only.
    std::unordered_map<uint8_t, std::unordered_set<uint8_t>> mIgnoredSlots;
};
} // namespace Ship
