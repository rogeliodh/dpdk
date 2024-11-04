..  SPDX-License-Identifier: BSD-3-Clause
    Copyright(c) 2024 Marvell.

Marvell CNXK RVU LF Driver
==========================

CNXK product families can have a use case to allow RVU PF and RVU VF
driver to communicate using mailboxes and also get notified
of any interrupt that may occur on the device.
Hence, a new raw device driver is added for such RVU LF devices.
These devices can map to a RVU PF or a RVU VF which
can send mailboxes to each other.

Features
--------

The RVU LF device implements following features in the rawdev API:

- Get PF FUNC of associated NPA and SSO devices.
- Get PF FUNC for RVU LF device.
- Register/unregister interrupt handlers.
- Register/unregister mailbox callbacks for the other side to process mailboxes.
- Set mailbox message ID range to be used by the driver.
- Process mailbox messages.

Limitations
-----------

In multi-process mode user-space application must ensure
no resources sharing takes place.
Otherwise, user-space application should ensure synchronization.

Device Setup
------------

The RVU LF devices will need to be bound to a user-space IO driver for use.
The script ``dpdk-devbind.py`` included with DPDK can be used to
view the state of the devices and to bind them to a suitable DPDK-supported
kernel driver. When querying the status of the devices, they will appear under
the category of "Misc (rawdev) devices", i.e. the command
``dpdk-devbind.py --status-dev misc`` can be used to see the state of those
devices alone.

Get NPA and SSO PF FUNC
-----------------------

APIs ``rte_pmd_rvu_lf_npa_pf_func_get()`` and ``rte_pmd_rvu_lf_sso_pf_func_get()``
can be used to get the cnxk NPA PF func and SSO PF func which application
can use for NPA/SSO specific configuration.

Get RVU LF PF FUNC
------------------

APIs ``rte_pmd_rvu_lf_pf_func_get()`` is added to get the RVU LF device PF FUNC.

Register or remove interrupt handler
------------------------------------

Out of tree drivers can register interrupt handlers using ``rte_pmd_rvu_lf_irq_register()``
or remove interrupt handler using ``rte_pmd_rvu_lf_irq_unregister()``.
The irq numbers for which the interrupts are registered is negotiated separately
and is not in scope of the driver.

RVU LF RAW MESSAGE PROCESSING
-----------------------------

For processing of mailboxes received on RVU PF/VF application, out of tree
drivers can register/unregister callbacks using ``rte_pmd_rvu_lf_msg_handler_register()``
and ``rte_pmd_rvu_lf_msg_handler_unregister()``.
Required responses as per the request and message id received can be filled
in the callbacks.

Once a RVU LF raw device is probed, a range of message ids can be configured for
which mailboxes will be sent using the API ``rte_pmd_rvu_lf_msg_id_range_set()``.

The out of tree driver can call ``rte_pmd_rvu_lf_msg_process()`` to send a request and
receive corresponding response from the other side(PF/VF).
It accepts an opaque pointer of a request and its size which can be defined by user
and provides an opaque pointer for a response and its length.
PF and VF out of tree driver can define its own request and response based on the message id
of the mailbox.
