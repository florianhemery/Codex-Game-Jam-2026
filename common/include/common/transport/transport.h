#pragma once

#include "common/messages/reliable_messages.h"
#include "common/messages/unreliable_messages.h"

// Frontière unique entre le client et server_core. Le client n'appelle jamais
// server_core directement : il ne connait que ce vtable + les structs de message.
// LoopbackTransport (aujourd'hui) et SocketTransport (plus tard) implementent
// la meme interface, donc le code client ne changera pas quand le vrai reseau arrivera.

struct Transport;

struct TransportVTable {
    void (*send_reliable)(Transport* self, const common::messages::ReliableMessage& msg);
    bool (*poll_reliable)(Transport* self, common::messages::ReliableMessage& out);

    void (*send_unreliable)(Transport* self, const common::messages::UnreliableMessage& msg);
    bool (*poll_unreliable)(Transport* self, common::messages::UnreliableMessage& out);

    void (*tick)(Transport* self, float dt);
    void (*destroy)(Transport* self);
};

struct Transport {
    const TransportVTable* vtable;
    void* impl;
};

inline void transport_send_reliable(Transport* t, const common::messages::ReliableMessage& msg) {
    t->vtable->send_reliable(t, msg);
}

inline bool transport_poll_reliable(Transport* t, common::messages::ReliableMessage& out) {
    return t->vtable->poll_reliable(t, out);
}

inline void transport_send_unreliable(Transport* t, const common::messages::UnreliableMessage& msg) {
    t->vtable->send_unreliable(t, msg);
}

inline bool transport_poll_unreliable(Transport* t, common::messages::UnreliableMessage& out) {
    return t->vtable->poll_unreliable(t, out);
}

inline void transport_tick(Transport* t, float dt) {
    t->vtable->tick(t, dt);
}

inline void transport_destroy(Transport* t) {
    t->vtable->destroy(t);
}
