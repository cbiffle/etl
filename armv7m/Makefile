depth := ../..
products := armv7m exception_table

armv7m[type] := library
armv7m[libs] := \
  etl/common:common

# This is broken into a separate target so that we can link it with
# whole-archive.
exception_table[type] := library

exception_table[cc_files] := exception_table.cc
exception_table[cc_flags] := -std=gnu++0x
exception_table[libs] := \
  etl/armv7m:armv7m \
  etl/common:common

include $(depth)/build/Makefile.rules
