depth := ..
products := etl subdirs
subdirectories := *

etl[type] := library

subdirs[type] := subdirectories

include $(depth)/build/Makefile.rules
