// SPDX-FileCopyrightText: 2019–2025 Andy Curtis <contactandyc@gmail.com>
// SPDX-FileCopyrightText: 2024–2025 Knode.ai — technical questions: contact Andy (above)
// SPDX-License-Identifier: Apache-2.0

#ifndef _aml_block_allocator_H
#define _aml_block_allocator_H

#include "a-memory-library/aml_pool.h"
#include <stdint.h>

struct aml_block_allocator_s;
typedef struct aml_block_allocator_s aml_block_allocator_t;

aml_block_allocator_t *aml_block_allocator_init(aml_pool_t *pool);

static inline uint32_t aml_block_allocator_id(uint32_t size);
static inline uint32_t aml_block_allocator_size(uint32_t id);

void *aml_block_allocator_alloc_by_id(aml_block_allocator_t *h, uint32_t id);
void *aml_block_allocator_alloc(aml_block_allocator_t *h, uint32_t size);

void aml_block_allocator_release(aml_block_allocator_t *h, void *data, uint32_t size);

#include "a-memory-library/extras/impl/aml_block_allocator.h"

#endif
