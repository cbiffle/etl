depth := ../..
products := armv7m exception_table manual_crt0

armv7m[type] := library
armv7m[cc_flags] := -std=gnu++0x
armv7m[cc_files] := \
  nvic.cc \
  scb.cc
armv7m[libs] := \
  etl/common:common
armv7m[using_l_flags] := $(shell pwd)/peripherals.ld

# This is broken into a separate target so that we can link it with
# whole-archive.
exception_table[type] := library

exception_table[cc_files] := exception_table.cc
exception_table[cc_flags] := -std=gnu++0x
exception_table[libs] := \
  etl/armv7m:armv7m \
  etl/common:common


manual_crt0[type] := library
manual_crt0[cc_flags] := -std=gnu++0x
manual_crt0[cc_files] := \
  crt0.cc
manual_crt0[libs] := \
  etl/armv7m:armv7m \
  etl/common:common

include $(depth)/build/Makefile.rules
