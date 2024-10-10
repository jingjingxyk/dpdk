/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2023 Napatech A/S
 */

#ifndef __NTOSS_VIRT_QUEUE_H__
#define __NTOSS_VIRT_QUEUE_H__

#include <stdint.h>
#include <stdalign.h>

#include <rte_memory.h>

struct nthw_virt_queue;

#define SPLIT_RING        0
#define PACKED_RING       1
#define IN_ORDER          1

/*
 * SPLIT : This marks a buffer as continuing via the next field.
 * PACKED: This marks a buffer as continuing. (packed does not have a next field, so must be
 * contiguous) In Used descriptors it must be ignored
 */
#define VIRTQ_DESC_F_NEXT 1

/*
 * Split Ring virtq Descriptor
 */
struct __rte_aligned(8) virtq_desc {
	/* Address (guest-physical). */
	uint64_t addr;
	/* Length. */
	uint32_t len;
	/* The flags as indicated above. */
	uint16_t flags;
	/* Next field if flags & NEXT */
	uint16_t next;
};

/* descr phys address must be 16 byte aligned */
struct __rte_aligned(16) pvirtq_desc {
	/* Buffer Address. */
	uint64_t addr;
	/* Buffer Length. */
	uint32_t len;
	/* Buffer ID. */
	uint16_t id;
	/* The flags depending on descriptor type. */
	uint16_t flags;
};

/*
 * Common virtq descr
 */
#define vq_set_next(vq, index, nxt) \
do { \
	struct nthw_cvirtq_desc *temp_vq = (vq); \
	if (temp_vq->vq_type == SPLIT_RING) \
		temp_vq->s[index].next = nxt; \
} while (0)

#define vq_add_flags(vq, index, flgs) \
do { \
	struct nthw_cvirtq_desc *temp_vq = (vq); \
	uint16_t tmp_index = (index); \
	typeof(flgs) tmp_flgs = (flgs); \
	if (temp_vq->vq_type == SPLIT_RING) \
		temp_vq->s[tmp_index].flags |= tmp_flgs; \
	else if (temp_vq->vq_type == PACKED_RING) \
		temp_vq->p[tmp_index].flags |= tmp_flgs; \
} while (0)

#define vq_set_flags(vq, index, flgs) \
do { \
	struct nthw_cvirtq_desc *temp_vq = (vq); \
	uint32_t temp_flags = (flgs); \
	uint32_t temp_index = (index); \
	if ((temp_vq)->vq_type == SPLIT_RING) \
		(temp_vq)->s[temp_index].flags = temp_flags; \
	else if ((temp_vq)->vq_type == PACKED_RING) \
		(temp_vq)->p[temp_index].flags = temp_flags; \
} while (0)

struct nthw_virtq_desc_buf {
	/* Address (guest-physical). */
	alignas(16) uint64_t addr;
	/* Length. */
	uint32_t len;
};

struct nthw_cvirtq_desc {
	union {
		struct nthw_virtq_desc_buf *b;  /* buffer part as is common */
		struct virtq_desc     *s;  /* SPLIT */
		struct pvirtq_desc    *p;  /* PACKED */
	};
	uint16_t vq_type;
};

struct nthw_received_packets {
	void *addr;
	uint32_t len;
};

#endif /* __NTOSS_VIRT_QUEUE_H__ */
