// Add debugging to see if transport is working
#include <future>
#include <iomanip>
#include <iostream>
#include <thread>

#include "metawear/core/cpp/datasignal_private.h"
#include "metawear/core/data.h"
#include "metawear/core/datasignal.h"
#include "metawear/core/metawearboard.h"
#include "metawear/core/status.h"
#include "metawear/core/types.h"
#include "metawear/sensor/accelerometer.h"

#include <simu_lib/metawear_transport_simpleble.h>

using namespace std::chrono_literals;

static void on_accel(const MblMwData* data, void* /*ctx*/) {
    std::cout << "Callback called! ";

    if (!data) {
        std::cout << "data is NULL\n";
        return;
    }

    if (!data->value) {
        std::cout << "data->value is NULL\n";
        return;
    }

    const auto* v = reinterpret_cast<const MblMwCartesianFloat*>(data->value);
    std::cout << "Accel data: " << std::fixed << std::setprecision(3) << v->x << ", " << v->y
              << ", " << v->z << " g\n";
}

static void init_cb(void* ctx, MblMwMetaWearBoard* /*board*/, int status) {
    auto* prom = static_cast<std::promise<int>*>(ctx);
    prom->set_value(status);
}

int main() {
    MetaWearBle ble;
    if (!ble.connect(2000)) {
        std::cerr << "MetaWear not found/connect failed.\n";
        return 1;
    }

    std::cout << "Connected, waiting before initialization...\n";
    std::this_thread::sleep_for(2s);

    // Debug: Check if your transport layer is properly set up
    auto* btle_conn = ble.btle();
    std::cout << "BLE transport created: " << (btle_conn ? "YES" : "NO") << std::endl;

    MblMwMetaWearBoard* board = mbl_mw_metawearboard_create(btle_conn);
    std::cout << "Board created: " << (board ? "YES" : "NO") << std::endl;

    std::promise<int> prom;
    auto fut = prom.get_future();

    std::cout << "Starting initialization...\n";
    mbl_mw_metawearboard_initialize(board, &prom, init_cb);

    if (fut.wait_for(10s) == std::future_status::timeout) {
        std::cerr << "Initialization timed out after 10 seconds\n";
        mbl_mw_metawearboard_free(board);
        ble.disconnect();
        return 1;
    }

    auto a = mbl_mw_metawearboard_lookup_module(board, MBL_MW_MODULE_ACCELEROMETER);
    std::cout << "Accelerometer module: " << (a ? "FOUND" : "NOT FOUND") << std::endl;

    int init_status = fut.get();
    if (init_status != MBL_MW_STATUS_OK) {
        std::cerr << "Init failed with status: " << init_status << std::endl;
        mbl_mw_metawearboard_free(board);
        ble.disconnect();
        return 1;
    }

    std::cout << "Board initialized successfully!\n";

    // Add more debugging
    std::cout << "Configuring accelerometer...\n";
    mbl_mw_acc_set_odr(board, 100.f);
    mbl_mw_acc_set_range(board, 8.f);
    mbl_mw_acc_write_acceleration_config(board);

    std::this_thread::sleep_for(1s);

    const MblMwDataSignal* acc_const = mbl_mw_acc_get_acceleration_data_signal(board);
    if (!acc_const) {
        std::cerr << "No accel signal.\n";
        mbl_mw_metawearboard_free(board);
        ble.disconnect();
        return 1;
    }
    std::cout << "Got accelerometer signal: " << acc_const << std::endl;

    MblMwDataSignal* acc = const_cast<MblMwDataSignal*>(acc_const);

    std::cout << "Subscribing to accelerometer data...\n";
    mbl_mw_datasignal_subscribe(acc, nullptr, (MblMwFnData)on_accel);

    std::cout << "Enabling acceleration sampling...\n";
    mbl_mw_acc_enable_acceleration_sampling(board);

    std::cout << "Starting accelerometer...\n";
    mbl_mw_acc_start(board);

    std::cout << "Streaming for 8s...\n";
    auto start_time = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start_time < 8s) {
        std::this_thread::sleep_for(500ms);
    }

    mbl_mw_acc_stop(board);
    mbl_mw_acc_disable_acceleration_sampling(board);
    mbl_mw_datasignal_unsubscribe(acc);

    mbl_mw_metawearboard_free(board);
    ble.disconnect();
    return 0;
}