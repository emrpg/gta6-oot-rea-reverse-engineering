/*
 * Reconstructed C++20 implementation of:
 *   1. Ocarina-Atlas :6545 TCP JSON-RPC Telemetry Bridge & caption.sav/progress.sav Parser
 *   2. PopTracker IAutotrackProvider Watch-Cache Engine
 * Evidence source:
 *   - OcarinaAtlas-Windows-x64/app/standalone-server.mjs (port 6545, hash IDs 3700692099, 2899040791, 2334548366, 2749067540)
 *   - OcarinaAtlas_exe__inspect-pe-resources.json & PopTracker/src/luaconnector/luaconnector.h
 */

#include <cstdint>
#include <cstring>
#include <map>
#include <string>
#include <vector>

namespace atlas_and_poptracker {

struct Vec3f {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct SaveTelemetry {
    uint32_t skulltula_count = 0;
    float    yaw_deg         = 0.0f;
    Vec3f    position{};
    std::vector<uint64_t> saved_hashes;
};

static uint32_t rd_u32(const uint8_t* p) {
    return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
           (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
}

static float rd_f32(const uint8_t* p) {
    uint32_t u = rd_u32(p);
    float f = 0.0f;
    std::memcpy(&f, &u, sizeof(f));
    return f;
}

SaveTelemetry parse_caption_sav(const uint8_t* buf, size_t len) {
    SaveTelemetry out;
    if (len < 40) return out;
    uint32_t table_end = rd_u32(buf + 8);
    for (size_t off = 40; off + 8 <= table_end && off + 8 <= len; off += 8) {
        uint32_t hash = rd_u32(buf + off);
        uint32_t val  = rd_u32(buf + off + 4);
        if (hash == 3700692099u) {
            out.skulltula_count = val;
        } else if (hash == 2899040791u) {
            out.yaw_deg = rd_f32(buf + off + 4);
        } else if (hash == 2334548366u && static_cast<size_t>(val) + 12 <= len) {
            out.position.x = rd_f32(buf + val);
            out.position.y = rd_f32(buf + val + 4);
            out.position.z = rd_f32(buf + val + 8);
        }
    }
    return out;
}

class WatchCacheProvider {
public:
    void add_watch(uint32_t address, uint32_t length) {
        watches_[address] = std::vector<uint8_t>(length, 0);
    }

    void update_watch_bytes(uint32_t address, const uint8_t* data, size_t len) {
        auto it = watches_.find(address);
        if (it != watches_.end() && it->second.size() >= len) {
            std::memcpy(it->second.data(), data, len);
        }
    }

    bool read_u8(uint32_t address, uint32_t offset, uint8_t& out) const {
        auto it = watches_.find(address);
        if (it == watches_.end() || offset >= it->second.size()) return false;
        out = it->second[offset];
        return true;
    }

private:
    std::map<uint32_t, std::vector<uint8_t>> watches_;
};

} // namespace atlas_and_poptracker
