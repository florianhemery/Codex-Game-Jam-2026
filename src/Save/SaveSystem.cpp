/*
** EPITECH PROJECT, 2026
** racer
** File description:
** SaveSystem implementation
*/

#include "Save/SaveSystem.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <system_error>

#include "World/Aurelia/AureliaTypes.hpp"

namespace racer::save {

namespace {

constexpr const char *kSaveDir = "saves";
constexpr const char *kDefaultProfile = "default";
constexpr int kSaveFormatVersion = 1;

bool isSafeProfileChar(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
        || (c >= '0' && c <= '9') || c == '_' || c == '-';
}

// Keeps profile names filesystem-safe (no path traversal, no separators).
std::string sanitizeProfileName(const std::string &name)
{
    std::string safe;
    safe.reserve(name.size());
    for (char c : name) {
        safe.push_back(isSafeProfileChar(c) ? c : '_');
    }
    if (safe.empty()) {
        safe = kDefaultProfile;
    }
    return safe;
}

} // namespace

const char *SaveSystem::saveDirectory()
{
    return kSaveDir;
}

const char *SaveSystem::defaultProfileName()
{
    return kDefaultProfile;
}

std::string SaveSystem::profilePath(const std::string &profileName)
{
    std::string safe = sanitizeProfileName(profileName);
    return std::string(kSaveDir) + "/profile_" + safe + ".sav";
}

bool SaveSystem::save(const std::string &profileName,
    const racer::world::ProgressionState &state)
{
    std::error_code ec;
    std::filesystem::create_directories(kSaveDir, ec);
    if (ec) {
        return false;
    }

    const std::string path = profilePath(profileName);
    std::ofstream out(path, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        return false;
    }

    out << "version=" << kSaveFormatVersion << "\n";
    for (int i = 0; i < racer::world::ProgressionState::kRegionCount; ++i) {
        auto region = static_cast<racer::world::RegionId>(i);
        out << "reputation_" << i << "=" << state.reputation(region) << "\n";
    }
    out << "garageMask=" << static_cast<unsigned>(state.garageMask()) << "\n";
    out << "loreMask=" << state.loreMask() << "\n";

    out.close();
    return !out.fail();
}

bool SaveSystem::load(const std::string &profileName,
    racer::world::ProgressionState &outState)
{
    // Always start from a clean slate so partial/corrupt files can't
    // leave outState half-mutated.
    outState = racer::world::ProgressionState();

    const std::string path = profilePath(profileName);
    std::ifstream in(path);
    if (!in.is_open()) {
        return false;
    }

    racer::world::ProgressionState parsed;
    bool sawVersion = false;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) {
            continue;
        }
        size_t eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);
        if (key.empty() || value.empty()) {
            continue;
        }

        try {
            if (key == "version") {
                sawVersion = true;
            } else if (key.rfind("reputation_", 0) == 0) {
                int idx = std::stoi(key.substr(11));
                if (idx < 0 || idx >= racer::world::ProgressionState::kRegionCount) {
                    continue;
                }
                int amount = std::stoi(value);
                parsed.setReputation(
                    static_cast<racer::world::RegionId>(idx), amount);
            } else if (key == "garageMask") {
                unsigned long mask = std::stoul(value);
                parsed.setGarageMask(static_cast<std::uint8_t>(mask));
            } else if (key == "loreMask") {
                unsigned long mask = std::stoul(value);
                parsed.setLoreMask(static_cast<std::uint32_t>(mask));
            }
        } catch (const std::exception &) {
            // Corrupt line: skip it and keep parsing the rest of the
            // file rather than failing the whole load.
            continue;
        }
    }

    if (!sawVersion) {
        // Doesn't look like one of our save files; treat as corrupt and
        // fall back to the default state already left in outState.
        return false;
    }

    outState = parsed;
    return true;
}

std::vector<std::string> SaveSystem::listProfiles()
{
    std::vector<std::string> profiles;
    std::error_code ec;
    if (!std::filesystem::exists(kSaveDir, ec) || ec) {
        return profiles;
    }

    static const std::string prefix = "profile_";
    static const std::string suffix = ".sav";
    for (const auto &entry :
        std::filesystem::directory_iterator(kSaveDir, ec)) {
        if (ec) {
            break;
        }
        if (!entry.is_regular_file()) {
            continue;
        }
        std::string filename = entry.path().filename().string();
        if (filename.size() <= prefix.size() + suffix.size()) {
            continue;
        }
        if (filename.compare(0, prefix.size(), prefix) != 0) {
            continue;
        }
        if (filename.compare(filename.size() - suffix.size(),
                suffix.size(), suffix) != 0) {
            continue;
        }
        profiles.push_back(filename.substr(
            prefix.size(), filename.size() - prefix.size() - suffix.size()));
    }
    return profiles;
}

} // namespace racer::save
