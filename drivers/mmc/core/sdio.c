/*
 *  linux/drivers/mmc/sdio.c
 *
 *  Copyright 2006-2007 Pierre Ossman
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 */

#include <linux/err.h>

#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/sdio_func.h>

#include "core.h"
#include "bus.h"
#include "sdio_bus.h"
#include "mmc_ops.h"
#include "sd_ops.h"
#include "sdio_ops.h"
#include "sdio_cis.h"

#ifdef CONFIG_MMC_EMBEDDED_SDIO
#include <linux/mmc/sdio_ids.h>
#endif

static int sdio_read_fbr(struct sdio_func *func)
{
	int ret;
	unsigned char data;

	ret = mmc_io_rw_direct(func->card, 0, 0,
		SDIO_FBR_BASE(func->num) + SDIO_FBR_STD_IF, 0, &data);
	if (ret)
		goto out;

	data &= 0x0f;

	if (data == 0x0f) {
		ret = mmc_io_rw_direct(func->card, 0, 0,
			SDIO_FBR_BASE(func->num) + SDIO_FBR_STD_IF_EXT, 0, &data);
		if (ret)
			goto out;
	}

	func->class = data;

out:
	return ret;
}

static int sdio_init_func(struct mmc_card *card, unsigned int fn)
{
	int ret;
	struct sdio_func *func;

	BUG_ON(fn > SDIO_MAX_FUNCS);

	func = sdio_alloc_func(card);
	if (IS_ERR(func))
		return PTR_ERR(func);

	func->num = fn;

	ret = sdio_read_fbr(func);
	if (ret)
		goto fail;

	ret = sdio_read_func_cis(func);
	if (ret)
		goto fail;

	card->sdio_func[fn - 1] = func;

	return 0;

fail:
	/*
	 * It is okay to remove the function here even though we hold
	 * the host lock as we haven't registered the device yet.
	 */
	sdio_remove_func(func);
	return ret;
}

static int sdio_read_cccr(struct mmc_card *card)
{
	int ret;
	int cccr_vsn;
	unsigned char data;

	memset(&card->cccr, 0, sizeof(struct sdio_cccr));

	ret = mmc_io_rw_direct(card, 0, 0, SDIO_CCCR_CCCR, 0, &data);
	if (ret)
		goto out;

	cccr_vsn = data & 0x0f;

	if (cccr_vsn > SDIO_CCCR_REV_1_20) {
		printk(KERN_ERR "%s: unrecognised CCCR structure version %d\n",
			mmc_hostname(card->host), cccr_vsn);
		return -EINVAL;
	}

	card->cccr.sdio_vsn = (data & 0xf0) >> 4;

	ret = mmc_io_rw_direct(card, 0, 0, SDIO_CCCR_CAPS, 0, &data);
	if (ret)
		goto out;

	if (data & SDIO_CCCR_CAP_SMB)
		card->cccr.multi_block = 1;
	if (data & SDIO_CCCR_CAP_LSC)
		card->cccr.low_speed = 1;
	if (data & SDIO_CCCR_CAP_4BLS)
		card->cccr.wide_bus = 1;

	if (cccr_vsn >= SDIO_CCCR_REV_1_10) {
		ret = mmc_io_rw_direct(card, 0, 0, SDIO_CCCR_POWER, 0, &data);
		if (ret)
			goto out;

		if (data & SDIO_POWER_SMPC)
			card->cccr.high_power = 1;
	}

	if (cccr_vsn >= SDIO_CCCR_REV_1_20) {
		ret = mmc_io_rw_direct(card, 0, 0, SDIO_CCCR_SPEED, 0, &data);
		if (ret)
			goto out;

		if (data & SDIO_SPEED_SHS)
			card->cccr.high_speed = 1;
	}

out:
	return ret;
}

static int sdio_enable_wide(struct mmc_card *card)
{
	int ret;
	u8 ctrl;

	if (!(card->host->caps & MMC_CAP_4_BIT_DATA))
		return 0;

	if (card->cccr.low_speed && !card->cccr.wide_bus)
		return 0;

	ret = mmc_io_rw_direct(card, 0, 0, SDIO_CCCR_IF, 0, &ctrl);
	if (ret)
		return ret;

	ctrl |= SDIO_BUS_WIDTH_4BIT;

	ret = mmc_io_rw_direct(card, 1, 0, SDIO_CCCR_IF, ctrl, NULL);
	if (ret)
		return ret;

	mmc_set_bus_width(card->host, MMC_BUS_WIDTH_4);

	return 0;
}

/*
 * Test if the card supports high-speed mode and, if so, switch to it.
 */
static int sdio_enable_hs(struct mmc_card *card)
{
	int ret;
	u8 speed;

	if (!(card->host->caps & MMC_CAP_SD_HIGHSPEED))
		return 0;

	if (!card->cccr.high_speed)
		return 0;

	ret = mmc_io_rw_direct(card, 0, 0, SDIO_CCCR_SPEED, 0, &speed);
	if (ret)
		return ret;

	speed |= SDIO_SPEED_EHS;

	ret = mmc_io_rw_direct(card, 1, 0, SDIO_CCCR_SPEED, speed, NULL);
	if (ret)
		return ret;

	mmc_card_set_highspeed(card);
	mmc_set_timing(card->host, MMC_TIMING_SD_HS);

	return 0;
}

/*
 * Host is being removed. Free up the current card.
 */
static void mmc_sdio_remove(struct mmc_host *host)
{
	int i;

	BUG_ON(!host);
	BUG_ON(!host->card);

	for (i = 0;i < host->card->sdio_funcs;i++) {
		if (host->card->sdio_func[i]) {
			sdio_remove_func(host->card->sdio_func[i]);
			host->card->sdio_func[i] = NULL;
		}
	}

	mmc_remove_card(host->card);
	host->card = NULL;
}

/*
 * Card detection callback from host.
 */
static void mmc_sdio_detect(struct mmc_host *host)
{
	int err;

	BUG_ON(!host);
	BUG_ON(!host->card);

	mmc_claim_host(host);

	/*
	 * Just check if our card has been removed.
	 */
	err = mmc_select_card(host->card);

	mmc_release_host(host);

	if (err) {
		mmc_sdio_remove(host);

		mmc_claim_host(host);
		mmc_detach_bus(host);
		mmc_release_host(host);
	}
}


static const struct mmc_bus_ops mmc_sdio_ops = {
	.remove = mmc_sdio_remove,
	.detect = mmc_sdio_detect,
};


/*
 * Starting point for SDIO card init.
 */
int mmc_attach_sdio(struct mmc_host *host, u32 ocr)
{
	int err;
	int i, funcs;
	struct mmc_card *card;

	BUG_ON(!host);
	WARN_ON(!host->claimed);

	mmc_attach_bus(host, &mmc_sdio_ops);

	/*
	 * Sanity check the voltages that the card claims to
	 * support.
	 */
	if (ocr & 0x7F) {
		printk(KERN_WARNING "%s: card claims to support voltages "
		       "below the defined range. These will be ignored.\n",
		       mmc_hostname(host));
		ocr &= ~0x7F;
	}

	if (ocr & MMC_VDD_165_195) {
		printk(KERN_WARNING "%s: SDIO card claims to support the "
		       "incompletely defined 'low voltage range'. This "
		       "will be ignored.\n", mmc_hostname(host));
		ocr &= ~MMC_VDD_165_195;
	}

	host->ocr = mmc_select_voltage(host, ocr);

	/*
	 * Can we support the voltage(s) of the card(s)?
	 */
	if (!host->ocr) {
		err = -EINVAL;
		goto err;
	}

	/*
	 * Inform the card of the voltage
	 */
	err = mmc_send_io_op_cond(host, host->ocr, &ocr);
	if (err)
		goto err;

	/*
	 * For SPI, enable CRC as appropriate.
	 */
	if (mmc_host_is_spi(host)) {
		err = mmc_spi_set_crc(host, use_spi_crc);
		if (err)
			goto err;
	}

	/*
	 * The number of functions on the card is encoded inside
	 * the ocr.
	 */
	funcs = (ocr & 0x70000000) >> 28;

#ifdef CONFIG_MMC_EMBEDDED_SDIO
	if (host->embedded_sdio_data.funcs)
		funcs = host->embedded_sdio_data.num_funcs;
#endif

	/*
	 * Allocate card structure.
	 */
	card = mmc_alloc_card(host, NULL);
	if (IS_ERR(card)) {
		err = PTR_ERR(card);
		goto err;
	}

	card->type = MMC_TYPE_SDIO;
	card->sdio_funcs = funcs;

	host->card = card;

	/*
	 * For native busses:  set card RCA and quit open drain mode.
	 */
	if (!mmc_host_is_spi(host)) {
		err = mmc_send_relative_addr(host, &card->rca);
		if (err)
			goto remove;

		mmc_set_bus_mode(host, MMC_BUSMODE_PUSHPULL);
	}

	/*
	 * Select card, as all following commands rely on that.
	 */
	if (!mmc_host_is_spi(host)) {
		err = mmc_select_card(card);
		if (err)
			goto remove;
	}

	/*
	 * Read the common registers.
	 */

#ifdef CONFIG_MMC_EMBEDDED_SDIO
	if (host->embedded_sdio_data.cccr)
		memcpy(&card->cccr, host->embedded_sdio_data.cccr, sizeof(struct sdio_cccr));
	else {
#endif
		err = sdio_read_cccr(card);
		if (err)
			goto remove;
#ifdef CONFIG_MMC_EMBEDDED_SDIO
	}
#endif

#ifdef CONFIG_MMC_EMBEDDED_SDIO
	if (host->embedded_sdio_data.cis)
		memcpy(&card->cis, host->embedded_sdio_data.cis, sizeof(struct sdio_cis));
	else {
#endif
		/*
		 * Read the common CIS tuples.
		 */
		err = sdio_read_common_cis(card);
		if (err)
			goto remove;
#ifdef CONFIG_MMC_EMBEDDED_SDIO
	}
#endif
// [JLH] Force host controller to high speed timing if clock above 25 Mhz

	if(card->cis.max_dtr > 25000000)
		card->cccr.high_speed = 1;

	/*
	 * Switch to high-speed (if supported).
	 */
	err = sdio_enable_hs(card);
	if (err)
		goto remove;

	/*
	 * Change to the card's maximum speed.
	 */
	if (mmc_card_highspeed(card)) {
		/*
		 * The SDIO specification doesn't mention how
		 * the CIS transfer speed register relates to
		 * high-speed, but it seems that 50 MHz is
		 * mandatory.
		 */
		mmc_set_clock(host, 50000000);
	} else {
		mmc_set_clock(host, card->cis.max_dtr);
	}

	/*
	 * Switch to wider bus (if supported).
	 */
	err = sdio_enable_wide(card);
	if (err)
		goto remove;

	/*
	 * Initialize (but don't add) all present functions.
	 */
	for (i = 0;i < funcs;i++) {
#ifdef CONFIG_MMC_EMBEDDED_SDIO
		if (host->embedded_sdio_data.funcs) {
			struct sdio_func *tmp;

			tmp = sdio_alloc_func(host->card);
			if (IS_ERR(tmp))
				goto remove;
			tmp->num = (i + 1);
			card->sdio_func[i] = tmp;
			tmp->class = host->embedded_sdio_data.funcs[i].f_class;
			tmp->max_blksize = host->embedded_sdio_data.funcs[i].f_maxblksize;
			tmp->vendor = card->cis.vendor;
			tmp->device = card->cis.device;
		} else {
#endif
			err = sdio_init_func(host->card, i + 1);
			if (err)
				goto remove;
#ifdef CONFIG_MMC_EMBEDDED_SDIO
		}
#endif
	}

	mmc_release_host(host);

	/*
	 * First add the card to the driver model...
	 */
	err = mmc_add_card(host->card);
	if (err)
		goto remove_added;

	/*
	 * ...then the SDIO functions.
	 */
	for (i = 0;i < funcs;i++) {
		err = sdio_add_func(host->card->sdio_func[i]);
		if (err)
			goto remove_added;
	}

	return 0;


remove_added:
	/* Remove without lock if the device has been added. */
	mmc_sdio_remove(host);
	mmc_claim_host(host);
remove:
	/* And with lock if it hasn't been added. */
	if (host->card)
		mmc_sdio_remove(host);
err:
	mmc_detach_bus(host);
	mmc_release_host(host);

	printk(KERN_ERR "%s: error %d whilst initialising SDIO card\n",
		mmc_hostname(host), err);

	return err;
}

int sdio_reset_comm(struct mmc_card *card)
{
	struct mmc_host *host = card->host;
	u32 ocr;
	int err;

	printk("%s():\n", __func__);
	mmc_go_idle(host);

	mmc_set_clock(host, host->f_min);

	err = mmc_send_io_op_cond(host, 0, &ocr);
	if (err)
		goto err;

	host->ocr = mmc_select_voltage(host, ocr);
	if (!host->ocr) {
		err = -EINVAL;
		goto err;
	}

	err = mmc_send_io_op_cond(host, host->ocr, &ocr);
	if (err)
		goto err;

	if (mmc_host_is_spi(host)) {
		err = mmc_spi_set_crc(host, use_spi_crc);
		if (err)
		goto err;
	}

	if (!mmc_host_is_spi(host)) {
		err = mmc_send_relative_addr(host, &card->rca);
		if (err)
			goto err;
		mmc_set_bus_mode(host, MMC_BUSMODE_PUSHPULL);
	}
	if (!mmc_host_is_spi(host)) {
		err = mmc_select_card(card);
		if (err)
			goto err;
	}

	mmc_set_clock(host, card->cis.max_dtr);
	err = sdio_enable_wide(card);
	if (err)
		goto err;

	return 0;
 err:
	printk("%s: Error resetting SDIO communications (%d)\n",
	       mmc_hostname(host), err);
	mmc_release_host(host);
	return err;
}
EXPORT_SYMBOL(sdio_reset_comm);


