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
	/* the demodulator's reset gpios */
	struct gpio_desc *reset_gpiod;
	struct device *dev;
};

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


#define Si21662_EVB_Rev1_0_sat\
  /* SW Init for front end 0 */\
  front_end                   = &(FrontEnd_Table[0]);\
  SiLabs_API_Frontend_Chip            (front_end, 0x2183);\
  SiLabs_API_SW_Init                  (front_end, 0xc8, 0x00, 0x14);\
  SiLabs_API_Select_SAT_Tuner         (front_end, 0x5816d, 0);\
  SiLabs_API_SAT_Select_LNB_Chip      (front_end, 26, 0x12);\
  SiLabs_API_SAT_LNB_Chip_Index       (front_end, 0);\
  SiLabs_API_SAT_tuner_I2C_connection (front_end, 0);\
  SiLabs_API_SAT_Clock                (front_end, 2, 44, 27, 1);\
  SiLabs_API_SAT_Spectrum             (front_end, 0);\
  SiLabs_API_SAT_AGC                  (front_end, 0xc, 1, 0x0, 0);\
  SiLabs_API_Set_Index_and_Tag        (front_end, 0, "fe[0]");\
  SiLabs_API_HW_Connect               (front_end, 1);\
\
  /* SW Init for front end 1 */\
  front_end                   = &(FrontEnd_Table[1]);\
  SiLabs_API_Frontend_Chip            (front_end, 0x2183);\
  SiLabs_API_SW_Init                  (front_end, 0xce, 0x00, 0x16);\
  SiLabs_API_Select_SAT_Tuner         (front_end, 0x5816d, 0);\
  SiLabs_API_SAT_Select_LNB_Chip      (front_end, 26, 0x12);\
  SiLabs_API_SAT_LNB_Chip_Index       (front_end, 1);\
  SiLabs_API_SAT_tuner_I2C_connection (front_end, 0);\
  SiLabs_API_SAT_Clock                (front_end, 2, 44, 27, 1);\
  SiLabs_API_SAT_Spectrum             (front_end, 0);\
  SiLabs_API_SAT_AGC                  (front_end, 0xb, 1, 0x0, 0);\
  SiLabs_API_Set_Index_and_Tag        (front_end, 1, "fe[1]");\
  SiLabs_API_HW_Connect               (front_end, 1);


#endif // DVB_SI2183_H
