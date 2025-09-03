#include <simu_lib/metawear_async.h>
#include <memory>

std::future<void> async_initialize_board(MblMwMetaWearBoard* board) {
    auto promise = std::make_shared<std::promise<void>>();
    auto future = promise->get_future();

    // Pass the shared_ptr itself as context
    mbl_mw_metawearboard_initialize(
        board,
        new std::shared_ptr<std::promise<void>>(promise), // allocate on heap
        [](void* context, MblMwMetaWearBoard* board, int32_t status) {
            // Recover the shared_ptr
            auto holder =
                static_cast<std::shared_ptr<std::promise<void>>*>(context);
            auto p = *holder; // copy shared_ptr
            delete holder;    // free heap allocation

            try {
                if (status == MBL_MW_STATUS_OK) {
                    p->set_value();
                } else {
                    p->set_exception(std::make_exception_ptr(
                        std::runtime_error("Board init failed: " +
                                           std::to_string(status))));
                }
            } catch (...) {
                // ignore double set
            }
        });

    return future;
}