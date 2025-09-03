#pragma once

#include <metawear/core/cpp/metawearboard_def.h>
#include <metawear/core/types.h>
#include <metawear/core/metawearboard_fwd.h>
#include <metawear/core/datasignal.h>
#include <metawear/core/metawearboard.h>
#include <metawear/core/status.h>
#include <metawear/core/types.h>
#include <iostream>
#include <future>

std::future<void> async_initialize_board(MblMwMetaWearBoard* board);