#include "metawear/core/status.h"
#include "metawear/core/types.h"
#include "simu_lib/metawear_transport_simpleble.h"
#include "metawear/core/metawearboard.h"
#include "metawear/core/datasignal.h"
#include "metawear/sensor/accelerometer.h"
#include "metawear/sensor/gyro_bosch.h"

#include <chrono>
#include <iostream>
#include <thread>
#include <csignal>
#include <atomic>

// Global flag to control streaming
std::atomic<bool> keep_streaming{true};
std::atomic<bool> init_complete{false};

void signal_handler(int signal) {
    std::cout << "\nStopping stream..." << std::endl;
    keep_streaming = false;
}

// Callback for acceleration data
void acc_data_handler(void* context, const MblMwData* data) {
    auto acceleration = (MblMwCartesianFloat*)data->value;
    printf("ACC: (%.3fg, %.3fg, %.3fg)\n", 
           acceleration->x, acceleration->y, acceleration->z);
}

// Callback for gyro data
void gyro_data_handler(void* context, const MblMwData* data) {
    auto rotation = (MblMwCartesianFloat*)data->value;
    printf("GYRO: (%.3f°/s, %.3f°/s, %.3f°/s)\n", 
           rotation->x, rotation->y, rotation->z);
}

int main() {
    // Set up signal handler for clean exit
    std::signal(SIGINT, signal_handler);
    
    MetaWearBle transport;
    
    std::cout << "Scanning for MetaWear devices..." << std::endl;
    if (!transport.connect(5000)) {  // 5 second scan
        std::cout << "Failed to connect to MetaWear device" << std::endl;
        return -1;
    }
    
    std::cout << "Connected! Initializing board..." << std::endl;
    
    // Create board instance
    auto board = mbl_mw_metawearboard_create(transport.btle());
    
    // Fix: Use proper context handling
    mbl_mw_metawearboard_initialize(board, nullptr, 
        [](void* context, MblMwMetaWearBoard* board, int32_t status) {
            if (status == MBL_MW_STATUS_OK) {
                std::cout << "Board initialization successful!" << std::endl;
            } else {
                std::cout << "Board initialization failed: " << status << std::endl;
            }
            init_complete = true;  // Use atomic variable directly
        });
    
    // Wait for initialization to complete
    while (!init_complete && transport.is_connected()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    if (!init_complete) {
        std::cout << "Initialization timeout" << std::endl;
        mbl_mw_metawearboard_free(board);
        return -1;
    }
    
    // Configure accelerometer
    mbl_mw_acc_set_odr(board, 50.0f);  // 50Hz sampling rate
    mbl_mw_acc_set_range(board, 2.0f); // ±2g range
    mbl_mw_acc_write_acceleration_config(board);
    
    // Configure gyroscope
    // mbl_mw_gyro_bmi160_set_odr(board, MBL_MW_GYRO_BMI160_ODR_50HZ);
    // mbl_mw_gyro_bmi160_set_range(board, MBL_MW_GYRO_BMI160_FSR_250DPS);
    mbl_mw_gyro_bmi160_set_odr(board, MBL_MW_GYRO_BOSCH_ODR_25Hz);
    mbl_mw_gyro_bmi160_set_range(board, MBL_MW_GYRO_BOSCH_RANGE_125dps);
    mbl_mw_gyro_bmi160_write_config(board);
    
    // Subscribe to accelerometer data
    auto acc_signal = mbl_mw_acc_get_acceleration_data_signal(board);
    mbl_mw_datasignal_subscribe(acc_signal, nullptr, acc_data_handler);
    
    // Subscribe to gyroscope data
    auto gyro_signal = mbl_mw_gyro_bmi160_get_rotation_data_signal(board);
    mbl_mw_datasignal_subscribe(gyro_signal, nullptr, gyro_data_handler);
    
    // Start streaming
    std::cout << "Starting sensor streaming... (Press Ctrl+C to stop)" << std::endl;
    
    mbl_mw_acc_enable_acceleration_sampling(board);
    mbl_mw_acc_start(board);
    
    mbl_mw_gyro_bmi160_enable_rotation_sampling(board);
    mbl_mw_gyro_bmi160_start(board);
    
    // Keep streaming until interrupted
    while (keep_streaming && transport.is_connected()) {
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