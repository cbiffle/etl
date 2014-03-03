extern "C" int __aeabi_atexit(void *, void (*)(void *), void *);

int __aeabi_atexit(void *, void(*)(void *), void *) {
  return 1;
}

void *__dso_handle;
