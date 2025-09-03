#include <cstdio>
#include <simu_lib/handler.h>

void gyro_data_handler_io(void* context, const MblMwData* data) {
    auto rotation = (MblMwCartesianFloat*)data->value;
    // return *rotation;
    printf("GYRO: (%.3f°/s, %.3f°/s, %.3f°/s)\n", 
           rotation->x, rotation->y, rotation->z);
}

void acc_data_handler_io(void* context, const MblMwData* data) {
    auto acceleration = (MblMwCartesianFloat*)data->value;
    // return *acceleration;
    printf("ACC: (%.3fg, %.3fg, %.3fg)\n", 
           acceleration->x, acceleration->y, acceleration->z);
}