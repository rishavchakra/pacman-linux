#include <linux/string.h>
// Having a struct like this is one way to do it
// another would be to overwrite the function return value
// but then you may need to consider stack canaries, etc.
// on second note, that might just be in the LR register
typedef struct {
  char buf[8];
  void (*fp)(void);
} obj_t;

// Some arbitrary salt number, doesn't matter
static unsigned long long salt = 0x1122334455667788;

static void vulnerable_syscall(const char *str, char cond) {
  obj_t obj;
  memcpy(obj.buf, str, strlen(str));

  // Start of the speculative window
  if (cond == 'y') {
    // authorize the function pointer
    asm("autia %[ptr], %[salt]" : [ptr] "+r"(obj.fp) : [salt] "r"(salt));
    // call the function pointer
    obj.fp();
  } else {
    // no-op, effectively
    return;
  }
}
