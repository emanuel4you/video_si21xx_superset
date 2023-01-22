/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *  Driver for si2183 Frontend
 *
 *  Written by Emanuel Strobel <emanuel@ihad.tv>
 */

#ifndef DVB_SI2183_H
#define DVB_SI2183_H

#include <linux/dvb/frontend.h>
#include <media/dvb_frontend.h>
#include <linux/gpio/consumer.h>

#define DRIVER_NAME "si2183"


struct si2183_state {
	struct dvb_frontend frontend;
};

struct si2183_config {
	/* the demodulator's i2c address */
	unsigned char demod_address_0;
	unsigned char demod_address_1;
	/* the demodulator's reset gpios */
	//struct gpio_desc *reset_gpio;
	struct device *dev;
	
	
};

void fe_exit(void);
struct dvb_frontend *si2183_ofdm_attach(void);
struct dvb_frontend *si2183_qpsk_attach(void);
struct dvb_frontend *si2183_qam_attach(void);
struct dvb_frontend *si2183_attach(const struct si2183_config *config, struct i2c_adapter *i2c);

/* the demodulator's api read foo */
int I2C_ReadBytes(unsigned char clientAddr, 
						 unsigned char indexSize, 
						 signed int iNbBytes, 
						 unsigned char *pucDataBuffer);

/* the demodulator's api write foo */
int I2C_WriteByte( unsigned char clientAddr, 
						 unsigned char indexSize, 
						 signed int iNbBytes, 
						 unsigned char *pucDataBuffer);


#endif // DVB_SI2183_H
