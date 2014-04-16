depth    := ../..

products := stm32f4xx interrupt_table

stm32f4xx[type] := library

stm32f4xx[cc_files] := \
  gpio.cc \
  interrupts.cc \
  rcc.cc

stm32f4xx[cc_flags] := -std=gnu++0x

stm32f4xx[libs] := \
  etl/armv7m:armv7m \
  etl:etl

stm32f4xx[using_l_flags] := $(shell pwd)/peripherals.ld


interrupt_table[type] := library

interrupt_table[cc_files] := \
  interrupt_table.cc

interrupt_table[cc_flags] := -std=gnu++0x

interrupt_table[libs] := \
  etl:etl \
  etl/stm32f4xx:stm32f4xx

include $(depth)/build/Makefile.rules
