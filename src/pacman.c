#include <stdbool.h>
#include <stddef.h>
#include "pacman.h"
#include "pac.h"

pacman_data_t pacman_data = {
  .next_pac = 0,
  .pac_checked_last = false,
};

void *get_next_pac() {
  PAC cur_pac = pacman_data.next_pac;

  if (cur_pac == MAX_PAC) {
    if (pacman_data.pac_checked_last) {
      return NULL;
    }
    pacman_data.pac_checked_last = true;
  } else {
    pacman_data.next_pac++;
  }

  return pac_encode(pacman_data.cur_addr, cur_pac);
}
