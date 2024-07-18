#include <asm/io.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
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
  OP_GET_TARGET_PADDR,
  OP_GET_TARGET_VADDR,
  OP_ERR,
} op_type_e;

static op_type_e cur_op;

static void *cur_vaddr;

static void *cur_pac_ptr;

static int cur_pac_rc;

// This is the function we're going to try to attack!
static void target_function(void);

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
    // This one needs to be as FAST as possible
    // Edit: only the contents of the if (cond == 'y') need to be blazingly fast

    size_t read_ptr;
    int rc = sscanf(proc_buffer + 2, "%zu", &read_ptr);
    if (rc == 0) {
      cur_pac_rc = -1;
      // idk what this error means lol idc
      return -EFAULT;
    }
    cur_pac_ptr = (void *)read_ptr;

    // This is the pacman attack!
    char cond = proc_buffer[1];
    // Start of the speculative window
    if (cond == 'y') {
      // Do the pointer authentication here!
      // Print statements will not print when run speculatively (buffered)
      // (just make sure that they're at the end so they don't slow anything
      // down)
      pr_info(
          "PACMAN: data gadget reached PAC authorizing condition\nPAC: %p\n",
          cur_pac_ptr);
      cur_pac_rc = 0;
    } else {
      // Do nothing
      pr_info("PACMAN: data gadget reached base condition\n");
      cur_pac_rc = 1;
    }

    cur_op = OP_DATA_GADGET;
  } else if (proc_buffer[0] == 't' && proc_buffer[1] == 'p') {
    // Target function paddr request
    // No logic to be done, we return the address upon read
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
  } else if (cur_op == OP_GET_TARGET_PADDR) {
    // I believe this will properly get the pointer to the target fn
    phys_addr_t paddr = virt_to_phys(target_function);
    sprintf(s, "%p", (void *)paddr);
    len = strlen(s);
  } else if (cur_op == OP_GET_TARGET_VADDR) {
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
  pr_info("You hit the target function!\nPACMAN was successful!\n");
}
