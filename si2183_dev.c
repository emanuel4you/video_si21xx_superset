#include <linux/module.h>
#include <linux/init.h>
#include <linux/mod_devicetable.h>
#include <linux/property.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/i2c.h>
#include <linux/dvb/frontend.h>
#include <media/dvb_frontend.h>

#include "si2183.h"

/* Declate the probe and remove functions */
static int si2183_fe_probe(struct platform_device *pdev);
static int si2183_fe_remove(struct platform_device *pdev);

static struct i2c_adapter * si2183_i2c_adapter = NULL;
struct dvb_frontend *myfrontend;
struct dvb_adapter myadapter;
DVB_DEFINE_MOD_OPT_ADAPTER_NR(adapter_nr);

static struct si2183_config my_si2183_config = {
	/* the demodulators's i2c addresses */
	.demod_address = { 0x64, 0x67 },
	.reset_gpiod = NULL,
};

static int si2183_dt_probe(struct platform_device *pdev) {
	struct device *dev = &pdev->dev;
	const char *label;
	const char *status;
	int tuner_i2c_bus, ret; // id;

	printk(KERN_INFO "%s_dev: %s: - Now I am in the probe function!\n", DRIVER_NAME, __FUNCTION__);
	
	my_si2183_config.dev = &pdev->dev;

	/* Check for device properties */
	if(!device_property_present(dev, "label")) {
		printk(KERN_ERR "%s_dev: %s: Error: Device property 'label' not found!\n", DRIVER_NAME, __FUNCTION__);
		return -1;
	}
	if(!device_property_present(dev, "status")) {
		printk(KERN_ERR "%s_dev: %s: Error: Device property 'status' not found!\n", DRIVER_NAME, __FUNCTION__);
		return -1;
	}
	if(!device_property_present(dev, "tuner_i2c_bus")) {
		printk(KERN_ERR "%s_dev: %s: Error: Device property 'tuner_i2c_bus' not found!\n", DRIVER_NAME, __FUNCTION__);
		return -1;
	}
	if(!device_property_present(dev, "reset-gpios")) {
		printk(KERN_ERR "%s_dev: %s: Error: Device property 'reset-gpios' not found!\n", DRIVER_NAME, __FUNCTION__);
		return -1;
	}

	/* Read device properties */
	ret = device_property_read_string(dev, "label", &label);
	if(ret) {
		printk(KERN_ERR "%s_dev: %s: Error: Could not read 'label'\n", DRIVER_NAME, __FUNCTION__);
		return -1;
	}
	printk(KERN_INFO "%s_dev: %s: - label: %s\n", DRIVER_NAME, __FUNCTION__, label);
	
	ret = device_property_read_string(dev, "status", &status);
	if(ret) {
		printk(KERN_ERR "%s_dev: %s: Error: Could not read 'status'\n", DRIVER_NAME, __FUNCTION__);
		return -1;
	}
	printk(KERN_INFO "%s_dev: %s: - status = %s\n", DRIVER_NAME, __FUNCTION__, status);

	if (!((status == NULL) || !strcmp(status, "okay") || !strcmp(status, "ok"))) {
		printk(KERN_INFO "%s_dev: %s: - set status: off - exit\n", DRIVER_NAME, __FUNCTION__);
		return -1;
	}
	
	ret = device_property_read_u32(dev, "tuner_i2c_bus", &tuner_i2c_bus);
	if(ret) {
		printk(KERN_ERR "%s_dev: %s: Error: Could not read 'tuner_i2c_bus'\n", DRIVER_NAME, __FUNCTION__);
		return -1;
	}
	printk(KERN_INFO "%s_dev: %s: - tuner_i2c_bus = <%d>\n", DRIVER_NAME, __FUNCTION__, tuner_i2c_bus);
	
	my_si2183_config.reset_gpiod = gpiod_get(dev, "reset", GPIOD_OUT_LOW);
	if(IS_ERR(my_si2183_config.reset_gpiod)) {
		printk(KERN_ERR "%s_dev: %s: Error: Could not setup the GPIO\n", DRIVER_NAME, __FUNCTION__);
		return -1;
	}
	printk(KERN_INFO "%s_dev: %s: setup the reset GPIO\n", DRIVER_NAME, __FUNCTION__);

	/* Get an adapter */
	si2183_i2c_adapter = i2c_get_adapter(tuner_i2c_bus);
	if(si2183_i2c_adapter == NULL) {
		printk(KERN_ERR "%s_dev: %s: Error: si2183_i2c_adapter NULL!\n", DRIVER_NAME, __FUNCTION__);
		return -1;
	}
	return 0;
}

/**
 * @brief This function is called on loading the driver 
 */
static int si2183_fe_probe(struct platform_device *pdev) {
	int ret;
	struct device *dev = &pdev->dev;
	
	if (si2183_dt_probe(pdev))
		return -1;
	
	myfrontend = si2183_attach(&my_si2183_config, si2183_i2c_adapter);
	printk(KERN_INFO "%s_dev: %s: - i2c adapter added!\n", DRIVER_NAME, __FUNCTION__);

	ret = dvb_register_adapter(&myadapter, "My Adapter", THIS_MODULE, dev, adapter_nr);
	printk(KERN_INFO "%s_dev: %s: dvb_register_adapter ret=%d\n", DRIVER_NAME, __FUNCTION__, ret);
	
	ret = dvb_register_frontend(&myadapter, myfrontend);
	printk(KERN_INFO "%s_dev: %s: dvb_register_frontend ret=%d\n", DRIVER_NAME, __FUNCTION__, ret);
	
	
	return 0;
}

/**
 * @brief This function is called on unloading the driver 
 */
static int si2183_fe_remove(struct platform_device *pdev) {
	printk(KERN_INFO "%s_dev: %s: - Now I am in the remove function\n", DRIVER_NAME, __FUNCTION__);
	return 0;
}

static struct of_device_id si2183_fe_driver_ids[] = {
	{
		.compatible = "amlogic,dvb",
	}, { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, si2183_fe_driver_ids);

static struct platform_driver si2183_fe_driver = {
	.probe = si2183_fe_probe,
	.remove = si2183_fe_remove,
	.driver = {
		.name = DRIVER_NAME,
		.of_match_table = si2183_fe_driver_ids,
	},
};

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init si2183_fe_init(void) {
	printk(KERN_INFO "%s_dev: %s: - Loading the driver...\n", DRIVER_NAME, __FUNCTION__);
	if(platform_driver_register(&si2183_fe_driver)) {
		printk(KERN_ERR "%s_dev: %s: Error: Could not load driver\n", DRIVER_NAME, __FUNCTION__);
		return -1;
	}
	return 0;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit si2183_fe_exit(void) {
	dvb_unregister_adapter(&myadapter);
	dvb_unregister_frontend(myfrontend);
	gpiod_put(my_si2183_config.reset_gpiod);
	
	printk(KERN_INFO "%s_dev: %s: - Unload driver", DRIVER_NAME, __FUNCTION__);
	platform_driver_unregister(&si2183_fe_driver);
}

module_init(si2183_fe_init);
module_exit(si2183_fe_exit);

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Emanuel Strobel");
MODULE_DESCRIPTION("si2183 DVB Demodulator driver");
