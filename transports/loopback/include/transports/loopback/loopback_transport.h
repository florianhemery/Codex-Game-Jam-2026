#pragma once

#include <cstdint>
#include <string>

#include "common/transport/transport.h"

// Implementation "en memoire" de Transport : appelle server_core directement
// dans le meme process, sans aucune socket. Le header ne mentionne aucun type
// de server_core -- seuls des primitifs et Transport* traversent cette API,
// exactement ce que SocketTransport exposera plus tard. C'est ce qui garantit
// que client/ n'a jamais besoin de connaitre server_core, meme indirectement.

Transport* loopback_transport_create(const std::string& worldSaveDir, uint32_t seed, const std::string& playerName);
