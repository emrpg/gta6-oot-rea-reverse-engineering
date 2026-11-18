// Reconstructed from Round 2 incremental scan:
//   DottieDot/gtav-timecycle-editor (source/rage/TimecycleModifier.hpp)
//   RatPrez/ragekit (include/ragekit/rpf.h)
#pragma once

#include <cstdint>
#include <vector>

namespace Round2Rage {

#pragma pack(push, 1)
struct TimecycleModifierValue {
    uint32_t m_type;       // 0x0000
    float m_lightValue;    // 0x0004
    float m_darkValue;     // 0x0008
};

struct TimecycleModifierHeader {
    uint64_t m_elementsPtr; // 0x0000 (rage::atArray<TimecycleModifierValue>)
    uint16_t m_count;       // 0x0008
    uint16_t m_capacity;    // 0x000A
    uint32_t m_pad0C;       // 0x000C
    uint32_t m_nameHash;    // 0x0010
    uint8_t  m_pad14[12];   // 0x0014
    uint32_t m_userFlags;   // 0x0020
};
#pragma pack(pop)

enum class Rpf7Encryption : uint32_t {
    None = 0x00000000,
    Open = 0x4E45504F, // "OPEN"
    Aes  = 0x0FFFFFF9,
    Ng   = 0x0FEFFFFF,
};

struct Rpf7KeySpec {
    static constexpr size_t AES_KEY_BYTES = 32;
    static constexpr size_t NG_KEY_COUNT = 101;
    static constexpr size_t NG_KEY_STRIDE = 272;
    static constexpr size_t NG_TABLE_WORDS = 17 * 16 * 256;
    static constexpr size_t HASH_LUT_BYTES = 256;
    static constexpr size_t AWC_KEY_BYTES = 16;
};

} // namespace Round2Rage
