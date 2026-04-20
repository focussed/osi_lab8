// Memory driver - reads temperature from Arduino over USB serial
// Based on Lab 5 solution. Students implement get_temperature_from_arduino()
// ATU Sligo - Operating Systems Interfacing Lab 8

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/fcntl.h>      // For O_RDWR, O_NOCTTY
#include <linux/file.h>       // For filp_open, filp_close
#include <linux/delay.h>      // For msleep()
#include <linux/string.h>     // For strstr(), memset()

#define DEVICE_NAME "memory"
#define CLASS_NAME "memory_class"
#define SERIAL_PORT "/dev/ttyACM0"

static int major_num;
static struct class *memory_class = NULL;
static struct device *memory_device = NULL;
static struct cdev memory_cdev;

// Buffer to store temperature as string
static char temp_buffer[16];
static int temp_len = 0;

// TODO: Students implement this function
static int get_temperature_from_arduino(char *buffer, size_t buf_len)
{
    struct file *serial_fp;
    char cmd = 'T';
    char response[64];
    ssize_t bytes_written, bytes_read;
    char *temp_start;
    int i;
    
    // STEP 1: Open the serial port device file
    serial_fp = filp_open(SERIAL_PORT, O_RDWR | O_NOCTTY, 0);
    if (IS_ERR(serial_fp)) {
        printk(KERN_ERR "memory: Failed to open %s, error=%ld\n", 
               SERIAL_PORT, PTR_ERR(serial_fp));
        return PTR_ERR(serial_fp);
    }
    
    // STEP 2: Send the 'T' command
    bytes_written = kernel_write(serial_fp, &cmd, 1, &serial_fp->f_pos);
    if (bytes_written != 1) {
        printk(KERN_ERR "memory: Failed to write to Arduino, wrote %zd bytes\n", 
               bytes_written);
        filp_close(serial_fp, NULL);
        return -EIO;
    }
    
    // STEP 3: Wait for Arduino to respond
    msleep(100);
    
    // STEP 4: Read the response
    memset(response, 0, sizeof(response));
    bytes_read = kernel_read(serial_fp, response, sizeof(response) - 1, 
                             &serial_fp->f_pos);
    
    // STEP 5: Close the serial port
    filp_close(serial_fp, NULL);
    
    if (bytes_read <= 0) {
        printk(KERN_ERR "memory: No response from Arduino, read %zd bytes\n", 
               bytes_read);
        return -EIO;
    }
    
    printk(KERN_DEBUG "memory: Raw response: '%s'\n", response);
    
    // STEP 6: Parse the temperature
    temp_start = strstr(response, "TEMP: ");
    if (!temp_start) {
        printk(KERN_ERR "memory: Unexpected response format: '%s'\n", response);
        return -EIO;
    }
    temp_start += 6;  // Skip "TEMP: "
    
    // STEP 7: Copy to output buffer
    for (i = 0; i < buf_len - 1; i++) {
        char c = temp_start[i];
        if (c == '\n' || c == '\r' || c == '\0') {
            break;
        }
        buffer[i] = c;
    }
    buffer[i] = '\0';
    
    printk(KERN_INFO "memory: Temperature read: %s\n", buffer);
    
    return 0;
}

static int memory_open(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "memory: Device opened\n");
    return 0;
}

static int memory_release(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "memory: Device closed\n");
    return 0;
}

static ssize_t memory_read(struct file *filp, char __user *buf, 
                           size_t count, loff_t *f_pos)
{
    int ret;
    size_t to_copy;
    
    printk(KERN_INFO "memory: Read called, count=%zu, f_pos=%lld\n", count, *f_pos);
    
    if (*f_pos >= temp_len) {
        return 0;
    }
    
    ret = get_temperature_from_arduino(temp_buffer, sizeof(temp_buffer));
    if (ret < 0) {
        return ret;
    }
    temp_len = strlen(temp_buffer);
    
    if (*f_pos >= temp_len) {
        return 0;
    }
    
    to_copy = min(count, (size_t)(temp_len - *f_pos));
    
    if (copy_to_user(buf, temp_buffer + *f_pos, to_copy)) {
        printk(KERN_ERR "memory: Failed to copy to user\n");
        return -EFAULT;
    }
    
    *f_pos += to_copy;
    
    printk(KERN_INFO "memory: Read %zu bytes: %.*s\n", to_copy, 
           (int)to_copy, temp_buffer + *f_pos - to_copy);
    
    return to_copy;
}

static ssize_t memory_write(struct file *filp, const char __user *buf,
                            size_t count, loff_t *f_pos)
{
    // Write not supported - read-only device
    return -EPERM;
}

static struct file_operations memory_fops = {
    .owner = THIS_MODULE,
    .open = memory_open,
    .release = memory_release,
    .read = memory_read,
    .write = memory_write,
};

static int __init memory_init(void)
{
    dev_t dev_num;
    int result;
    
    printk(KERN_INFO "memory: Initializing temperature sensor driver\n");
    
    result = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (result < 0) {
        printk(KERN_ERR "memory: Failed to allocate major number\n");
        return result;
    }
    
    major_num = MAJOR(dev_num);
    printk(KERN_INFO "memory: Allocated major number %d\n", major_num);
    
    cdev_init(&memory_cdev, &memory_fops);
    memory_cdev.owner = THIS_MODULE;
    
    result = cdev_add(&memory_cdev, dev_num, 1);
    if (result < 0) {
        printk(KERN_ERR "memory: Failed to add cdev\n");
        unregister_chrdev_region(dev_num, 1);
        return result;
    }
    
    memory_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(memory_class)) {
        printk(KERN_ERR "memory: Failed to create class\n");
        cdev_del(&memory_cdev);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(memory_class);
    }
    
    memory_device = device_create(memory_class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(memory_device)) {
        printk(KERN_ERR "memory: Failed to create device\n");
        class_destroy(memory_class);
        cdev_del(&memory_cdev);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(memory_device);
    }
    
    printk(KERN_INFO "memory: Driver initialized successfully\n");
    printk(KERN_INFO "memory: /dev/memory created automatically\n");
    printk(KERN_INFO "memory: Run: sudo chmod 666 /dev/memory && cat /dev/memory\n");
    
    return 0;
}

static void __exit memory_exit(void)
{
    dev_t dev_num = MKDEV(major_num, 0);
    
    printk(KERN_INFO "memory: Exiting driver\n");
    
    device_destroy(memory_class, dev_num);
    class_destroy(memory_class);
    cdev_del(&memory_cdev);
    unregister_chrdev_region(dev_num, 1);
    
    printk(KERN_INFO "memory: Driver removed\n");
}

module_init(memory_init);
module_exit(memory_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ATU Sligo Student");
MODULE_DESCRIPTION("Temperature sensor driver via Arduino TMP36");