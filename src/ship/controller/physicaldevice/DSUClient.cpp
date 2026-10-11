#include "ship/controller/physicaldevice/DSUClient.h"

#include <atomic>
#include <chrono>
#include <cstring>
#include <mutex>
#include <random>
#include <thread>
#include <spdlog/spdlog.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#endif

namespace Ship {

namespace {
// Cemuhook protocol: https://v1993.github.io/cemuhook-protocol/
constexpr uint16_t PROTOCOL_VERSION = 1001;
constexpr uint32_t MESSAGE_PORT_INFO = 0x100001;
constexpr uint32_t MESSAGE_PAD_DATA = 0x100002;
constexpr int HEADER_SIZE = 20;    // magic, version, length, CRC32, sender id, message type
constexpr int PAD_DATA_SIZE = 100; // a full pad data packet, header included
constexpr auto RESUBSCRIBE_INTERVAL = std::chrono::seconds(1);
constexpr auto RETRY_INTERVAL = std::chrono::seconds(1);       // between attempts to resolve and connect
constexpr auto INPUT_TIMEOUT = std::chrono::milliseconds(500); // a slot with no packets for this long reads neutral
constexpr auto SLOT_TIMEOUT = std::chrono::seconds(2);         // and after this long, disconnected
constexpr auto RECONNECT_AFTER = std::chrono::seconds(5);      // no pad data for this long: resolve and connect again
constexpr int RECEIVE_TIMEOUT_MS = 20;                         // how often the thread wakes up to check on things
constexpr uint8_t SLOT_STATE_CONNECTED = 2;

#ifdef _WIN32
using SocketHandle = SOCKET;
const SocketHandle INVALID_SOCKET_HANDLE = INVALID_SOCKET;
#else
using SocketHandle = int;
const SocketHandle INVALID_SOCKET_HANDLE = -1;
#endif

void CloseSocket(SocketHandle socketHandle) {
#ifdef _WIN32
    closesocket(socketHandle);
#else
    close(socketHandle);
#endif
}

uint32_t Crc32(const uint8_t* data, size_t size) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < size; i++) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; bit++) {
            crc = (crc >> 1) ^ (0xEDB88320 & (0 - (crc & 1)));
        }
    }
    return ~crc;
}

template <typename T> T Read(const uint8_t* data, int offset) {
    T value;
    std::memcpy(&value, data + offset, sizeof(T));
    return value;
}

template <typename T> void Write(uint8_t* data, int offset, T value) {
    std::memcpy(data + offset, &value, sizeof(T));
}

// A client message: header, then the payload. The CRC is computed with its own field at 0.
std::vector<uint8_t> MakeMessage(uint32_t clientId, uint32_t messageType, const std::vector<uint8_t>& payload) {
    std::vector<uint8_t> message(HEADER_SIZE + payload.size());
    std::memcpy(message.data(), "DSUC", 4);
    Write<uint16_t>(message.data(), 4, PROTOCOL_VERSION);
    Write<uint16_t>(message.data(), 6, static_cast<uint16_t>(message.size() - 16)); // counted from the message type
    Write<uint32_t>(message.data(), 12, clientId);
    Write<uint32_t>(message.data(), 16, messageType);
    std::memcpy(message.data() + HEADER_SIZE, payload.data(), payload.size());
    Write<uint32_t>(message.data(), 8, Crc32(message.data(), message.size()));
    return message;
}

enum class OpenResult { Opened, HostNotFound, Failed };

// Resolves host:port (IPv4 or IPv6) and opens a UDP socket connected to it, so only the server's
// packets arrive.
OpenResult OpenSocket(const std::string& host, uint16_t port, SocketHandle& socketHandle) {
    addrinfo hints = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    addrinfo* result = nullptr;
    if (getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &result) != 0 || result == nullptr) {
        return OpenResult::HostNotFound;
    }
    socketHandle = INVALID_SOCKET_HANDLE;
    for (const addrinfo* address = result; address != nullptr && socketHandle == INVALID_SOCKET_HANDLE;
         address = address->ai_next) {
        socketHandle = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
        if (socketHandle != INVALID_SOCKET_HANDLE &&
            connect(socketHandle, address->ai_addr, static_cast<int>(address->ai_addrlen)) != 0) {
            CloseSocket(socketHandle);
            socketHandle = INVALID_SOCKET_HANDLE;
        }
    }
    freeaddrinfo(result);
    if (socketHandle == INVALID_SOCKET_HANDLE) {
        return OpenResult::Failed;
    }

    // Wake up regularly to resubscribe and to notice Stop().
#ifdef _WIN32
    DWORD timeout = RECEIVE_TIMEOUT_MS;
#else
    timeval timeout = { 0, RECEIVE_TIMEOUT_MS * 1000 };
#endif
    setsockopt(socketHandle, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));
    return OpenResult::Opened;
}
} // namespace

bool ParseDSUPadData(const uint8_t* data, int size, uint8_t& slot, DSUPadState& state) {
    if (size < HEADER_SIZE || size > 512 || std::memcmp(data, "DSUS", 4) != 0) {
        return false;
    }
    // The CRC covers the length the header gives. Anything after that is ignored.
    const int messageSize = Read<uint16_t>(data, 6) + 16;
    if (messageSize < HEADER_SIZE || messageSize > size) {
        return false;
    }
    uint8_t copy[512];
    std::memcpy(copy, data, messageSize);
    Write<uint32_t>(copy, 8, 0);
    if (Crc32(copy, messageSize) != Read<uint32_t>(data, 8)) {
        return false;
    }
    if (Read<uint32_t>(data, 16) != MESSAGE_PAD_DATA || messageSize < PAD_DATA_SIZE || data[20] >= DSU_SLOT_COUNT) {
        return false;
    }

    slot = data[20];
    state = DSUPadState{};
    state.Connected = data[21] == SLOT_STATE_CONNECTED;
    state.Buttons1 = data[36];
    state.Buttons2 = data[37];
    state.Home = data[38];
    state.Touch = data[39];
    state.LeftX = data[40];
    state.LeftY = data[41];
    state.RightX = data[42];
    state.RightY = data[43];
    state.R2 = data[54];
    state.L2 = data[55];
    return true;
}

struct DSUClient::Run {
    std::string Host;
    uint16_t Port = 0;
    std::atomic<bool> Stopping{ false };
    std::atomic<bool> Failed{ false };       ///< Networking didn't start; the thread has exited.
    std::atomic<bool> HostNotFound{ false }; ///< The last attempt to resolve the host failed.

    std::mutex Mutex; ///< Guards everything below.
    std::array<DSUPadState, DSU_SLOT_COUNT> Received = {};
    std::array<std::chrono::steady_clock::time_point, DSU_SLOT_COUNT> LastPacketTime = {};

    std::string Server() const {
        return (Host.find(':') != std::string::npos ? "[" + Host + "]" : Host) + ":" + std::to_string(Port);
    }
};

DSUClient::DSUClient() {
    // Like SDL gamepads, every slot drives port 1 and the other ports ignore it until told otherwise.
    for (uint8_t port = 1; port < 4; port++) {
        for (uint8_t slot = 0; slot < DSU_SLOT_COUNT; slot++) {
            mIgnoredSlots[port].insert(slot);
        }
    }
}

DSUClient::~DSUClient() {
    Stop();
}

void DSUClient::Start(const std::string& host, uint16_t port) {
    Stop();
    mRun = std::make_shared<Run>();
    mRun->Host = host;
    mRun->Port = port;
    mServer = mRun->Server();
    std::thread(ReceiveLoop, mRun).detach();
}

void DSUClient::Stop() {
    if (mRun != nullptr) {
        mRun->Stopping = true;
        mRun.reset();
    }
    mServer.clear();
    mLatched = {};
}

bool DSUClient::IsRunning() const {
    return mRun != nullptr && !mRun->Failed;
}

DSUStatus DSUClient::GetStatus() const {
    if (!IsRunning()) {
        return DSUStatus::Off;
    }
    const auto now = std::chrono::steady_clock::now();
    {
        std::lock_guard lock(mRun->Mutex);
        for (const auto& time : mRun->LastPacketTime) {
            if (now - time <= SLOT_TIMEOUT) {
                return DSUStatus::Receiving;
            }
        }
    }
    return mRun->HostNotFound ? DSUStatus::HostNotFound : DSUStatus::Waiting;
}

std::string DSUClient::GetServer() const {
    return mServer;
}

void DSUClient::ReceiveLoop(std::shared_ptr<Run> run) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        SPDLOG_ERROR("DSU client: networking failed to start");
        run->Failed = true;
        return;
    }
#endif
    const std::string server = run->Server();
    const uint32_t clientId = std::random_device{}();
    // Subscribe to every slot (flags 0), and list the slots as Dolphin does, both every second.
    // Servers keep sending pad data for a few seconds per request. The slot list replies aren't
    // needed: pad data says which slots are connected.
    const std::vector<uint8_t> padDataRequest = MakeMessage(clientId, MESSAGE_PAD_DATA, { 0, 0, 0, 0, 0, 0, 0, 0 });
    const std::vector<uint8_t> portInfoRequest =
        MakeMessage(clientId, MESSAGE_PORT_INFO, { DSU_SLOT_COUNT, 0, 0, 0, 0, 1, 2, 3 });

    // Never logs after Stop(): the logger may be going away.
    auto log = [&](bool error, const std::string& text) {
        if (run->Stopping) {
            return;
        }
        if (error) {
            SPDLOG_ERROR("DSU client: {}", text);
        } else {
            SPDLOG_INFO("DSU client: {}", text);
        }
    };

    SocketHandle socketHandle = INVALID_SOCKET_HANDLE;
    auto lastAttempt = std::chrono::steady_clock::time_point{};
    auto lastRequest = std::chrono::steady_clock::time_point{};
    auto lastPadData = std::chrono::steady_clock::time_point{}; // or when the socket opened
    bool reportedNotFound = false, reportedWaiting = false, receiving = false;
    uint8_t buffer[512];
    while (!run->Stopping) {
        const auto now = std::chrono::steady_clock::now();
        if (socketHandle == INVALID_SOCKET_HANDLE) {
            if (now - lastAttempt < RETRY_INTERVAL) {
                std::this_thread::sleep_for(std::chrono::milliseconds(RECEIVE_TIMEOUT_MS));
                continue;
            }
            lastAttempt = now;
            const OpenResult opened = OpenSocket(run->Host, run->Port, socketHandle);
            run->HostNotFound = opened == OpenResult::HostNotFound;
            if (opened == OpenResult::HostNotFound) {
                if (!reportedNotFound) {
                    log(true, "can't find " + run->Host + ", retrying");
                    reportedNotFound = true;
                }
                continue;
            }
            reportedNotFound = false;
            if (opened == OpenResult::Failed) {
                continue;
            }
            if (!reportedWaiting) {
                log(false, "waiting for a server at " + server);
                reportedWaiting = true;
            }
            lastPadData = now;
            lastRequest = {};
        }

        if (now - lastRequest >= RESUBSCRIBE_INTERVAL) {
            const bool sent = send(socketHandle, reinterpret_cast<const char*>(portInfoRequest.data()),
                                   static_cast<int>(portInfoRequest.size()), 0) >= 0 &&
                              send(socketHandle, reinterpret_cast<const char*>(padDataRequest.data()),
                                   static_cast<int>(padDataRequest.size()), 0) >= 0;
            lastRequest = now;
            // A failed send, or a long silence, can mean the address changed (a new IP, a host name
            // that points elsewhere now): resolve and connect again.
            if (!sent || now - lastPadData > RECONNECT_AFTER) {
                CloseSocket(socketHandle);
                socketHandle = INVALID_SOCKET_HANDLE;
                continue;
            }
        }

        const int size = recv(socketHandle, reinterpret_cast<char*>(buffer), sizeof(buffer), 0);
        uint8_t slot;
        DSUPadState state;
        if (size > 0 && ParseDSUPadData(buffer, size, slot, state)) {
            const auto received = std::chrono::steady_clock::now();
            lastPadData = received;
            {
                std::lock_guard lock(run->Mutex);
                run->Received[slot] = state;
                run->LastPacketTime[slot] = received;
            }
            if (!receiving) {
                log(false, "receiving from " + server);
                receiving = true;
            }
        } else if (receiving && std::chrono::steady_clock::now() - lastPadData > SLOT_TIMEOUT) {
            log(false, "no data from " + server + ", waiting");
            receiving = false;
        }
    }

    if (socketHandle != INVALID_SOCKET_HANDLE) {
        CloseSocket(socketHandle);
    }
#ifdef _WIN32
    WSACleanup();
#endif
}

void DSUClient::LatchState() {
    if (mRun == nullptr) {
        return;
    }
    const auto now = std::chrono::steady_clock::now();
    std::lock_guard lock(mRun->Mutex);
    for (uint8_t slot = 0; slot < DSU_SLOT_COUNT; slot++) {
        const auto age = now - mRun->LastPacketTime[slot];
        if (age > SLOT_TIMEOUT) {
            mLatched[slot] = DSUPadState{};
        } else if (age > INPUT_TIMEOUT) {
            // Still connected, but don't keep a press going without fresh data.
            mLatched[slot] = DSUPadState{};
            mLatched[slot].Connected = mRun->Received[slot].Connected;
        } else {
            mLatched[slot] = mRun->Received[slot];
        }
    }
}

const DSUPadState& DSUClient::GetLatchedState(uint8_t slot) const {
    static const DSUPadState sDisconnected;
    return slot < DSU_SLOT_COUNT ? mLatched[slot] : sDisconnected;
}

std::vector<uint8_t> DSUClient::GetConnectedSlots() const {
    std::vector<uint8_t> slots;
    for (uint8_t slot = 0; slot < DSU_SLOT_COUNT; slot++) {
        if (mLatched[slot].Connected) {
            slots.push_back(slot);
        }
    }
    return slots;
}

std::vector<uint8_t> DSUClient::GetConnectedSlotsForPort(uint8_t portIndex) const {
    std::vector<uint8_t> slots;
    for (const uint8_t slot : GetConnectedSlots()) {
        if (!PortIsIgnoringSlot(portIndex, slot)) {
            slots.push_back(slot);
        }
    }
    return slots;
}

bool DSUClient::PortIsIgnoringSlot(uint8_t portIndex, uint8_t slot) const {
    const auto ignored = mIgnoredSlots.find(portIndex);
    return ignored != mIgnoredSlots.end() && ignored->second.contains(slot);
}

void DSUClient::IgnoreSlotForPort(uint8_t portIndex, uint8_t slot) {
    mIgnoredSlots[portIndex].insert(slot);
}

void DSUClient::UnignoreSlotForPort(uint8_t portIndex, uint8_t slot) {
    mIgnoredSlots[portIndex].erase(slot);
}
} // namespace Ship
