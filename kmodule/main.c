#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rishav C.");

////////////////////////////////////////////////////////////////
/// Function and global variable declarations
////////////////////////////////////////////////////////////////

static struct proc_dir_entry *proc_file;

// The buffer used to store the characters read from procfile
static char proc_buffer[1024];

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
  if (proc_buffer_size > 1024) {
    proc_buffer_size = 1024;
  }

  if (copy_from_user(proc_buffer, buffer, proc_buffer_size)) {
    return -EFAULT;
  }

  proc_buffer[proc_buffer_size & 1023] = '\0';
  *offset += proc_buffer_size;
  pr_info("%s\n", proc_buffer);

  return proc_buffer_size;
}

// proc_read
// called when the user tries to read from the procfile, which the kernel writes
// to Confusingly named!
static ssize_t on_proc_read(struct file *file, char __user *buffer,
                            size_t buffer_len, loff_t *offset) {
  const char *s = "Hello from PACMAN!\n";
  ssize_t len = 19;

  if (*offset >= len || copy_to_user(buffer, s, len)) {
    pr_info("PACMAN: procfile read failed\n");
    len = 0;
  } else {
    pr_info("PACMAN: procfile read %s\n", file->f_path.dentry->d_name.name);
    *offset += len;
  }

  return len;
}
