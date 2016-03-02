/*
 * Copyright (C) 2015 The PASTA Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#define AES_BIG_INPUT      1
#define AES_LITTLE_INPUT   0
#define AES_NORMAL_INPUT   4
#define AES_REVERSED_INPUT 0
#define AES_BLOCK_SIZE 0x10

#define AES_CCM_DECRYPT_MODE (0 << 27)
#define AES_CCM_ENCRYPT_MODE (1 << 27)
#define AES_CTR_MODE         (2 << 27)
#define AES_CTR_MODE         (2 << 27)
#define AES_CBC_DECRYPT_MODE (4 << 27)
#define AES_CBC_ENCRYPT_MODE (5 << 27)
#define AES_ECB_DECRYPT_MODE (6 << 27)
#define AES_ECB_ENCRYPT_MODE (7 << 27)

#define REG_AESCNT     ((volatile uint32_t*)0x10009000)
#define REG_AESBLKCNT  ((volatile uint32_t*)0x10009004)
#define REG_AESWRFIFO  ((volatile uint32_t*)0x10009008)
#define REG_AESRDFIFO  ((volatile uint32_t*)0x1000900C)
#define REG_AESKEYSEL  ((volatile uint8_t *)0x10009010)
#define REG_AESKEYCNT  ((volatile uint8_t *)0x10009011)
#define REG_AESCTR     ((volatile uint32_t*)0x10009020)
#define REG_AESKEYFIFO ((volatile uint32_t*)0x10009108)
#define REG_AESKEYXFIFO ((volatile uint32_t*)0x10009104)

#define AES_CNT_START         0x80000000
#define AES_CNT_INPUT_ORDER   0x02000000
#define AES_CNT_OUTPUT_ORDER  0x01000000
#define AES_CNT_INPUT_ENDIAN  0x00800000
#define AES_CNT_OUTPUT_ENDIAN 0x00400000
#define AES_CNT_FLUSH_READ    0x00000800
#define AES_CNT_FLUSH_WRITE   0x00000400

void aesSetKeyX(uint8_t keyslot, void* keyx);
void aesSetKey(uint32_t keyno, int value, void* key);
void aesSelKey(uint32_t keyno);
void aesSetCtr(void* iv);
void aesAddCtr(void* ctr, uint32_t carry);
void aesDecrypt(void *dst, const void *src, uint16_t blocks, uint32_t mode);
void aesCtr(void *dst, const void *src, size_t n, void *ctr);
