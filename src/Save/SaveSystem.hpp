/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Persistent save/load of player progression to disk
*/

#ifndef SAVE_SYSTEM_HPP_
#define SAVE_SYSTEM_HPP_

#include <string>
#include <vector>

#include "World/Sim/ProgressionState.hpp"

namespace racer::save {

// Save file format:
// A simple line-based "key=value" text format (no third-party JSON dep),
// one file per profile: saves/profile_<name>.sav (relative to the
// executable's working directory). This keeps saves human-readable and
// easy to diff/hand-edit, at the cost of a little verbosity vs binary.
class SaveSystem {
public:
    // Directory (relative to CWD) where profile files are stored.
    static const char *saveDirectory();

    // Full relative path to a profile's save file, e.g.
    // "saves/profile_default.sav".
    static std::string profilePath(const std::string &profileName);

    // Writes `state` to disk under `profileName`. Creates the save
    // directory if needed. Returns false on I/O failure (never throws).
    static bool save(const std::string &profileName,
        const racer::world::ProgressionState &state);

    // Reads the profile's save file into `outState`. If the file is
    // missing or corrupt/unparseable, `outState` is reset to a fresh
    // default ProgressionState and false is returned (no crash).
    static bool load(const std::string &profileName,
        racer::world::ProgressionState &outState);

    // Lists profile names found on disk (derived from
    // "profile_<name>.sav" file names in the save directory). Empty if
    // the directory doesn't exist yet.
    static std::vector<std::string> listProfiles();

    // Name used when no explicit profile is selected.
    static const char *defaultProfileName();
};

} // namespace racer::save

#endif /* !SAVE_SYSTEM_HPP_ */
