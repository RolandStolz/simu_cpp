#include <metawear/core/datasignal.h>
#include <metawear/core/metawearboard.h>
#include <metawear/core/status.h>
#include <metawear/core/types.h>
#include <metawear/sensor/accelerometer.h>
#include <metawear/sensor/gyro_bosch.h>
#include <simu_lib/handler.h>
#include <simu_lib/metawear_transport_simpleble.h>

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <iostream>
#include <thread>

std::atomic<bool> init_complete{false};

int main() {
    MetaWearBle transport;

    std::string peripheral_id = "37b0d50a-0a93-b6b3-8670-7bb95122fc86";

    std::printf("Trying to connect to device with ID: %s\n", peripheral_id.c_str());
    if (!transport.connect_by_id(peripheral_id, 5000)) { // 5 second scan
        std::cout << "Failed to connect to MetaWear device" << std::endl;
        return -1;
    }

    std::cout << "Connected! Initializing board..." << std::endl;

    // Create board instance
    auto board = mbl_mw_metawearboard_create(transport.btle());

    // Fix: Use proper context handling
    mbl_mw_metawearboard_initialize(
        board, nullptr, [](void* context, MblMwMetaWearBoard* board, int32_t status) {
            if (status == MBL_MW_STATUS_OK) {
                std::cout << "Board initialization successful!" << std::endl;
            } else {
                std::cout << "Board initialization failed: " << status << std::endl;
            }
            init_complete = true; // Use atomic variable directly
        });

    // Wait for initialization to complete
    while (!init_complete && transport.is_connected()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // if (!init_complete) {
    //     std::cout << "Initialization timeout" << std::endl;
    //     mbl_mw_metawearboard_free(board);
    //     return -1;
    // }
    auto model_name = mbl_mw_metawearboard_get_model_name(board);
    std::printf("Board model: %s\n", model_name);

    // Configure accelerometer
    mbl_mw_acc_set_odr(board, 50.0f);  // 50Hz sampling rate
    mbl_mw_acc_set_range(board, 2.0f); // ±2g range
    mbl_mw_acc_write_acceleration_config(board);

    // Configure gyroscope
    mbl_mw_gyro_bmi160_set_odr(board, MBL_MW_GYRO_BOSCH_ODR_25Hz);
    mbl_mw_gyro_bmi160_set_range(board, MBL_MW_GYRO_BOSCH_RANGE_125dps);
    mbl_mw_gyro_bmi160_write_config(board);

    // Subscribe to accelerometer data
    auto acc_signal = mbl_mw_acc_get_acceleration_data_signal(board);
    mbl_mw_datasignal_subscribe(acc_signal, nullptr, acc_data_handler_io);

    // Subscribe to gyroscope data
    auto gyro_signal = mbl_mw_gyro_bmi160_get_rotation_data_signal(board);
    mbl_mw_datasignal_subscribe(gyro_signal, nullptr, gyro_data_handler_io);

    // Start streaming
    std::cout << "Starting sensor streaming... (Press Ctrl+C to stop)" << std::endl;

    mbl_mw_acc_enable_acceleration_sampling(board);
    mbl_mw_acc_start(board);

    mbl_mw_gyro_bmi160_enable_rotation_sampling(board);
    mbl_mw_gyro_bmi160_start(board);

    // Keep streaming until interrupted
    while (transport.is_connected()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Clean shutdown
    std::cout << "Stopping sensors..." << std::endl;

    // Stop sensors first (this sends the stop commands you see)
    mbl_mw_acc_stop(board);
    mbl_mw_acc_disable_acceleration_sampling(board);
    mbl_mw_gyro_bmi160_stop(board);
    mbl_mw_gyro_bmi160_disable_rotation_sampling(board);

    // Give time for stop commands to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    std::cout << "Cleaning up..." << std::endl;

    // Skip unsubscribe to avoid hanging - just free the board
    mbl_mw_datasignal_unsubscribe(acc_signal);
    mbl_mw_datasignal_unsubscribe(gyro_signal);

    // Free board resources
    mbl_mw_metawearboard_free(board);

    std::cout << "Disconnecting..." << std::endl;
    transport.disconnect();

    std::cout << "Done!" << std::endl;
    return 0;
}