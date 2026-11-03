// ASan bundles LeakSanitizer; leaks are not the bug class this fleet fuzzes for.
extern "C" int __lsan_is_turned_off() { return 1; }
