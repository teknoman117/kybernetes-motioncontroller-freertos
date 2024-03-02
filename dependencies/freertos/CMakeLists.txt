add_library(freertos STATIC
    kernel/portable/ThirdParty/GCC/ATmega/port.c
    kernel/portable/MemMang/heap_3.c
    kernel/croutine.c
    kernel/event_groups.c
    kernel/list.c
    kernel/queue.c
    kernel/stream_buffer.c
    kernel/tasks.c
    kernel/timers.c
)

target_include_directories(freertos
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/kernel/portable/ThirdParty/GCC/ATmega>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/kernel/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

target_compile_options(freertos PRIVATE $<$<C_COMPILER_ID:GNU>:-Wno-cpp>)