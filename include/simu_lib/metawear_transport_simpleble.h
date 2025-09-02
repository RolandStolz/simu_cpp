#pragma once

#include <simpleble/SimpleBLE.h>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "metawear/platform/btle_connection.h"

class MetaWearBle {
  public:
    // Connect to first device advertising the MetaWear service.
    bool connect(int scan_ms = 2000);
    void disconnect();
    bool is_connected() const;
    MblMwBtleConnection* btle();

  private:
    struct Ctx {
        explicit Ctx(SimpleBLE::Peripheral periph);
        SimpleBLE::Peripheral p;
        std::mutex m;
        std::vector<std::pair<const void*, MblMwFnVoidVoidPtrInt>> dc;
    };

    static constexpr const char* kMetaWearService = "326A9000-85CB-9195-D9DD-464CFBBAE75A";

    static bool iequals(const std::string& a, const std::string& b);
    static std::string uuid128(uint64_t hi, uint64_t lo);
    
    void wire_disconnect();
    
    static void s_write(void* context, const void* caller, MblMwGattCharWriteType t,
                        const MblMwGattChar* ch, const uint8_t* v, uint8_t len);
    static void s_read(void* context, const void* caller, const MblMwGattChar* ch,
                       MblMwFnIntVoidPtrArray cb);
    static void s_notify(void* context, const void* caller, const MblMwGattChar* ch,
                         MblMwFnIntVoidPtrArray cb, MblMwFnVoidVoidPtrInt ready);
    static void s_on_dc(void* context, const void* caller, MblMwFnVoidVoidPtrInt handler);

    std::unique_ptr<Ctx> ctx_;
    MblMwBtleConnection conn_{};
};