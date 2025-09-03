#include <cstdio>
#include <simu_lib/utils.h>

int main() {
    auto result =
        iequals("37b0d50a-0a93-b6b3-8670-7bb95122fc86", "37B0D50A-0A93-B6B3-8670-7BB95122FC86");
    std::printf("Result: %d\n", result);
}