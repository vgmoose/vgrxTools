/*
 * Copyright (C) 2015 The PASTA Team
 * Originally written by Roxas75
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

#include <wchar.h>
#include <stdio.h>
#include <stdint.h>
#include <memory.h>
#include <svc.h>
#include <emunand.h>
#include <process9.h>
#ifndef PLATFORM_KTR
#include <handlers.h>
#include "hookswi.h"
#include "font.c"
#endif
#include "lib.c"

#include <wchar.h>
#include <stdio.h>
 
#define VRAM (unsigned char*)0x18000000
#define FCRAM (unsigned char*)0x20000000
#define ARM9_RAM (unsigned char*)0x8000000
#define TOP_FRAME 0
#define BOT_FRAME 1
 
unsigned char handle[32];
unsigned char bmpHead[] = {    
        0x42, 0x4D, 0x36, 0x65, 0x04, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0xF0, 0x00,
        0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x65, 0x04, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
 
void transpose (void * dst, const void * src, unsigned dim1, unsigned dim2, unsigned item_length) {
  char * ptr_write;
  const char * ptr_read;
  unsigned x, y, z;
  for (x = 0; x < dim1; x ++) for (y = 0; y < dim2; y ++) {
    ptr_write = ((char *) dst) + item_length * (y * dim1 + x);
    ptr_read = ((const char *) src) + item_length * (x * dim2 + y);
    for (z = 0; z < item_length; z ++) *(ptr_write ++) = *(ptr_read ++);
  }
}
 
void screenShot(int frame, int count){
    unsigned int br = 0;
    short width = frame == 0 ? 400 : 320;
    short height = 240;
    int frameOff = frame == 0 ? 0x300000 : 0; //0x1E6000 : 0x48F000;  //<- Defaults
    int length = frame == 0 ? 0x46500 : 0x38400;
   
	wchar_t tmp[100];
	swprintf(tmp, 100, L"sdmc:/img/%d/capture.bmp", count);
	
//	wchar_t * tmp = L"sdmc:/capture.bmp";
	P9File f;
	p9FileInit(f);
	p9Open(f, tmp, 6);
    transpose(FCRAM+0xF80000, VRAM+frameOff, width, height, 3);
    bmpHead[18] = frame == 0 ? 0x90 : 0x40;
    p9Write(f, &br, bmpHead, 0x36);
    p9Write(f, &br, FCRAM+0xF80000, length);
    p9Close(f);
//    for(int i = 0; i < length; i++) *(VRAM+frameOff + i) = 0xFF;
}


//#define DEBUG_DUMP_RAM              //Uncomment this to enable RAM (fcram+axiwram) dumper


#ifdef DEBUG_DUMP_RAM
static void memdump(wchar_t *filename, unsigned char *buf, size_t size)
{
	unsigned int i;
	P9File f;

	for (i = 0; i < 0x600000; i++)
		((char *)ARM9_VRAM_ADDR)[i] = 0x77;			//Grey flush : Start Dumping

	p9FileInit(f);
	p9Open(f, filename, 6);
	p9Write(f, &i, buf, size);
	p9Close(f);

	for (i = 0; i < 0x600000; i++)
		((char *)ARM9_VRAM_ADDR)[i] = 0xFF;			//White flush : Finished Dumping
}
#endif

#define VER_LEN 4

static void patchLabel()
{
//	static const char verOrig[VER_LEN] = "Ver.";
//	static const char verEmu[VER_LEN] = "VG-E";
//	static const char verSys[VER_LEN] = "VG-S";
//	uintptr_t top, btm;
//	wchar_t *p;
//	const char *src;
//	unsigned int i;
//
//#ifdef PLATFORM_KTR
//	top = 0x27500000;
//	btm = 0x27B00000;
//#else
//	top = 0x23A00000;
//	btm = 0x24000000;
//#endif
//
//	p = (wchar_t *)top;
//	while ((uintptr_t)(p + VER_LEN) < btm) {
//		if (*p == verOrig[0]) {
//			i = 0;
//			while (*p == verOrig[i]) {
//				i++;
//				if (i == VER_LEN) {
//					src = nandSector > 0 ? verEmu : verSys;
//					while (i > 0) {
//						i--;
//						*p = src[i];
//						p--;
//					}
//
//					return;
//				}
//
//				p++;
//			}
//		} else
//			p++;
//	}
}

#ifndef PLATFORM_KTR 
	#ifndef DEBUG_DUMP_RAM

#define FB_HEIGHT 240
#define FB_WIDTH 320
#define FNT_HEIGHT 8
#define FNT_WIDTH 8

static int cur_x;
static int cur_y;

static void initScr()
{
	cur_x = 0;
	cur_y = FB_HEIGHT;
}

static void scrPutc(int c)
{
	struct px {
		unsigned char b;
		unsigned char g;
		unsigned char r;
	};

	struct px (* const fb)[FB_HEIGHT] = (void *)ARM9_VRAM_ADDR;
	unsigned int fnt_x, fnt_y;
	const unsigned char *p;

	switch(c) {
		case '\t':
			cur_x += FNT_WIDTH * 4;
			break;

		case '\n':
			cur_x = 0;
			cur_y -= FNT_HEIGHT;
			if (cur_y < 0)
				cur_y = FB_HEIGHT;
			break;

		default:
			p = font + (c - 32) * FNT_WIDTH;

			for (fnt_x = 0; fnt_x < FNT_WIDTH; fnt_x++) {
				for (fnt_y = 0; fnt_y < FNT_HEIGHT; fnt_y++) {
					if ((0x80 >> fnt_y) & *p) {
						fb[cur_x][cur_y - fnt_y].r = 255;
						fb[cur_x][cur_y - fnt_y].g = 255;
						fb[cur_x][cur_y - fnt_y].b = 255;
					} else {
						fb[cur_x][cur_y - fnt_y].r = 0;
						fb[cur_x][cur_y - fnt_y].g = 0;
						fb[cur_x][cur_y - fnt_y].b = 0;
					}
				}

				p++;
				cur_x++;
			}
	}
}

static void scrPuts(const char *s)
{
	while (*s) {
		scrPutc(*s);
		s++;
	}
}

static void scrItoa(unsigned val, int base, int w)
{
        char buf[10];
        int i = 0;
        int rem;

        do {
                rem = val % base;
                buf[i++] = (rem < 10 ? 0x30 : 0x37) + rem;
                val /= base;
                w--;
        } while (val || w > 0);

        while (i > 0)
                scrPutc(buf[--i]);
}

static int getArmBoff(void *p)
{
	int i;

	i = *(int *)p & 0xFFFFFF;
	if (i & 0x800000)
		i -= 0x1000000;

	return i << 2;
}

static void writeArmB(void *cur, const void *dst)
{
	*(int *)cur = (0xE << 28) | (0x5 << 25) | (0x0 << 24)
		| (((uintptr_t)dst - ((uintptr_t)cur + 8)) >> 2);
}

#define REG_NUM 17

_Noreturn void mainHandler(int regs[REG_NUM], const char *type)
{
	static const char regNames[REG_NUM][4] = {
		"R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7",
		"R8", "R9", "R10", "R11", "R12", "SP", "LR", "PC", "CPSR"
	};
	void * const arm11Swi = (void *)0x1FFF4008;
	unsigned int * const arm11Code = (void *)0x1FFF4B20;
	unsigned int *p, *q;
	unsigned int i, origCpsr;

	// Back up CPSR
	__asm__ ("mrs %0, cpsr" : "=r"(origCpsr));

	while (1) {
		// Change the mode to supervisor
		i = (origCpsr & ~0xF) | 0x13;
		__asm__ volatile ("msr cpsr_c, %0" :: "r"(i));

		// Allow to write to ARM11 exception vectors
		i = 0x10000037;
		__asm__ volatile ("mcr p15, 0, %0, c6, c3, 0" :: "r"(i));

		// Restore CPSR
		__asm__ volatile ("msr cpsr_c, %0" :: "r"(origCpsr));

		p = arm11Code;
		for (q = hookSwi; q != hookSwiBtm; q++) {
			*p = *q;
			p++;
		}

		writeArmB(arm11Swi, arm11Code);

		initScr();
		scrPuts("\nException Information by rxTools\n\n\n\n"
			"CPU: ARM9\tType: ");
		scrPuts(type);

		scrPuts("\n\n\nRegisters:\n");

		for (i = 0; i < REG_NUM; i++) {
			scrPutc(i % 2 ? '\t' : '\n');
			scrPuts(regNames[i]);
			scrPuts(": 0x");
			scrItoa(regs[i], 16, 8);
		}
	}
}

static void initExceptionHandler()
{
	*(void **)0x08000008 = handleFiq;
	*(void **)0x08000018 = handleInstr;
	*(void **)0x0800002C = handleData;
	*(void **)0x08000020 = handlePrefetch;
}
	#endif
#endif

_Noreturn void thread()
{

#ifndef PLATFORM_KTR
	#ifndef DEBUG_DUMP_RAM
		initExceptionHandler();
	#endif
#endif

	int frames = 0;
	int recording_active = 0;
	while (1) {

#ifdef DEBUG_DUMP_RAM
		if(getHID() & BUTTON_SELECT){
			memdump(L"sdmc:/FCRAM.bin", (void *)0x20000000, 0x10000);
			memdump(L"sdmc:/AXIWRAM.bin", (void *)0x1FF80000, 0x00080000);
		}
#endif

#ifdef DEBUG_DATA_ABORT
		if (getHID() & BUTTON_START)
			*(int *)8192 = 100;
#endif
		
		if (getHID() & BUTTON_Y)
			recording_active = 27;
		
		if ((getHID() & BUTTON_X) && (getHID() & BUTTON_Y))
			recording_active = 0;

		patchLabel();
		if (recording_active == 27) {
			frames++;
			screenShot(0, frames);
		}
	}
}
