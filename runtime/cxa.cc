extern "C" void __cxa_pure_virtual();

void __cxa_pure_virtual() {
  // TODO(cbiffle): still need a general assert mechanism.
  while (1);
}
