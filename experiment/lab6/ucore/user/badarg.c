#include <io.h>
#include <ulib.h>
#include <umain.h>

int
main(void) {
  int pid, exit_code, i;
  if ((pid = fork()) == 0) {
    cprintf("fork ok.\n");
    for (i = 0; i < 10; i++) {
      yield();
    }
    exit(0xbeaf);
  }
  assert(pid > 0);
  assert(waitpid(-1, NULL) != 0);
  assert(waitpid(pid, (void *)0xC0000000) != 0);
  assert(waitpid(pid, &exit_code) == 0 && exit_code == 0xbeaf);
  cprintf("badarg pass.\n");
  return 0;
}
