/*
 * Reconstructed C++20 implementation of AWC & DAT54 Audio Bank / Radio Metadata Builder
 * Evidence source:
 *   - AWCMaster/src/generation/core.py (SimpleSound, _construct_awc, _construct_dat54, _construct_nametable)
 *   - native-audio-tool/src/audioTool.js (simple & radio modes, AWC/DAT54/DAT151 generation)
 */

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace audio_builder_reconstructed {

struct TrackSpec {
    std::string name;
    uint32_t samples = 0;
    uint32_t sample_rate = 48000;
    int32_t headroom = -100;
};

class AudioBankGenerator {
public:
    AudioBankGenerator(std::string bank_name, std::string soundset_name)
        : bank_name_(std::move(bank_name)), soundset_name_(std::move(soundset_name)) {}

    void add_track(TrackSpec track) {
        tracks_.push_back(std::move(track));
    }

    void write_nametable(const std::filesystem::path& out_dir) const {
        std::filesystem::create_directories(out_dir);
        std::ofstream out(out_dir / (bank_name_ + ".dat54.nametable"));
        for (size_t i = 0; i < tracks_.size(); i++) {
            out << tracks_[i].name << "_s ";
        }
        out << bank_name_;
    }

    std::string build_awc_xml() const {
        std::ostringstream oss;
        oss << "<AudioWaveContainer>\n  <Version value=\"1\" />\n  <Streams>\n";
        for (const auto& t : tracks_) {
            oss << "    <Item>\n"
                << "      <Name>" << t.name << "</Name>\n"
                << "      <Chunks>\n"
                << "        <Item><Type>peak</Type></Item>\n"
                << "        <Item><Type>data</Type></Item>\n"
                << "        <Item><Type>format</Type><Codec>PCM</Codec>"
                << "<Samples value=\"" << t.samples << "\" />"
                << "<SampleRate value=\"" << t.sample_rate << "\" />"
                << "<Headroom value=\"" << t.headroom << "\" /></Item>\n"
                << "      </Chunks>\n"
                << "    </Item>\n";
        }
        oss << "  </Streams>\n</AudioWaveContainer>\n";
        return oss.str();
    }

private:
    std::string bank_name_;
    std::string soundset_name_;
    std::vector<TrackSpec> tracks_;
};

} // namespace audio_builder_reconstructed
