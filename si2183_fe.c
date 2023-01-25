// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  Driver for si2183 Frontend
 *
 *  Written by Emanuel Strobel <emanuel@ihad.tv>
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/i2c.h>
#include <linux/gpio.h>

#include <dt-bindings/gpio/gpio.h>

#include <media/dvb_frontend.h>
#include "si2183.h"

#include "Silabs_L0_API.h"
#include "SiLabs_API_L3_Wrapper.h"

int si2183_is_sat = 0;
static struct i2c_adapter *si2183_i2c_adapter = NULL;
static struct gpio_desc *si2183_reset_gpiod = NULL;
struct mutex lock;

SILABS_FE_Context   FrontEnd_Table[FRONT_END_COUNT];
SILABS_FE_Context	*front_end;
CUSTOM_Status_Struct  FE_Status;
CUSTOM_Status_Struct si2183_custom_status;


int si2183_reg_read(unsigned char clientAddr, unsigned char reg_index, unsigned char *buffer)

{
	struct i2c_msg msg[2];

	msg[0].addr = (__u16)clientAddr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = &reg_index;

	msg[1].addr = (__u16)clientAddr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = buffer;

	return i2c_transfer(si2183_i2c_adapter, msg, 2);
}

int si2183_reg_write(unsigned char clientAddr, unsigned char reg_index, unsigned char new_reg_value)

{
	struct i2c_msg msg;
	unsigned char buf[2] = { reg_index, new_reg_value };

	msg.addr = (__u16)clientAddr;
	msg.flags = 0;
	msg.len = 2;
	msg.buf = buf;

	return i2c_transfer(si2183_i2c_adapter, &msg, 1);

}

int I2C_ReadBytes(unsigned char clientAddr, 
						 unsigned char indexSize, 
						 signed int iNbBytes, 
						 unsigned char *pucDataBuffer) 
{
	int n, err;
	unsigned char clientRegAddr;
	
	err = -1;
	clientRegAddr = indexSize;

	mutex_lock(&lock);
	
	for(n = 0; iNbBytes > n; n++) {
		if (si2183_reg_read(clientAddr, clientRegAddr, &pucDataBuffer[n]) < 0) {
			printk(KERN_ERR "%s_fe: %s: Error: can not read from i2c device %#02x\n", DRIVER_NAME, __FUNCTION__, clientAddr);
			err--;
			break;
		}
		else {
			err = 0;
		}
		printk(KERN_INFO "%s_fe: %s: (%d) client: %#02x clientRegAddr: %#02x val: %#02x err=%d\n", DRIVER_NAME, __FUNCTION__, n+1, clientAddr, clientRegAddr, pucDataBuffer[n], err);
		clientRegAddr++;
	}
	
	mutex_unlock(&lock);
	
	return err;
}

int I2C_WriteByte( unsigned char clientAddr, 
						 unsigned char indexSize, 
						 signed int iNbBytes, 
						 unsigned char *pucDataBuffer) 
{
	int n, err;
	unsigned char clientRegAddr;
	
	err = -1;
	clientRegAddr = indexSize;
	
	mutex_lock(&lock);

	for(n = 0; iNbBytes > n; n++) {
		err = si2183_reg_write(clientAddr, clientRegAddr, pucDataBuffer[n]);
		if (err < 1) {
			printk(KERN_ERR "%s_fe: %s: Error: can not write to i2c device %#02x\n", DRIVER_NAME, __FUNCTION__, clientAddr);
			err--;
			break;

		/*} else if (err != 2) {
			err = EREMOTEIO; */
		} else {
			err = 0;
		}
		printk(KERN_INFO "%s_fe: %s: (%d) client: %#02x clientRegAddr: %#02x val: %#02x err=%d\n", DRIVER_NAME, __FUNCTION__, n+1, clientAddr, clientRegAddr, pucDataBuffer[n], err);
		clientRegAddr++;
	}
	
	mutex_unlock(&lock);
	
	return err;
}

static void si2183_reset(void)
{
	gpiod_set_value(si2183_reset_gpiod, 0);
	msleep(300);
	printk(KERN_INFO "%s_fe: %s: set reset gpiod to 0\n", DRIVER_NAME, __FUNCTION__);
	gpiod_set_value(si2183_reset_gpiod, 1);
	msleep(100);
	printk(KERN_INFO "%s_fe: %s: set reset gpiod to 1\n", DRIVER_NAME, __FUNCTION__);
}

static int si2183_read_status(struct dvb_frontend *fe,
					enum fe_status *status)
{
	unsigned char s;
	
	s = 0;
	
	SiLabs_API_Demod_status(front_end, &si2183_custom_status);
	printk(KERN_INFO "%s_fe: %s: lock status is %x\n", DRIVER_NAME, __FUNCTION__, si2183_custom_status.fec_lock);
	
	s = si2183_custom_status.fec_lock;
	
	if(s == 1)
	{
		*status = FE_HAS_LOCK|FE_HAS_SIGNAL|FE_HAS_CARRIER|FE_HAS_VITERBI|FE_HAS_SYNC;
	} else {
		*status = FE_TIMEDOUT;
	}
	
	return  0;
}

static int si2183_read_ber(struct dvb_frontend *fe, u32 *ber)
{
	SiLabs_API_Demod_status(front_end, &si2183_custom_status);
	
	*ber = si2183_custom_status.ber_count;
	
	return 0;
}

static int si2183_read_signal_strength(struct dvb_frontend *fe,
						 u16 *strength)
{
	SiLabs_API_Demod_status(front_end, &si2183_custom_status);
	
	*strength = si2183_custom_status.RSSI;
	
	return 0;
}

static int si2183_read_snr(struct dvb_frontend *fe, u16 *snr)
{
	SiLabs_API_Demod_status(front_end, &si2183_custom_status);
	
	*snr = si2183_custom_status.c_n_100;
	
	return 0;
}

static int si2183_read_ucblocks(struct dvb_frontend *fe, u32 *ucblocks)
{
	*ucblocks = 0;
	return 0;
}

/*
 * Should only be implemented if it actually reads something from the hardware.
 * Also, it should check for the locks, in order to avoid report wrong data
 * to userspace.
 */
static int si2183_get_frontend(struct dvb_frontend *fe,
					 struct dtv_frontend_properties *p)
{
	return 0;
}

static int si2183_set_frontend(struct dvb_frontend *fe)
{
	if (fe->ops.tuner_ops.set_params) {
		fe->ops.tuner_ops.set_params(fe);
		if (fe->ops.i2c_gate_ctrl)
			fe->ops.i2c_gate_ctrl(fe, 0);
	}

	return 0;
}

static int si2183_sleep(struct dvb_frontend *fe)
{
	return (int) !SiLabs_API_set_standard(front_end, SILABS_SLEEP);
}

static int si2183_init(struct dvb_frontend *fe)
{
	printk(KERN_INFO "%s_fe: %s: - start\n", DRIVER_NAME, __FUNCTION__);
	
	si2183_reset();
	Si21662_EVB_Rev1_0_sat
	SiLabs_API_Demods_Kickstart();
	Si2183_L2_switch_to_standard (front_end->Si2183_FE, SILABS_DVB_S2, 1);
	
	printk(KERN_INFO "%s_fe: %s: - complete\n", DRIVER_NAME, __FUNCTION__);

	return 0;
}

static int si2183_set_tone(struct dvb_frontend *fe,
				 enum fe_sec_tone_mode tone)
{
	unsigned char si2183_tone;
	
	switch (tone) {
		case SEC_TONE_ON: 
			si2183_tone = 1;
			break;
		default: 
			si2183_tone = 0;
			break;
	}
	return (int) !SiLabs_API_SAT_tone(front_end, si2183_tone);
}

static int si2183_set_voltage(struct dvb_frontend *fe,
					enum fe_sec_voltage voltage)
{
	int si2183_voltage;
	
	switch (voltage) {
		case SEC_VOLTAGE_13:
			si2183_voltage = 13;
			break;

		case SEC_VOLTAGE_18:
			si2183_voltage = 18;
			break;

		default:
			si2183_voltage = 0;
			break;
	}
	return (int) !SiLabs_API_SAT_voltage(front_end, si2183_voltage);
}

static void si2183_release(struct dvb_frontend *fe)
{
	struct si2183_state *state = fe->demodulator_priv;
	printk(KERN_INFO "%s_fe: %s\n", DRIVER_NAME, __FUNCTION__);

	kfree(state);
}

static const struct dvb_frontend_ops si2183_qpsk_ops;
static const struct dvb_frontend_ops si2183_qam_ops;

struct dvb_frontend *si2183_attach(const struct si2183_config *config, struct i2c_adapter *i2c)
{
#if 0
	/* test buffers r/w */
	unsigned char registerDataBuffer[8];
	unsigned char DataBuffer[13];
	unsigned char addr;
	struct si2183_state *state = NULL;
	addr = 0xc8;
#else
	struct si2183_state *state = NULL;
#endif
	si2183_reset_gpiod = config->reset_gpiod;
	si2183_i2c_adapter = i2c;
	
	printk(KERN_INFO "%s_fe: %s: - dummy frontend added!\n", DRIVER_NAME, __FUNCTION__);
	
	if(si2183_i2c_adapter == NULL) {
		printk(KERN_ERR "%s_fe: %s: Error: si2183_i2c_adapter NULL!\n", DRIVER_NAME, __FUNCTION__);
	}
	
	mutex_init(&lock);
#if 0
	printk(KERN_INFO "%s_fe: %s: - read test:\n", DRIVER_NAME, __FUNCTION__);
	I2C_ReadBytes(addr, 0x0, 8, registerDataBuffer);
	
	memcpy(DataBuffer, "\xc0\x12\x00\x0c\x00\x0d\x16\x00\x00\x00\x00\x00\x00", 13);
	printk(KERN_INFO "%s_fe: %s: - write test:\n", DRIVER_NAME, __FUNCTION__);
	I2C_WriteByte(addr, 0x0, 13, DataBuffer);
	
#endif
	/* allocate memory for the internal state */
	state = kzalloc(sizeof(struct si2183_state), GFP_KERNEL);
	if (!state)
		return NULL;
	
	printk(KERN_INFO "%s_fe: %s: - write test LNB\n", DRIVER_NAME, __FUNCTION__);
	si2183_is_sat = si2183_reg_write(0x09, 0x0, 0x01);
	if (si2183_is_sat < 0) {
		printk(KERN_INFO "%s_fe: %s: - No LNB found @0x09, setting upDVB-T/T2/C demodulator...\n", DRIVER_NAME, __FUNCTION__);
		/* create dvb_frontend for sat */
		memcpy(&state->frontend.ops,
		   &si2183_qam_ops,
		   sizeof(struct dvb_frontend_ops));
	}
	else {
		printk(KERN_INFO "%s_fe: %s: LNB found @0x09, setting up DVB-S/S2 demodulator...\n", DRIVER_NAME, __FUNCTION__);
		/* create dvb_frontend for ter/c */
		memcpy(&state->frontend.ops,
		   &si2183_qpsk_ops,
		   sizeof(struct dvb_frontend_ops));
	}

	state->frontend.demodulator_priv = state;
	return &state->frontend;
}

static const struct dvb_frontend_ops si2183_qpsk_ops = {
	.delsys = { SYS_DVBS, SYS_DVBS2 },
	.info = {
		.name	= "si21662 DVB-S/S2 demodulator",
		.frequency_min_hz	=  950 * MHz,
		.frequency_max_hz	= 2150 * MHz,
		.frequency_stepsize_hz	= 250 * kHz,
		.frequency_tolerance_hz	= 29500 * kHz,
		.symbol_rate_min	= 1000000,
		.symbol_rate_max	= 45000000,
		.caps = FE_CAN_INVERSION_AUTO |
			FE_CAN_FEC_1_2 |
			FE_CAN_FEC_2_3 |
			FE_CAN_FEC_3_4 |
			FE_CAN_FEC_5_6 |
			FE_CAN_FEC_6_7 |
			FE_CAN_FEC_7_8 |
			FE_CAN_FEC_AUTO |
			FE_CAN_QPSK
	},

	.release = si2183_release,

	.init = si2183_init,
	.sleep = si2183_sleep,

	.set_frontend = si2183_set_frontend,
	.get_frontend = si2183_get_frontend,

	.read_status = si2183_read_status,
	.read_ber = si2183_read_ber,
	.read_signal_strength = si2183_read_signal_strength,
	.read_snr = si2183_read_snr,
	.read_ucblocks = si2183_read_ucblocks,

	.set_voltage = si2183_set_voltage,
	.set_tone = si2183_set_tone,
};

static const struct dvb_frontend_ops si2183_qam_ops = {
	.delsys = { SYS_DVBT, SYS_DVBT2, SYS_DVBC_ANNEX_A },
	.info = {
			.name   = "si2183 DVB-T/T2/C demodulator",
			.frequency_min_hz      =  48 * MHz,
			.frequency_max_hz      = 870 * MHz,
			.frequency_stepsize_hz = 62500,
			.symbol_rate_min       = 1000000,
			.symbol_rate_max       = 7200000,
			.caps =	FE_CAN_FEC_1_2 |
				FE_CAN_FEC_2_3 |
				FE_CAN_FEC_3_4 |
				FE_CAN_FEC_5_6 |
				FE_CAN_FEC_7_8 |
				FE_CAN_FEC_AUTO |
				FE_CAN_QPSK |
				FE_CAN_QAM_16 |
				FE_CAN_QAM_32 |
				FE_CAN_QAM_64 |
				FE_CAN_QAM_128 |
				FE_CAN_QAM_256 |
				FE_CAN_QAM_AUTO |
				FE_CAN_TRANSMISSION_MODE_AUTO |
				FE_CAN_GUARD_INTERVAL_AUTO |
				FE_CAN_HIERARCHY_AUTO |
				FE_CAN_MUTE_TS |
				FE_CAN_2G_MODULATION |
				FE_CAN_MULTISTREAM
	},

	.release = si2183_release,

	.init = si2183_init,
	.sleep = si2183_sleep,

	.set_frontend = si2183_set_frontend,
	.get_frontend = si2183_get_frontend,

	.read_status = si2183_read_status,
	.read_ber = si2183_read_ber,
	.read_signal_strength = si2183_read_signal_strength,
	.read_snr = si2183_read_snr,
	.read_ucblocks = si2183_read_ucblocks,
};
