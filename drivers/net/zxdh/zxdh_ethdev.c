/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2024 ZTE Corporation
 */

#include <ethdev_pci.h>
#include <bus_pci_driver.h>
#include <rte_ethdev.h>

#include "zxdh_ethdev.h"

static int
zxdh_eth_dev_init(struct rte_eth_dev *eth_dev)
{
	struct rte_pci_device *pci_dev = RTE_ETH_DEV_TO_PCI(eth_dev);
	struct zxdh_hw *hw = eth_dev->data->dev_private;
	int ret = 0;

	eth_dev->dev_ops = NULL;

	/* Allocate memory for storing MAC addresses */
	eth_dev->data->mac_addrs = rte_zmalloc("zxdh_mac",
			ZXDH_MAX_MAC_ADDRS * RTE_ETHER_ADDR_LEN, 0);
	if (eth_dev->data->mac_addrs == NULL)
		return -ENOMEM;

	memset(hw, 0, sizeof(*hw));
	hw->bar_addr[0] = (uint64_t)pci_dev->mem_resource[0].addr;
	if (hw->bar_addr[0] == 0)
		return -EIO;

	hw->device_id = pci_dev->id.device_id;
	hw->port_id = eth_dev->data->port_id;
	hw->eth_dev = eth_dev;
	hw->speed = RTE_ETH_SPEED_NUM_UNKNOWN;
	hw->duplex = RTE_ETH_LINK_FULL_DUPLEX;
	hw->is_pf = 0;

	if (pci_dev->id.device_id == ZXDH_E310_PF_DEVICEID ||
		pci_dev->id.device_id == ZXDH_E312_PF_DEVICEID) {
		hw->is_pf = 1;
	}

	return ret;
}

static int
zxdh_eth_pci_probe(struct rte_pci_driver *pci_drv __rte_unused,
		struct rte_pci_device *pci_dev)
{
	return rte_eth_dev_pci_generic_probe(pci_dev,
						sizeof(struct zxdh_hw),
						zxdh_eth_dev_init);
}

static int
zxdh_dev_close(struct rte_eth_dev *dev __rte_unused)
{
	int ret = 0;

	return ret;
}

static int
zxdh_eth_dev_uninit(struct rte_eth_dev *eth_dev)
{
	int ret = 0;

	ret = zxdh_dev_close(eth_dev);

	return ret;
}

static int
zxdh_eth_pci_remove(struct rte_pci_device *pci_dev)
{
	int ret = rte_eth_dev_pci_generic_remove(pci_dev, zxdh_eth_dev_uninit);

	return ret;
}

static const struct rte_pci_id pci_id_zxdh_map[] = {
	{RTE_PCI_DEVICE(ZXDH_PCI_VENDOR_ID, ZXDH_E310_PF_DEVICEID)},
	{RTE_PCI_DEVICE(ZXDH_PCI_VENDOR_ID, ZXDH_E310_VF_DEVICEID)},
	{RTE_PCI_DEVICE(ZXDH_PCI_VENDOR_ID, ZXDH_E312_PF_DEVICEID)},
	{RTE_PCI_DEVICE(ZXDH_PCI_VENDOR_ID, ZXDH_E312_VF_DEVICEID)},
	{.vendor_id = 0, /* sentinel */ },
};
static struct rte_pci_driver zxdh_pmd = {
	.id_table = pci_id_zxdh_map,
	.drv_flags = RTE_PCI_DRV_NEED_MAPPING | RTE_PCI_DRV_INTR_LSC,
	.probe = zxdh_eth_pci_probe,
	.remove = zxdh_eth_pci_remove,
};

RTE_PMD_REGISTER_PCI(net_zxdh, zxdh_pmd);
RTE_PMD_REGISTER_PCI_TABLE(net_zxdh, pci_id_zxdh_map);
RTE_PMD_REGISTER_KMOD_DEP(net_zxdh, "* vfio-pci");
