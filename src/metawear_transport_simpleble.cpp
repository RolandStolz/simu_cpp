#include "simu_lib/metawear_transport_simpleble.h"

#include <chrono>
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <thread>

MetaWearBle::Ctx::Ctx(SimpleBLE::Peripheral periph) : p(std::move(periph)) {
}

bool MetaWearBle::connect(int scan_ms) {
    if (!SimpleBLE::Adapter::bluetooth_enabled())
        return false;
    auto adapters = SimpleBLE::Adapter::get_adapters();
    if (adapters.empty())
        return false;
    auto adapter = adapters[0];

    adapter.scan_for(scan_ms);
    for (auto& p : adapter.scan_get_results()) {
        // Check if device advertises MetaWear service instead of checking name
        bool has_metawear_service = false;
        try {
            auto services = p.services();
            for (auto& service : services) {
                // Get the UUID from the Service object
                if (iequals(service.uuid(), kMetaWearService)) {
                    has_metawear_service = true;
                    std::printf("Found device: %s [%s]\n", p.identifier().c_str(),
                                p.address().c_str());
                    std::printf("with MetaWear service: %s\n", service.uuid().c_str());
                    break;
                }
            }
        } catch (...) {
            continue;
        }

        if (!has_metawear_service) {
            continue;
        }

        try {
            std::cout << "Found MetaWear device: " << p.identifier() << std::endl;
            p.connect();
            std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Increased delay
            if (p.is_connected()) {
                ctx_ = std::make_unique<Ctx>(p);
                wire_disconnect();
                conn_.context = ctx_.get();
                conn_.write_gatt_char = &s_write;
                conn_.read_gatt_char = &s_read;
                conn_.enable_notifications = &s_notify;
                conn_.on_disconnect = &s_on_dc;
                return true;
            }
        } catch (...) {
            // Try next device
        }
    }
    return false;
}

void MetaWearBle::disconnect() {
    if (ctx_) {
        try {
            if (ctx_->p.is_connected()) {
                std::cout << "Disconnecting BLE..." << std::endl;

                // Try disconnect with timeout using async approach
                std::atomic<bool> disconnect_done{false};
                std::thread disconnect_thread([&]() {
                    try {
                        ctx_->p.disconnect();
                        disconnect_done = true;
                    } catch (...) {
                        disconnect_done = true;
                    }
                });

                // Wait up to 3 seconds for disconnect
                auto start = std::chrono::steady_clock::now();
                while (!disconnect_done &&
                       std::chrono::steady_clock::now() - start < std::chrono::seconds(3)) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }

                if (!disconnect_done) {
                    std::cout << "Disconnect timeout, forcing cleanup..." << std::endl;
                    // Force terminate the disconnect thread if it's still running
                    disconnect_thread.detach(); // Let it finish on its own
                } else {
                    disconnect_thread.join();
                }

                std::cout << "BLE disconnect complete" << std::endl;
            }
        } catch (...) {
            std::cout << "Exception during disconnect, continuing cleanup..." << std::endl;
        }
        ctx_.reset();
    }
}

bool MetaWearBle::is_connected() const {
    return ctx_ && ctx_->p.is_connected();
}

MblMwBtleConnection* MetaWearBle::btle() {
    return &conn_;
}

bool MetaWearBle::iequals(const std::string& a, const std::string& b) {
    if (a.size() != b.size())
        return false;
    for (size_t i = 0; i < a.size(); ++i) {
        char ca = a[i], cb = b[i];
        if ('A' <= ca && ca <= 'Z')
            ca = ca - 'A' + 'a';
        if ('A' <= cb && cb <= 'Z')
            cb = cb - 'A' + 'a';
        if (ca != cb)
            return false;
    }
    return true;
}

std::string MetaWearBle::uuid128(uint64_t hi, uint64_t lo) {
    std::ostringstream oss;
    oss << std::hex << std::nouppercase << std::setfill('0') << std::setw(16) << hi << std::setw(16)
        << lo;
    auto s = oss.str(); // 32 hex chars
    return s.substr(0, 8) + "-" + s.substr(8, 4) + "-" + s.substr(12, 4) + "-" + s.substr(16, 4) +
           "-" + s.substr(20, 12);
}

void MetaWearBle::wire_disconnect() {
    if (!ctx_)
        return;
    // If your SimpleBLE version uses a different name, adjust accordingly.
    ctx_->p.set_callback_on_disconnected([c = ctx_.get()]() {
        std::vector<std::pair<const void*, MblMwFnVoidVoidPtrInt>> handlers;
        {
            std::lock_guard<std::mutex> lk(c->m);
            handlers = c->dc;
        }
        for (auto& h : handlers)
            h.second(h.first, 0);
    });
}
void MetaWearBle::s_write(void* context, const void* /*caller*/, MblMwGattCharWriteType t,
                          const MblMwGattChar* ch, const uint8_t* v, uint8_t len) {
    auto* c = reinterpret_cast<Ctx*>(context);
    if (!c) {
        return;
    }
    auto svc = uuid128(ch->service_uuid_high, ch->service_uuid_low);
    auto chr = uuid128(ch->uuid_high, ch->uuid_low);
    std::vector<uint8_t> data(v, v + len);

    try {
        // Use write_command for all writes to avoid timeout issues
        c->p.write_command(svc, chr, data);
    } catch (const std::exception& e) {
        std::cout << "WRITE ERROR: " << e.what() << std::endl;
    }
}

void MetaWearBle::s_read(void* context, const void* caller, const MblMwGattChar* ch,
                         MblMwFnIntVoidPtrArray cb) {
    auto* c = reinterpret_cast<Ctx*>(context);
    if (!c) {
        std::cout << "ERROR: read - null context" << std::endl;
        return;
    }
    auto svc = uuid128(ch->service_uuid_high, ch->service_uuid_low);
    auto chr = uuid128(ch->uuid_high, ch->uuid_low);


    try {
        auto data = c->p.read(svc, chr);
        cb(caller, data.data(), static_cast<uint8_t>(data.size()));
    } catch (const std::exception& e) {
        std::printf("READ ERROR: %s\n", e.what());
        cb(caller, nullptr, 0);
    }
}

void MetaWearBle::s_notify(void* context, const void* caller, const MblMwGattChar* ch,
                           MblMwFnIntVoidPtrArray cb, MblMwFnVoidVoidPtrInt ready) {
    auto* c = reinterpret_cast<Ctx*>(context);
    if (!c) {
        std::cout << "ERROR: notify - null context" << std::endl;
        ready(caller, -1);
        return;
    }
    auto svc = uuid128(ch->service_uuid_high, ch->service_uuid_low);
    auto chr = uuid128(ch->uuid_high, ch->uuid_low);

    try {
        c->p.notify(svc, chr, [caller, cb](SimpleBLE::ByteArray bytes) {
            if (!bytes.empty()) {
                cb(caller, bytes.data(), static_cast<uint8_t>(bytes.size()));
            } else {
                std::printf("WARNING: Empty notification received\n");
            }
        });
        ready(caller, 0);
    } catch (const std::exception& e) {
        std::cout << "NOTIFY ERROR: " << e.what() << std::endl;
        ready(caller, -1);
    }
}

void MetaWearBle::s_on_dc(void* context, const void* caller, MblMwFnVoidVoidPtrInt handler) {
    auto* c = reinterpret_cast<Ctx*>(context);
    if (!c)
        return;
    std::lock_guard<std::mutex> lk(c->m);
    c->dc.emplace_back(caller, handler);
}