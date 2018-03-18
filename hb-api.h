#pragma once

#include "Ctu.h"

enum {
    END_OF_LIST,
    MAIN_THREAD_HANDLE,
    NEXT_LOAD_PATH,
    OVERRIDE_HEAP,
    OVERRIDE_SERVICE,
    ARGV,
    SYSCALL_AVAILABLE_HINT,
    APPLET_TYPE,
    APPLET_WORKAROUND,
    STDIO_SOCKETS,
    PROCESS_HANDLE,
    LAST_LOAD_RESULT,
    LOG = 51
} LoaderKey;

enum {
    APPLET_TYPE_APPLICATION = 0,
    APPLET_TYPE_SYSTEM_APPLET = 1,
    APPLET_TYPE_LIBRARY_APPLET = 2,
    APPLET_TYPE_OVERLAY_APPLET = 3,
    APPLET_TYPE_SYSTEM_APPLICATION = 4
} AppletType;

struct LoaderConfigEntry {
  uint32_t key;
  uint32_t flags;
  uint64_t value[2];
};
