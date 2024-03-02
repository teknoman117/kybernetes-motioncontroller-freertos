add_executable(kybernetes-motioncontroller
    main.c
    uart.c
    i2c.c
    console.c
)

target_link_libraries(kybernetes-motioncontroller PRIVATE freertos)

avr_add_extra_outputs(kybernetes-motioncontroller)