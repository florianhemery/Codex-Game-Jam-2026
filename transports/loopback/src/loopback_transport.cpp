#include "transports/loopback/loopback_transport.h"

#include "server_core/server_core.h"

namespace {

struct LoopbackState {
    server_core::ServerCore* core = nullptr;
    server_core::ClientId localClientId = 0;
};

void lb_send_reliable(Transport* self, const common::messages::ReliableMessage& msg) {
    auto* st = static_cast<LoopbackState*>(self->impl);
    server_core::submit_reliable(st->core, st->localClientId, msg);
}

bool lb_poll_reliable(Transport* self, common::messages::ReliableMessage& out) {
    auto* st = static_cast<LoopbackState*>(self->impl);
    return server_core::poll_outgoing_reliable(st->core, st->localClientId, out);
}

void lb_send_unreliable(Transport* self, const common::messages::UnreliableMessage& msg) {
    auto* st = static_cast<LoopbackState*>(self->impl);
    server_core::submit_unreliable(st->core, st->localClientId, msg);
}

bool lb_poll_unreliable(Transport* self, common::messages::UnreliableMessage& out) {
    auto* st = static_cast<LoopbackState*>(self->impl);
    return server_core::poll_outgoing_unreliable(st->core, st->localClientId, out);
}

void lb_tick(Transport* self, float dt) {
    auto* st = static_cast<LoopbackState*>(self->impl);
    // En solo, c'est ICI que la simulation avance : le tick du transport
    // pilote directement server_core. Le futur SocketTransport fera plutot
    // un poll() non-bloquant des sockets ici, et un process serveur dedie
    // separe appellera server_core::tick() de son cote.
    server_core::tick(st->core, dt);
}

void lb_destroy(Transport* self) {
    auto* st = static_cast<LoopbackState*>(self->impl);
    server_core::destroy(st->core);
    delete st;
    delete self;
}

constexpr TransportVTable kLoopbackVTable{
    lb_send_reliable,
    lb_poll_reliable,
    lb_send_unreliable,
    lb_poll_unreliable,
    lb_tick,
    lb_destroy,
};

} // namespace

Transport* loopback_transport_create(const std::string& worldSaveDir, uint32_t seed, const std::string& playerName) {
    auto* st = new LoopbackState();

    server_core::ServerConfig cfg;
    cfg.worldSaveDir = worldSaveDir;
    cfg.seed = seed;
    st->core = server_core::create(cfg);
    st->localClientId = server_core::connect_client(st->core, playerName);

    auto* transport = new Transport();
    transport->vtable = &kLoopbackVTable;
    transport->impl = st;
    return transport;
}
