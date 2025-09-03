#pragma once

#include <metawear/core/data.h>
#include <metawear/core/types.h>

void acc_data_handler_io(void* context, const MblMwData* data);
void gyro_data_handler_io(void* context, const MblMwData* data);