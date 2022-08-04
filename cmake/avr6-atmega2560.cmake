set(F_CPU 16000000UL)
set(MCU atmega2560)
set(LINKER_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/../arch/avr/avrtos-avr6.ld)
set(QEMU_MCU mega2560)
set(PROG_TYPE wiring)
set(PROG_PARTNO m2560)

include(${CMAKE_CURRENT_LIST_DIR}/avr.cmake)