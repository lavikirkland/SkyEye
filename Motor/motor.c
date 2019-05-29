#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h> /* O_ACCMODE */
#include <linux/timer.h> 
#include <linux/jiffies.h>

#include <asm/system.h> /* cli(), *_flags */
#include <asm/uaccess.h> /* copy_from/to_user */
#include <asm/arch/pxa-regs.h>
#include <asm/arch/gpio.h>
#include <asm/arch/hardware.h>
#include <linux/interrupt.h>

MODULE_LICENSE("Dual BSD/GPL");

static int motor_init(void);
static void motor_exit(void);
static int motor_open(struct inode *inode, struct file *filp);
static int motor_release(struct inode *inode, struct file *filp);
static void motor_timer_callback(unsigned long data);
static ssize_t motor_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);


// gpio output for different parts
static unsigned int horzMotorStep = 30;
static unsigned int horzMotorDirection = 31;
static unsigned int vertMotorStep = 28;
static unsigned int vertMotorDirection = 29; 
static unsigned int trigger = 16;

//file operation
struct file_operations motor_fops = {
	write: motor_write,
	open: motor_open,
	release: motor_release
};

static struct timer_list mytimer;

static int capacity = 256; 
static int count = 0;

static int horzStep = 0; // used to count horizontal angles
static int vertStep = 0; // used to count vertical angles
static int horzMove = 0;
static int vertMove = 0;
static int horzDrctn = 0; // 0: left, 1: right
static int vertDrctn = 0; // 0: up, 1: down  
static int shootCount = 0;
static int shoot = 1; // 1: off, 0: on

//default command settings
static char target = 'N';
static char H = 'N';
static char V = 'N';

module_init(motor_init);
module_exit(motor_exit);

static int motor_init(void)
{
	int result;

	result = register_chrdev(61, "motor", &motor_fops);

	if (result < 0)
	{
		printk(KERN_ALERT "mytimer: cannot obtain major number %d\n", 61);
		return result;
	}

	// specifying gpio output signals
	gpio_direction_output(horzMotorStep, 0);
	gpio_direction_output(horzMotorDirection, 0);
	gpio_direction_output(vertMotorStep, 0);
	gpio_direction_output(vertMotorDirection, 0);
	gpio_direction_output(trigger, 0);
	//initialize gpio
	pxa_gpio_set_value(horzMotorStep, 0);
	pxa_gpio_set_value(horzMotorDirection, 0);
	pxa_gpio_set_value(vertMotorStep, 0);
	pxa_gpio_set_value(vertMotorDirection, 0);
	pxa_gpio_set_value(trigger, 1);
	count = 0;

	// setup timer
	setup_timer(&mytimer, motor_timer_callback, 0);
	mod_timer(&mytimer, jiffies + msecs_to_jiffies(HZ/100));

	return 0;

}

static void motor_exit(void)
{
	unregister_chrdev(61,"motor");
	del_timer(&mytimer);
	//free gpio signals
	gpio_free(horzMotorStep);
	gpio_free(horzMotorDirection);
	gpio_free(vertMotorStep);
	gpio_free(vertMotorDirection);
	gpio_free(trigger);
	gpio_free(16);
}

static void motor_timer_callback(unsigned long data)
{
	if (count > 50) //motor not moving 50% of duty cycles
	{
		pxa_gpio_set_value(horzMotorStep, 0);
		pxa_gpio_set_value(horzMotorDirection, horzDrctn);
		pxa_gpio_set_value(vertMotorStep, 0);
		pxa_gpio_set_value(vertMotorDirection, vertDrctn);
		pxa_gpio_set_value(trigger, shoot);
		count += 1;
		if (count >= 100) // update motor and trigger signal when reaches 50% of duty cycles
		{
			//update the count value
			count = 0;
printk("2 target = %c, H = %c, V = %c\n", target, H, V);

			if (target == '1')
				shoot = 0;
			else
				shoot = 1;

			if (shoot == 0) // if recieved "shoot" command, we shoot and stop moving the gun
			{
				horzMove = 0;
				vertMove = 0;
				shoot = 0;
			} else if (H != 'N' || V != 'N') // if detect target but not aiming at it, we move the gun and not shoot
				horzMove = 1;
				// update the rotation direction
				if (H == 'L') // rotate to the left
					horzDrctn = 0; 
				else if (H == 'R') // rotate to the right
					horzDrctn = 1;
				else if (H == 'N')
					horzMove = 0; // not moving if the object is align with the z-axis

				vertMove = 1;
				// update the rotation direction
				if (V == 'U') // rotate upwards
					vertDrctn = 0;
				else if (V == 'D') // rotate downwards
					vertDrctn = 1;
				else if (V == 'N')
					vertMove = 0; // not moving if the object is align with the horizontal plane
				shoot = 1;
			} else // if no object detected, we scan horizontally around, 
			{
				horzMove = 1;
				shoot = 1;
				if (horzStep > 50) // if reach 90 degrees to the right, we move back to left
				{
					horzStep = 50;
					horzDrctn = 0;
				}
				if (horzStep < -50)// if reach 90 degrees to the left, we move back to right
				{
					horzStep = -50;
					horzDrctn = 1;
				}	
				// updating roation angles
				if (horzDrctn == 1)
					horzStep += 1;
				else
					horzStep -= 1;		
			}
printk("shoot = %d, vertStep = %d, vertDrctn = %d, vertMove = %d, horzStep = %d, horzDrctn = %d, horzMove = %d\n", shoot, vertStep, vertDrctn, vertMove, horzStep, horzDrctn, horzMove);
		}
		
	} else
	{	//motor moves for the other 50% of duty cycles
		count += 1;
		pxa_gpio_set_value(horzMotorStep, horzMove);
		pxa_gpio_set_value(horzMotorDirection, horzDrctn);
		pxa_gpio_set_value(vertMotorStep, vertMove);
		pxa_gpio_set_value(vertMotorDirection, vertDrctn);
		pxa_gpio_set_value(trigger, shoot);
	}

	mod_timer(&mytimer, jiffies + HZ/100);
}

static int motor_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int motor_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t motor_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	char instr[16];

	if (count > capacity - *f_pos)
		count = capacity - *f_pos;

	if (copy_from_user(instr, buf, count))
	{
		return -EFAULT;
	}

	if (count > 256)
		return count;
	
	//read 3 bits from user level
	target = instr[0];
	H = instr[1];
	V = instr[2];

	// default situation
	if (target == '0' || H == '0' || V == '0')
	{
		printk("NULL instruction\n");
		target = 'N';
		H = 'N';
		V = 'N';
	}

	printk("1 target = %c, H = %c, V = %c\n", target, H, V);

	*f_pos += count;

	return count; 
}
