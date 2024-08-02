#include <asm/io.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rishav C.");

////////////////////////////////////////////////////////////////
/// Function and global variable declarations
////////////////////////////////////////////////////////////////

static struct proc_dir_entry *proc_file;

// The buffer used to store the characters read from procfile
static char proc_buffer[128];

// The size of the buffer
static unsigned long proc_buffer_size = 0;

static ssize_t on_proc_write(struct file *file, const char __user *buffer,
                             size_t buffer_len, loff_t *offset);

static ssize_t on_proc_read(struct file *file, char __user *buffer,
                            size_t buffer_len, loff_t *offset);

static const struct proc_ops proc_file_ops = {
    .proc_read = on_proc_read,
    .proc_write = on_proc_write,
};

typedef enum {
  OP_VIRT_TO_PHYS,
  OP_DATA_GADGET,
  OP_AUTH_GADGET,
  OP_GET_TARGET_PADDR,
  OP_GET_TARGET_VADDR,
  OP_ERR,
} op_type_e;

static op_type_e cur_op;

static void *cur_vaddr;

static void *cur_pac_ptr;

static int cur_pac_rc;

// Instruction PACMAN

// This is the function we're going to try to point to
// simulating arbitrary code execution, i guess
static void target_function(void);

// Data PACMAN
// This is a known...
// what am I talking about
static char *safe = "blahblahblah";

// The pointer that is not signed that the attacker attempts to forge a
// signature for The attacker knows only the virtual address, not the contents
static char *secret = "secret string!";

// Some arbitrary salt number, doesn't matter
static unsigned long long salt = 0x1122334455667788;

// This is the function we're going to try to attack!
void auth_syscall(const char *str, char cond);

// Example vulnerable struct
typedef struct {
  char buf[8];
  void (*fp)(void);
} obj_t;

////////////////////////////////////////////////////////////////
/// Module init and cleanup
////////////////////////////////////////////////////////////////

// init_module
// called when the kernel module is started (with sudo insmod)
int init_module() {
  pr_info("PACMAN: starting kernel module\n");

  // Proc file permissions: anybody can read or write from/to proc file
  proc_file = proc_create("PACMAN-kmod", 0666, NULL, &proc_file_ops);
  if (proc_file == NULL) {
    proc_remove(proc_file);
    pr_alert("PACMAN: Could not initialize /proc/PACMAN-kmod\n");
    return -ENOMEM;
  } else {
    pr_info("PACMAN: Initialized /proc/PACMAN-kmod\n");
  }

  return 0;
}

// cleanup_module
// called when the kernel module is stopped (with sudo rmmod)
void cleanup_module() {
  pr_info("PACMAN: stopping kernel module\n");
  proc_remove(proc_file);
}

////////////////////////////////////////////////////////////////
/// Function definitions
////////////////////////////////////////////////////////////////

// proc_write
// called when the user tries to write to the procfile, which the kernel reads
// from Confusingly named!
static ssize_t on_proc_write(struct file *file, const char __user *buffer,
                             size_t buffer_len, loff_t *offset) {
  pr_info("PACMAN: write handler\n");

  proc_buffer_size = buffer_len;
  if (proc_buffer_size > 128) {
    proc_buffer_size = 128;
  }

  if (copy_from_user(proc_buffer, buffer, proc_buffer_size)) {
    cur_op = OP_ERR;
    return -EFAULT;
  }

  proc_buffer[proc_buffer_size & 1023] = '\0';
  *offset += proc_buffer_size;
  pr_info("PACMAN: read %s\n", proc_buffer);

  if (proc_buffer[0] == 'v') {
    // Virt -> phys translation request
    size_t read_vaddr;
    int rc = sscanf(proc_buffer + 1, "%zu", &read_vaddr);
    if (rc == 0) {
      cur_op = OP_ERR;
      pr_info("PACMAN: pointer parsing failed with vaddr %zu\n", read_vaddr);
      return -EFAULT;
    }
    cur_op = OP_VIRT_TO_PHYS;
    cur_vaddr = (void *)read_vaddr;
  } else if (proc_buffer[0] == 'd') {
    // Data gadget request
    // This one is not fully implemented yet...

    size_t read_ptr;
    char *read_str;
    int rc = sscanf(proc_buffer + 2, "%s", read_str);
    if (rc == 0) {
      cur_pac_rc = -1;
      // idk what this error means lol idc
      return -EFAULT;
    }
    cur_pac_ptr = (void *)read_ptr;

    // This is the pacman attack!
    char cond = proc_buffer[1];
    // This is instruction oriented. Think: What does the data version look
    // like? vulnerable_syscall(read_str, cond);

    // // Start of the speculative window
    // if (cond == 'y') {
    //   // Do the pointer authentication here!
    //   // Print statements will not print when run speculatively (buffered)
    //   // (just make sure that they're at the end so they don't slow anything
    //   // down)
    //   pr_info(
    //       "PACMAN: data gadget reached PAC authorizing condition\nPAC: %p\n",
    //       cur_pac_ptr);
    //   cur_pac_rc = 0;
    // } else {
    //   // Do nothing
    //   pr_info("PACMAN: data gadget reached base condition\n");
    //   cur_pac_rc = 1;
    // }

    cur_op = OP_DATA_GADGET;
  } else if (proc_buffer[0] == 'a') {
    char *read_str;
    int rc = sscanf(proc_buffer + 2, "%s", read_str);
    if (rc == 0) {
      cur_pac_rc = -1;
      pr_info("PACMAN: inst gadget failed reading string\n");
      return -EFAULT;
    }
    char cond = proc_buffer[1];

    pr_info("PACMAN:\nTaking PAC path: %c\nString input: %s\n", cond, read_str);

    auth_syscall(read_str, cond);

    cur_op = OP_AUTH_GADGET;
  } else if (proc_buffer[0] == 't' && proc_buffer[1] == 'p') {
    // Target function paddr request
    // No logic to be done here, we return the address upon read
    cur_op = OP_GET_TARGET_PADDR;
  } else if (proc_buffer[0] == 't' && proc_buffer[1] == 'v') {
    cur_op = OP_GET_TARGET_VADDR;
  } else {
    cur_op = OP_ERR;
  }

  return proc_buffer_size;
}

// proc_read
// called when the user tries to read from the procfile, which the kernel writes
// to Confusingly named!
static ssize_t on_proc_read(struct file *file, char __user *buffer,
                            size_t buffer_len, loff_t *offset) {
  pr_info("PACMAN: read handler\n");
  ssize_t len;
  if (cur_op == OP_ERR) {
    pr_info("PACMAN: previous request failed\n");
    return 0;
  }

  char s[64];
  if (cur_op == OP_VIRT_TO_PHYS) {
    phys_addr_t paddr = virt_to_phys(cur_vaddr);
    sprintf(s, "%p", (void *)paddr);
    len = strlen(s);
  } else if (cur_op == OP_DATA_GADGET) {
    sprintf(s, "%d", cur_pac_rc);
    len = strlen(s);
  } else if (cur_op == OP_AUTH_GADGET) {
    sprintf(s, "%d", cur_pac_rc);
    len = strlen(s);
  } else if (cur_op == OP_GET_TARGET_PADDR) {
    // I believe this will properly get the pointer to the target fn
    phys_addr_t paddr = virt_to_phys(target_function);
    sprintf(s, "%p", (void *)paddr);
    len = strlen(s);
  } else if (cur_op == OP_GET_TARGET_VADDR) {
    // TODO: Make sure this function pointer is signed
    sprintf(s, "%p", target_function);
    len = strlen(s);
  }

  // const char *s = "Hello from PACMAN!\n";
  // ssize_t len = 19;

  if (*offset >= len || copy_to_user(buffer, s, len)) {
    pr_info("PACMAN: procfile read finished\n");
    return 0;
  }

  pr_info("PACMAN: procfile read %s\n", file->f_path.dentry->d_name.name);
  *offset += len;

  return len;
}

static void target_function(void) {
  pr_info("PACMAN: You hit the target function!\nPACMAN was successful!\n");
}
void auth_syscall(const char *str, char cond) {
  obj_t obj;
  memcpy(obj.buf, str, strlen(str));

  void *temp;

  // Start of the speculative window, if 'n' is taken
  if (cond == 'y') {
    // authorize the function pointer
    asm("autia %[ptr], %[salt]" : [ptr] "+r"(obj.fp) : [salt] "r"(salt));

    // Make sure the pointer is cached
    temp = obj.fp;
  } else {
    // no-op, effectively
    return;
  }
}
