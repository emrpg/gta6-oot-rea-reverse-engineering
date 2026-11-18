// Reconstructed from REA static evidence of saat_sfx.exe & saat_stream.exe
// San Andreas Audio Toolkit (SAAT) v1.10 (PE32 MinGW + statically linked libsndfile)
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace SAAT {

constexpr int MIN_SFX_LOOKUP_INDEX = 0;
constexpr int MAX_SFX_LOOKUP_INDEX = 8;
constexpr int MIN_SFX_BANKS = 1;
constexpr int MAX_SFX_BANKS = 400;

constexpr int BEAT_END_TOKEN = 33;

struct SfxArchiveConfig {
    int lookupIndex = 0; // [0-8]
    int numBanks = 0;    // [1-400]
    std::vector<std::string> bankDirs;
};

struct StreamTrackConfig {
    int beatTotal = 0;
    std::vector<int> beats; // Terminated by BEAT_END_TOKEN (33)
};

struct StreamArchiveConfig {
    int lookupIndex = 0; // [0-1] or [3-16]
    int numTracks = 0;   // >= 1
    std::vector<StreamTrackConfig> tracks;
};

int RunSfxExport(const std::string& archiveFiles, const std::string& targetDir, const std::string& lookupFile);
int RunSfxImport(const std::string& targetArchive, const std::string& importIni, const std::string& lookupFile);

int RunStreamExport(const std::string& streamFiles, const std::string& targetDir, const std::string& metadataFile = "");
int RunStreamImport(const std::string& targetStream, const std::string& importIni, const std::string& lookupFile);

} // namespace SAAT
