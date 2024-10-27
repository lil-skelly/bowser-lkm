#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/random.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("lil-skelly");
MODULE_DESCRIPTION("Bowser's defining traits include his monstrous appearance with dragon-like elements, full-throated roar, fire-breathing abilities, and tyrannical personality.");
MODULE_VERSION("48.0");

dev_t dev = 0;
static struct class *bowser_class;
static struct cdev bowser_cdev;
uint8_t *kbuffer;


static ssize_t  bowser_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
//static ssize_t  bowser_write(struct file *filp, const char *buf, size_t len, loff_t * off);

static struct file_operations bowser_fops = 
{
    .owner = THIS_MODULE,
    .read  = bowser_read,
//  .write = bowser_write
};

static const char *bowser_banners[] = {
"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⠺⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀ \n"
"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⠤⣀⠀⠀⠀⡶⣄⠀⠀⠀⠀⠀⠀⠀⠀⡰⠁⡀⢧⠀⠀⠀⠀⠀⣤⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀ \n"
"⠀⠀⠀⠀⠀⠀⠀⠀⣀⡀⢠⢊⠤⠠⢄⠑⣮⠒⠇⠈⠳⢄⠀⠀⠀⠀⠀⡰⠁⠀⠁⢸⠀⠀⠀⠀⠀⣿⣧⣰⣿⡆⣀⢸⣦⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⠀⠀⡰⠋⠀⠀⠈⢿⡏⠀⠀⠈⡆⣿⠀⠀⠀⡀⠀⠱⣄⠀⢠⠖⠓⠒⠲⠤⢤⡀⠀⠀⠀⠀⣿⡿⠿⣻⣿⣿⣾⣿⣷⡀⠀⣀⣴⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⠀⢘⠀⠀⠀⠐⢯⣿⢧⠀⠀⢠⡷⠿⡤⠆⠀⠁⠀⣀⠜⢙⣯⣐⣒⠲⠶⠶⢶⣿⠀⠀⠀⡴⠋⢠⣾⣿⣿⣿⣿⣿⣿⣿⣾⣿⣿⣿⢖⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⣀⣀⠼⢤⠖⠂⠐⢺⣿⠛⠦⠒⠉⠀⠐⢳⡏⠀⠈⠉⢀⣠⣾⠿⠧⣸⣄⣉⡉⢘⡿⠛⠉⢹⠀⠀⠈⢻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⢸⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠰⠋⠀⠀⢀⡈⣆⠀⠀⢸⣿⠄⠀⠀⠀⠀⠀⢸⡧⢤⡴⠞⢫⠷⡄⠀⠀⡀⠀⠀⠀⠀⠀⠀⢀⣾⠀⠀⠀⠈⢻⢿⣿⣿⣷⣽⣿⣿⣿⣿⣿⠇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠃⠀⠀⠀⢸⣿⠊⠳⣄⣸⣿⠀⠀⠀⣀⠄⠀⠀⢑⣾⣶⣾⣧⣥⣼⣮⡁⠀⠈⠉⠲⣤⣤⣾⣿⣿⣆⠀⢀⡤⠋⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⢰⡀⠀⣀⣴⣹⠃⠀⠀⠀⠁⠀⠀⠀⣏⣀⣤⢤⣧⣯⢱⡀⠙⣿⣿⣿⣿⡄⠀⠀⠀⠈⢻⡛⣿⣿⣟⠉⢁⣠⠶⢿⣾⣿⣿⣿⣿⣿⣿⣿⣿⣦⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠈⠳⡎⠁⠀⠹⡀⠀⠀⠀⡦⣐⡤⠒⠉⠀⠈⢱⠟⠈⢀⣷⠀⢸⣿⡇⠨⣻⠀⠀⠀⠀⠀⣿⣿⠟⠋⠉⠙⠳⣄⠀⠙⣿⣿⣿⣿⣿⣿⣿⣿⣿⡆⡀⠀⠀⠀⠀⠀⣴⢄⠀⠀⠀⠀\n"
"⠀⠀⠘⠢⣀⠀⡎⠒⠤⣬⣗⠈⠑⠦⣀⣀⣀⡞⢀⣸⡟⠉⣧⢀⣿⣷⣶⣿⠀⠀⠀⠀⠀⣿⠃⠀⠀⠀⠀⠀⠈⠳⣴⡃⢿⢿⣿⣿⡏⠹⣿⣿⡋⠸⣦⠤⢤⣀⡀⡃⠀⣱⡄⣠⡄\n"
"⠀⠀⠀⠀⠀⠉⣁⣠⢤⢻⡿⣿⣿⣿⣿⡿⢿⣿⡿⠋⠀⡜⠀⢸⣿⣿⣿⣿⠀⠀⠀⠀⢀⣿⠀⠀⠀⠀⠀⠀⠀⢠⠎⠙⠓⠤⣿⢻⣧⡾⠿⣿⣿⣷⡿⡦⠚⠀⠙⣏⠁⠀⢹⠁⠀\n"
"⠀⠀⠀⠶⣏⠁⠀⠀⡇⠈⡏⣹⠙⠛⠣⣆⡞⣉⠀⠀⠘⠀⢠⣾⡿⠛⢻⠏⠀⠀⠀⠀⢸⣿⣧⠀⠀⢠⣴⣤⡐⠁⠀⠀⠀⠀⠀⠀⠀⠀⣠⣀⠈⢻⡧⣹⠂⠀⢀⣗⠀⢀⡮⠓⠶\n"
"⠀⠀⠀⠀⠈⠑⠢⣠⡁⢠⢁⠃⠀⠀⢠⣿⡿⠉⠉⠳⢶⣶⣿⠿⣷⡶⡜⠀⠀⠀⠀⢀⠞⣿⣼⠀⠀⢸⣏⣽⣷⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠙⡆⡜⡧⣀⢀⠤⡾⠃⣠⠟⢋⡔⠁\n"
"⠀⠀⠀⠀⠀⠀⠀⣤⠓⠾⡼⠀⠀⠀⣸⣿⡇⠀⠀⠀⠈⠷⠁⠀⠘⠀⠀⠀⠀⠀⣠⠋⠀⠘⣿⣧⠀⠀⢿⣛⣿⣿⣧⣄⠀⠀⠀⠀⠀⠀⠀⠈⠀⡰⠋⠌⠉⠀⣁⡴⠡⠔⠋⠀⠀\n"
"⠀⠀⠀⠀⠀⠀⠀⠀⠉⠒⡇⠀⠀⢀⣿⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⠞⠉⣦⠀⠀⠈⢿⡄⠀⠀⠙⠿⠯⠿⢷⣷⣶⣦⣴⣶⣶⣤⡴⢾⡀⠀⠀⠀⡼⠋⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⠀⠀⠀⠀⠀⣀⠀⡇⠀⠀⣼⡿⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⠚⠉⣄⠀⠈⠃⠀⠀⠀⠹⣄⠀⠀⠀⠀⠀⠀⠀⠀⠈⣿⣿⣿⣹⣿⣷⣌⣀⡴⠋⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⢠⠢⢄⡀⣠⠊⣽⠀⢳⠀⢠⡟⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡰⡃⠀⠀⠈⠓⠤⣀⠀⠀⠀⢀⡹⣷⣤⣀⣀⣀⡀⣀⣀⢼⠁⠈⠿⣿⣿⣿⠿⡿⠇⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⢣⠀⠌⣧⠤⠿⠤⣠⠃⡼⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣼⠃⠈⠲⠄⠀⠀⠀⠀⠉⠐⠂⠉⣿⣿⣷⣿⣿⠀⠀⠀⠀⠐⢤⣀⠀⠈⣙⠿⠒⠇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⠀⠣⡀⠀⠀⠁⠀⢟⣴⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡰⢸⢣⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢺⠋⠉⠉⠈⠙⢻⡶⠤⣀⠤⢌⡫⠔⠊⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⠀⠀⠙⣝⢤⡤⠊⠀⠈⠀⠀⠀⠀⠀⠀⠀⠀⠁⠀⡰⠃⣸⣆⠈⠒⠤⣄⡀⠀⠀⠀⠀⣠⠏⠀⠀⠀⠀⠀⠀⢸⠀⠀⠉⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⠀⠀⠀⢈⠝⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠘⢁⡼⡿⠈⠑⠠⢄⣀⠀⠀⠀⣠⠞⠁⠀⠀⠀⠀⠀⠀⠀⢀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⠀⢀⠞⠁⠀⠀⠀⠀⠀⠀⠀⠀⡀⠀⠀⠀⡀⠀⠀⠀⢠⠾⠶⠤⢤⠤⢤⣽⣚⣉⣠⠤⠂⠀⠀⠀⠀⠀⠀⠀⠸⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⢀⠎⢀⠣⠀⠀⠀⠐⠀⠄⠀⡜⠀⣰⡃⡜⠀⠀⣠⠾⠿⠒⠂⠈⠁⠀⠀⢸⠉⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⣰⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⡈⠸⠈⣄⡤⢟⣢⣄⡐⢠⣨⣀⣞⡷⣏⡷⣠⠔⠁⠀⠀⠀⠀⠀⠀⠀⠈⠸⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠒⠤⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⣿⠀⠀⠀⠘⣿⠁⠉⠍⢻⡟⠛⢿⣿⡛⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢰⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠑⢚⣄⡠⢷⣖⠒⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⢏⢰⠋⠉⠙⣿⢠⠋⠈⠳⣿⡞⠉⠻⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠘⣄⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡎⠀⢱⣀⡹⣧⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n"
"⠀⠀⠀⠈⠿⢤⣀⡼⠟⠿⢦⣀⣴⠛⠓⣤⣴⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠉⠉⠉⠉⠉⠉⠉⠉⠉⠉⠉⠉⠁⠉⠀⠁\n\0",
"My army!\n\0",
"Not sure if you know who I am, but I'm about to marry a princess and rule the world. ...\n\0",
"Open the gates...or die!\n\0",
"I finally found it, AND NOW NO ONE CAN STOP ME!!\n\0",
"Goodbye, Mario.\n\0",
"GET HIM OUT OF MY SIGHT!!\n\0"
};

/* return random integer from x to y */
int get_random_number(int x, int y) {
    return x + get_random_u32() % (y - x + 1);
}
static char *random_bowser_banner(void) {
    int index = get_random_number(0, 6);
    return bowser_banners[index];
}
static ssize_t  bowser_read(struct file *filp, char __user *buf, size_t len,loff_t * off) {
    const char *banner = random_bowser_banner(); // Get banner string
    size_t banner_len = strlen(banner); // Calculate the length of the banner
    size_t error;

    if (*off == 0) {
        size_t to_copy = (len < banner_len) ? len : banner_len;
        error = copy_to_user(buf, banner, to_copy);
        if (error) {
            pr_err("bowser: read() -> copy_to_user failed\n");
            return -EFAULT; 
        }
        *off += banner_len; 
        return banner_len;
    }
    return 0; // EOF
}

static int __init bowser_init(void) {
    if (alloc_chrdev_region(&dev, 0, 1, "bowser") < 0) {
        pr_err("bowser: cannot allocate major number for device\n");
        return -1;
    }
    pr_info("bowser: major->%d | minor->%d\n",MAJOR(dev), MINOR(dev));
    
    /* Initialize cdev structure and add char device to system */
    cdev_init(&bowser_cdev, &bowser_fops);
    if ((cdev_add(&bowser_cdev,dev,1)) < 0){
        pr_err("bowser: Cannot add device to system\n");
        goto err_class;
    }

    /* Create class structure */
    bowser_class = class_create("bowser_cls");
    if (IS_ERR(bowser_class)) {
        pr_err("bowser: Failed to create `class` struct\n");
        goto err_class;
    }
    
    /* Create device */ 
    if (IS_ERR(device_create(bowser_class, NULL, dev, NULL, "bowser"))) {
        pr_err("bowser: Failed to create device\n");
        goto err_dev;
    }
    pr_info("bowser: LKM inserted successfully\n");

    return 0;

err_dev:
    class_destroy(bowser_class);

err_class:
    unregister_chrdev_region(dev, 1);
    return -1;
}

static void __exit bowser_exit(void) {
    device_destroy(bowser_class, dev);
    class_destroy(bowser_class);
    cdev_del(&bowser_cdev);
    unregister_chrdev_region(dev, 1);

    pr_info("bowser: LKM removed successfully\n");
}

module_init(bowser_init);
module_exit(bowser_exit);
