// SPDX-FileCopyrightText: 2019–2025 Andy Curtis <contactandyc@gmail.com>
// SPDX-FileCopyrightText: 2024–2025 Knode.ai — technical questions: contact Andy (above)
// SPDX-License-Identifier: Apache-2.0

#ifndef _aml_block_allocator_impl_H
#define _aml_block_allocator_impl_H

#pragma pack(push)
#pragma pack(2)
typedef struct
{
    uint16_t type : 1;
    uint16_t group : 15;
    uint8_t *block;
} _aml_block_allocator_node_t;
#pragma pack(pop)

static const uint32_t _aml_block_allocator_tbl[] = {
    0 * sizeof(_aml_block_allocator_node_t),
    1 * sizeof(_aml_block_allocator_node_t),
    2 * sizeof(_aml_block_allocator_node_t),
    3 * sizeof(_aml_block_allocator_node_t),
    4 * sizeof(_aml_block_allocator_node_t),
    5 * sizeof(_aml_block_allocator_node_t),
    6 * sizeof(_aml_block_allocator_node_t),
    7 * sizeof(_aml_block_allocator_node_t),
    12 * sizeof(_aml_block_allocator_node_t),
    24 * sizeof(_aml_block_allocator_node_t),
    32 * sizeof(_aml_block_allocator_node_t),
    48 * sizeof(_aml_block_allocator_node_t),
    64 * sizeof(_aml_block_allocator_node_t),
    96 * sizeof(_aml_block_allocator_node_t),
    128 * sizeof(_aml_block_allocator_node_t),
    196 * sizeof(_aml_block_allocator_node_t),
    256 * sizeof(_aml_block_allocator_node_t),
    512 * sizeof(_aml_block_allocator_node_t),
    512 * 2 * sizeof(_aml_block_allocator_node_t),
    512 * 4 * sizeof(_aml_block_allocator_node_t),
    512 * 6 * sizeof(_aml_block_allocator_node_t),
    512 * 8 * sizeof(_aml_block_allocator_node_t),
    2048 * 6 * sizeof(_aml_block_allocator_node_t),
    2048 * 8 * sizeof(_aml_block_allocator_node_t),
    2048 * 12 * sizeof(_aml_block_allocator_node_t),
    2048 * 16 * sizeof(_aml_block_allocator_node_t),
    2048 * 24 * sizeof(_aml_block_allocator_node_t),
    2048 * 32 * sizeof(_aml_block_allocator_node_t),
    2048 * 48 * sizeof(_aml_block_allocator_node_t),
    65536 * 2 * sizeof(_aml_block_allocator_node_t),
    65536 * 3 * sizeof(_aml_block_allocator_node_t),
    65536 * 4 * sizeof(_aml_block_allocator_node_t),
    65536 * 8 * sizeof(_aml_block_allocator_node_t),
    65536 * 12 * sizeof(_aml_block_allocator_node_t),
    65536 * 16 * sizeof(_aml_block_allocator_node_t),
    65536 * 24 * sizeof(_aml_block_allocator_node_t),
    65536 * 32 * sizeof(_aml_block_allocator_node_t),
    65536 * 48 * sizeof(_aml_block_allocator_node_t),
    65536 * 64 * sizeof(_aml_block_allocator_node_t),
    65536 * 96 * sizeof(_aml_block_allocator_node_t),
    65536 * 128 * sizeof(_aml_block_allocator_node_t),
    65536 * 196 * sizeof(_aml_block_allocator_node_t),
    65536 * 256 * sizeof(_aml_block_allocator_node_t)
};

static inline uint32_t aml_block_allocator_size(uint32_t id) {
    return _aml_block_allocator_tbl[id];
}

static inline uint32_t aml_block_allocator_id(uint32_t size)
{
    if (size <= _aml_block_allocator_tbl[16])
    {
        if (size <= _aml_block_allocator_tbl[8])
        {
            if (size <= _aml_block_allocator_tbl[4])
            {
                if (size <= _aml_block_allocator_tbl[2])
                {
                    if (size <= _aml_block_allocator_tbl[0])
                        return 0;
                    else if (size <= _aml_block_allocator_tbl[1])
                        return 1;
                    else
                        return 2;
                }
                else
                {
                    if (size <= _aml_block_allocator_tbl[3])
                        return 3;
                    else
                        return 4;
                }
            }
            else
            {
                if (size <= _aml_block_allocator_tbl[6])
                {
                    if (size <= _aml_block_allocator_tbl[5])
                        return 5;
                    else
                        return 6;
                }
                else
                {
                    if (size <= _aml_block_allocator_tbl[7])
                        return 7;
                    else
                        return 8;
                }
            }
        }
        else if (size <= _aml_block_allocator_tbl[12])
        {
            if (size <= _aml_block_allocator_tbl[10])
            {
                if (size <= _aml_block_allocator_tbl[9])
                    return 9;
                else
                    return 10;
            }
            else
            {
                if (size <= _aml_block_allocator_tbl[11])
                    return 11;
                else
                    return 12;
            }
        }
        else
        {
            if (size <= _aml_block_allocator_tbl[14])
            {
                if (size <= _aml_block_allocator_tbl[13])
                    return 13;
                else
                    return 14;
            }
            else
            {
                if (size <= _aml_block_allocator_tbl[15])
                    return 15;
                else
                    return 16;
            }
        }
    }
    else if (size <= _aml_block_allocator_tbl[32])
    {
        if (size <= _aml_block_allocator_tbl[24])
        {
            if (size <= _aml_block_allocator_tbl[20])
            {
                if (size <= _aml_block_allocator_tbl[18])
                {
                    if (size <= _aml_block_allocator_tbl[17])
                        return 17;
                    else
                        return 18;
                }
                else
                {
                    if (size <= _aml_block_allocator_tbl[19])
                        return 19;
                    else
                        return 20;
                }
            }
            else
            {
                if (size <= _aml_block_allocator_tbl[22])
                {
                    if (size <= _aml_block_allocator_tbl[21])
                        return 21;
                    else
                        return 22;
                }
                else
                {
                    if (size <= _aml_block_allocator_tbl[23])
                        return 23;
                    else
                        return 24;
                }
            }
        }
        else if (size <= _aml_block_allocator_tbl[28])
        {
            if (size <= _aml_block_allocator_tbl[26])
            {
                if (size <= _aml_block_allocator_tbl[25])
                    return 25;
                else
                    return 26;
            }
            else
            {
                if (size <= _aml_block_allocator_tbl[27])
                    return 27;
                else
                    return 28;
            }
        }
        else
        {
            if (size <= _aml_block_allocator_tbl[30])
            {
                if (size <= _aml_block_allocator_tbl[29])
                    return 29;
                else
                    return 30;
            }
            else
            {
                if (size <= _aml_block_allocator_tbl[31])
                    return 31;
                else
                    return 32;
            }
        }
    }
    else if (size <= _aml_block_allocator_tbl[33]) // all of these are increasingly unlikely, so scanning is probably most efficient
        return 33;
    else if (size <= _aml_block_allocator_tbl[34])
        return 34;
    else if (size <= _aml_block_allocator_tbl[35])
        return 35;
    else if (size <= _aml_block_allocator_tbl[36])
        return 36;
    else if (size <= _aml_block_allocator_tbl[37])
        return 37;
    else if (size <= _aml_block_allocator_tbl[38])
        return 38;
    else if (size <= _aml_block_allocator_tbl[39])
        return 39;
    else if (size <= _aml_block_allocator_tbl[40])
        return 40;
    else if (size <= _aml_block_allocator_tbl[41])
        return 41;
    else if (size <= _aml_block_allocator_tbl[42])
        return 42;
    else
        abort();
}

struct aml_block_allocator_free_node_s;
struct aml_block_allocator_free_head_s;
typedef struct aml_block_allocator_free_node_s aml_block_allocator_free_node_t;
typedef struct aml_block_allocator_free_head_s aml_block_allocator_free_head_t;

struct aml_block_allocator_free_node_s
{
    aml_block_allocator_free_node_t *next;
};

struct aml_block_allocator_free_head_s
{
    aml_block_allocator_free_head_t *next;
};

struct aml_block_allocator_s {
    aml_pool_t *pool;
    aml_block_allocator_free_head_t *free_list;
};

aml_block_allocator_t *aml_block_allocator_init(aml_pool_t *pool) {
    aml_block_allocator_t *h =
        (aml_block_allocator_t*)aml_pool_zalloc(pool, sizeof(*h) + (sizeof(aml_block_allocator_free_head_t)*44));
    h->pool = pool;
    h->free_list = (aml_block_allocator_free_head_t *)(h+1);
    return h;
}

static inline
void *aml_block_allocator_alloc_by_id(aml_block_allocator_t *h, uint32_t id) {
    if(h->free_list[id].next) {
        return (void *)aml_pool_alloc(h->pool, _aml_block_allocator_tbl[id]);
    } else {
        aml_block_allocator_free_node_t *fn = h->free_list[id].next;
        h->free_list[id].next = fn->next;
        return (void *)fn;
    }
}

static inline
void *aml_block_allocator_alloc(aml_block_allocator_t *h, uint32_t size) {
    if(!size)
        return NULL;

    uint32_t id = aml_block_allocator_id(size);
    return aml_block_allocator_alloc_by_id(h, id);
}

static inline
void aml_block_allocator_release(aml_block_allocator_t *h, void *data, uint32_t size) {
    if(!data) return;

    uint32_t id = aml_block_allocator_id(size);
    aml_block_allocator_free_node_t *fn = (aml_block_allocator_free_node_t *)data;
    fn->next = h->free_list[id].next;
    h->free_list[id].next = fn;
}

#endif
