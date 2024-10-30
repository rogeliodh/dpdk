/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2023 Napatech A/S
 */

#ifndef _FLOW_API_ENGINE_H_
#define _FLOW_API_ENGINE_H_

#include <stdint.h>
#include <stdatomic.h>

#include "hw_mod_backend.h"
#include "stream_binary_flow_api.h"

/*
 * Resource management
 */
#define BIT_CONTAINER_8_ALIGN(x) (((x) + 7) / 8)

/*
 * Resource management
 * These are free resources in FPGA
 * Other FPGA memory lists are linked to one of these
 * and will implicitly follow them
 */
enum res_type_e {
	RES_QUEUE,
	RES_CAT_CFN,
	RES_CAT_COT,
	RES_CAT_EXO,
	RES_CAT_LEN,
	RES_KM_FLOW_TYPE,
	RES_KM_CATEGORY,
	RES_HSH_RCP,
	RES_PDB_RCP,
	RES_QSL_RCP,
	RES_QSL_QST,
	RES_SLC_LR_RCP,

	RES_FLM_FLOW_TYPE,
	RES_FLM_RCP,
	RES_TPE_RCP,
	RES_TPE_EXT,
	RES_TPE_RPL,
	RES_SCRUB_RCP,
	RES_COUNT,
	RES_INVALID
};

/*
 * Flow NIC offload management
 */
#define MAX_OUTPUT_DEST (128)

#define MAX_CPY_WRITERS_SUPPORTED 8

#define MAX_MATCH_FIELDS 16

struct match_elem_s {
	int masked_for_tcam;	/* if potentially selected for TCAM */
	uint32_t e_word[4];
	uint32_t e_mask[4];

	int extr_start_offs_id;
	int8_t rel_offs;
	uint32_t word_len;
};

struct km_flow_def_s {
	struct flow_api_backend_s *be;

	/* For collect flow elements and sorting */
	struct match_elem_s match[MAX_MATCH_FIELDS];
	int num_ftype_elem;

	/* Flow information */
	/* HW input port ID needed for compare. In port must be identical on flow types */
	uint32_t port_id;
};

enum flow_port_type_e {
	PORT_NONE,	/* not defined or drop */
	PORT_INTERNAL,	/* no queues attached */
	PORT_PHY,	/* MAC phy output queue */
	PORT_VIRT,	/* Memory queues to Host */
};

struct output_s {
	uint32_t owning_port_id;/* the port who owns this output destination */
	enum flow_port_type_e type;
	int id;	/* depending on port type: queue ID or physical port id or not used */
	int active;	/* activated */
};

struct nic_flow_def {
	/*
	 * Frame Decoder match info collected
	 */
	int l2_prot;
	int l3_prot;
	int l4_prot;
	int tunnel_prot;
	int tunnel_l3_prot;
	int tunnel_l4_prot;
	int vlans;
	int fragmentation;
	int ip_prot;
	int tunnel_ip_prot;
	/*
	 * Additional meta data for various functions
	 */
	int in_port_override;
	int non_empty;	/* default value is -1; value 1 means flow actions update */
	struct output_s dst_id[MAX_OUTPUT_DEST];/* define the output to use */
	/* total number of available queues defined for all outputs - i.e. number of dst_id's */
	int dst_num_avail;

	/*
	 * Mark or Action info collection
	 */
	uint32_t mark;

	uint32_t jump_to_group;

	int full_offload;

	/*
	 * Modify field
	 */
	struct {
		uint32_t select;
		union {
			uint8_t value8[16];
			uint16_t value16[8];
			uint32_t value32[4];
		};
	} modify_field[MAX_CPY_WRITERS_SUPPORTED];

	uint32_t modify_field_count;

	/*
	 * Key Matcher flow definitions
	 */
	struct km_flow_def_s km;
};

enum flow_handle_type {
	FLOW_HANDLE_TYPE_FLOW,
	FLOW_HANDLE_TYPE_FLM,
};

struct flow_handle {
	enum flow_handle_type type;
	uint32_t flm_id;
	uint16_t caller_id;
	uint16_t learn_ignored;

	struct flow_eth_dev *dev;
	struct flow_handle *next;
	struct flow_handle *prev;

	void *user_data;

	union {
		struct {
			/*
			 * 1st step conversion and validation of flow
			 * verified and converted flow match + actions structure
			 */
			struct nic_flow_def *fd;
			/*
			 * 2nd step NIC HW resource allocation and configuration
			 * NIC resource management structures
			 */
			struct {
				uint32_t db_idx_counter;
				uint32_t db_idxs[RES_COUNT];
			};
			uint32_t port_id;	/* MAC port ID or override of virtual in_port */
		};

		struct {
			uint32_t flm_db_idx_counter;
			uint32_t flm_db_idxs[RES_COUNT];

			uint32_t flm_data[10];
			uint8_t flm_prot;
			uint8_t flm_kid;
			uint8_t flm_prio;
			uint8_t flm_ft;

			uint16_t flm_rpl_ext_ptr;
			uint32_t flm_nat_ipv4;
			uint16_t flm_nat_port;
			uint8_t flm_dscp;
			uint32_t flm_teid;
			uint8_t flm_rqi;
			uint8_t flm_qfi;
		};
	};
};

void km_free_ndev_resource_management(void **handle);

int km_add_match_elem(struct km_flow_def_s *km, uint32_t e_word[4], uint32_t e_mask[4],
	uint32_t word_len, enum frame_offs_e start, int8_t offset);

void kcc_free_ndev_resource_management(void **handle);

/*
 * Group management
 */
int flow_group_handle_create(void **handle, uint32_t group_count);
int flow_group_handle_destroy(void **handle);

int flow_group_translate_get(void *handle, uint8_t owner_id, uint8_t port_id, uint32_t group_in,
	uint32_t *group_out);

#endif  /* _FLOW_API_ENGINE_H_ */
