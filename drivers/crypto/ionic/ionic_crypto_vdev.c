/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2021-2024 Advanced Micro Devices, Inc.
 */

#include <stdint.h>
#include <errno.h>

#include <rte_errno.h>
#include <rte_common.h>
#include <rte_log.h>
#include <rte_eal.h>
#include <bus_vdev_driver.h>
#include <rte_dev.h>
#include <rte_string_fns.h>

#include "ionic_crypto.h"

#define IOCPT_VDEV_DEV_BAR          0
#define IOCPT_VDEV_INTR_CTL_BAR     1
#define IOCPT_VDEV_INTR_CFG_BAR     2
#define IOCPT_VDEV_DB_BAR           3
#define IOCPT_VDEV_BARS_MAX         4

#define IOCPT_VDEV_DEV_INFO_REGS_OFFSET      0x0000
#define IOCPT_VDEV_DEV_CMD_REGS_OFFSET       0x0800

static int
iocpt_vdev_setup_bars(struct iocpt_dev *dev)
{
	IOCPT_PRINT_CALL();

	dev->name = rte_vdev_device_name(dev->bus_dev);

	return 0;
}

static void
iocpt_vdev_unmap_bars(struct iocpt_dev *dev)
{
	struct iocpt_dev_bars *bars = &dev->bars;
	uint32_t i;

	for (i = 0; i < IOCPT_VDEV_BARS_MAX; i++)
		ionic_uio_rel_rsrc(dev->name, i, &bars->bar[i]);
}

static uint8_t iocpt_vdev_driver_id;
static const struct iocpt_dev_intf iocpt_vdev_intf = {
	.setup_bars = iocpt_vdev_setup_bars,
	.unmap_bars = iocpt_vdev_unmap_bars,
};

static int
iocpt_vdev_probe(struct rte_vdev_device *vdev)
{
	struct iocpt_dev_bars bars = {};
	const char *name = rte_vdev_device_name(vdev);
	unsigned int i;

	IOCPT_PRINT(NOTICE, "Initializing device %s%s", name,
		rte_eal_process_type() == RTE_PROC_SECONDARY ?
			" [SECONDARY]" : "");

	ionic_uio_scan_mcrypt_devices();

	for (i = 0; i < IOCPT_VDEV_BARS_MAX; i++)
		ionic_uio_get_rsrc(name, i, &bars.bar[i]);

	bars.num_bars = IOCPT_VDEV_BARS_MAX;

	return iocpt_probe((void *)vdev, &vdev->device,
			&bars, &iocpt_vdev_intf,
			iocpt_vdev_driver_id, rte_socket_id());
}

static int
iocpt_vdev_remove(struct rte_vdev_device *vdev)
{
	return iocpt_remove(&vdev->device);
}

static struct rte_vdev_driver rte_vdev_iocpt_pmd = {
	.probe = iocpt_vdev_probe,
	.remove = iocpt_vdev_remove,
};

static struct cryptodev_driver rte_vdev_iocpt_drv;

RTE_PMD_REGISTER_VDEV(crypto_ionic, rte_vdev_iocpt_pmd);
RTE_PMD_REGISTER_CRYPTO_DRIVER(rte_vdev_iocpt_drv, rte_vdev_iocpt_pmd.driver,
		iocpt_vdev_driver_id);
