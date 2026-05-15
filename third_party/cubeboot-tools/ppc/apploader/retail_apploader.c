/**
 * retail_apploader.c
 *
 * Simple clean-room apploader enabling booting DOLs from retail-style
 * GameCube disc layouts.
 *
 * Derived from the public cubeboot-tools apploader structure and adapted for
 * direct GameCube disc boot instead of El Torito ISO9660 boot catalogs.
 *
 * Copyright (C) 2005-2006 The GameCube Linux Team
 * Copyright (C) 2005,2006 Albert Herranz
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 */

#include <stddef.h>

#include "../include/system.h"

#include "../../include/gcm.h"
#include "../../include/dol.h"

#define DI_ALIGN_SHIFT	5
#define DI_ALIGN_SIZE	(1UL << DI_ALIGN_SHIFT)
#define DI_ALIGN_MASK	(~((1 << DI_ALIGN_SHIFT) - 1))

#define di_align(addr)	(void *) \
			((((unsigned long)(addr)) + \
				 DI_ALIGN_SIZE - 1) & DI_ALIGN_MASK)

struct dolphin_debugger_info {
	uint32_t		present;
	uint32_t		exception_mask;
	uint32_t		exception_hook_address;
	uint32_t		saved_lr;
	unsigned char		__pad1[0x10];
} __attribute__ ((__packed__));

struct dolphin_lowmem {
	struct gcm_disk_info	b_disk_info;

	uint32_t		a_boot_magic;
	uint32_t		a_version;

	uint32_t		b_physical_memory_size;
	uint32_t		b_console_type;

	uint32_t		a_arena_lo;
	uint32_t		a_arena_hi;
	void			*a_fst;
	uint32_t		a_fst_max_size;

	struct dolphin_debugger_info a_debugger_info;
	unsigned char		hook_code[0x60];

	uint32_t		o_current_os_context_phys;
	uint32_t		o_previous_os_interrupt_mask;
	uint32_t		o_current_os_interrupt_mask;

	uint32_t		tv_mode;
	uint32_t		b_aram_size;

	void			*o_current_os_context;
	void			*o_default_os_thread;
	void			*o_thread_queue_head;
	void			*o_thread_queue_tail;
	void			*o_current_os_thread;

	uint32_t		a_debug_monitor_size;
	void			*a_debug_monitor;

	uint32_t		a_simulated_memory_size;

	void			*a_bi2;

	uint32_t		b_bus_clock_speed;
	uint32_t		b_cpu_clock_speed;
} __attribute__ ((__packed__));

static void al_enter(void (*report) (char *text, ...));
static int al_load(void **address, uint32_t *length, uint32_t *offset);
static void *al_exit(void);

struct retail_apploader_control {
	unsigned		step;
	uint32_t		dol_offset;
	uint32_t		fst_offset;
	uint32_t		fst_size;
	unsigned long		fst_address;
	unsigned long		bi2_address;
	void			*entry_point;
	uint32_t		sects_bitmap;
	uint32_t		all_sects_bitmap;
	void			(*report) (char *text, ...);
};

static struct dolphin_lowmem *lowmem =
			 (struct dolphin_lowmem *)0x80000000;

static struct retail_apploader_control al_control = {
	.sects_bitmap = 0xffffffff
};

static unsigned char di_buffer[0x3000] __attribute__ ((aligned(32))) =
	"www.gc-linux.org";

/*
 * This is our particular "main".
 * It _must_ be the first function in this file.
 */
void al_start(void **enter, void **load, void **exit)
{
	al_control.step = 0;

	*enter = al_enter;
	*load = al_load;
	*exit = al_exit;
}

static int al_load_dol_sects_bitmap(struct dol_header *h)
{
	int i, sects_bitmap;

	sects_bitmap = 0;
	for (i = 0; i < DOL_MAX_SECT; i++) {
		if (dol_sect_size(h, i) == 0)
			continue;

		sects_bitmap |= (1 << i);
	}
	return sects_bitmap;
}

static void al_check_dol(struct dol_header *h)
{
	int i, valid = 0;
	uint32_t value;

	for (i = 0; i < DOL_MAX_SECT; i++) {
		if ((dol_sect_offset(h, i) != 0)
		    && (dol_sect_offset(h, i) < DOL_HEADER_SIZE)) {
			panic("detected segment offset within DOL header\n");
		}

		value = dol_sect_offset(h, i);
		if (value != (uint32_t) di_align(value)) {
			panic("detected unaligned section offset\n");
		}

		value = dol_sect_address(h, i);
		if (value != (uint32_t) di_align(value)) {
			panic("unaligned section address\n");
		}

		if (dol_sect_address(h, i) != 0) {
			if (!(dol_sect_address(h, i) & 0x80000000)) {
				panic("segment below 2GB\n");
			}
			if (dol_sect_address(h, i) > 0x81200000) {
				panic("segment above 0x81200000\n");
			}
		}

		if (i < DOL_SECT_MAX_TEXT) {
			if (h->entry_point >= dol_sect_address(h, i)
			    && h->entry_point < dol_sect_address(h, i) +
			    dol_sect_size(h, i))
				valid = 1;
		}
	}

	if (h->address_bss != 0 && !(h->address_bss & 0x80000000)) {
		panic("BSS segment below 2GB\n");
	}

	if (!valid) {
		panic("entry point out of text segment\n");
	}
}

static void al_enter(void (*report) (char *text, ...))
{
	al_control.step = 1;
	al_control.report = report;
	if (report)
		report("retail gc apploader\n");
}

static int al_load(void **address, uint32_t *length, uint32_t *offset)
{
	struct gcm_disk_header *disk_header;
	struct gcm_disk_header_info *disk_header_info;
	struct dol_header *dh;
	unsigned long lowest_start;
	int j, k;
	int need_more = 1;

	if (al_control.report)
		al_control.report("step %d\n", al_control.step);

	switch (al_control.step) {
	case 0:
	case 1:
		al_control.step = 1;

		*address = di_buffer;
		*length = (uint32_t) di_align(sizeof(*disk_header) + sizeof(*disk_header_info));
		*offset = 0;
		invalidate_dcache_range(*address, *address + *length);

		al_control.step++;
		break;
	case 2:
		disk_header = (struct gcm_disk_header *)di_buffer;

		if (disk_header->info.magic != GCM_MAGIC) {
			panic("invalid gamecube disc magic\n");
		}

		al_control.dol_offset = disk_header->layout.dol_offset;
		al_control.fst_offset = disk_header->layout.fst_offset;
		al_control.fst_size = disk_header->layout.fst_size;

		if (al_control.dol_offset < SYSTEM_AREA_SIZE) {
			panic("invalid dol offset\n");
		}

		*address = di_buffer;
		*length = DOL_HEADER_SIZE;
		*offset = al_control.dol_offset;
		invalidate_dcache_range(*address, *address + *length);

		al_control.sects_bitmap = 0xffffffff;
		al_control.step++;
		break;
	case 3:
		dh = (struct dol_header *)di_buffer;

		if (al_control.sects_bitmap == 0xffffffff) {
			al_check_dol(dh);
			al_control.entry_point = (void *)dh->entry_point;
			al_control.sects_bitmap = 0;
			al_control.all_sects_bitmap = al_load_dol_sects_bitmap(dh);
		}

		lowest_start = 0xffffffff;
		for (j = -1, k = 0; k < DOL_MAX_SECT; k++) {
			if ((al_control.sects_bitmap & (1 << k)) != 0)
				continue;
			if (!(al_control.all_sects_bitmap & (1 << k)))
				continue;
			if (dol_sect_address(dh, k) < lowest_start) {
				lowest_start = dol_sect_address(dh, k);
				j = k;
			}
		}

		al_control.sects_bitmap |= (1 << j);

		*address = (void *)dol_sect_address(dh, j);
		*length = (uint32_t) di_align(dol_sect_size(dh, j));
		*offset = al_control.dol_offset + dol_sect_offset(dh, j);

		invalidate_dcache_range(*address, *address + *length);
		if (dol_sect_is_text(dh, j))
			invalidate_icache_range(*address, *address + *length);

		if (al_control.sects_bitmap == al_control.all_sects_bitmap) {
			if (dh->size_bss)
				memset((void *)dh->address_bss, 0, dh->size_bss);

			al_control.step++;
		}
		break;
	case 4:
		if (al_control.fst_size == 0) {
			panic("invalid fst size\n");
		}

		al_control.fst_address = (0x81800000 - al_control.fst_size) & DI_ALIGN_MASK;

		*address = (void *)al_control.fst_address;
		*length = (uint32_t) di_align(al_control.fst_size);
		*offset = al_control.fst_offset;
		invalidate_dcache_range(*address, *address + *length);

		al_control.step++;
		break;
	case 5:
		al_control.bi2_address = al_control.fst_address - 0x2000;

		*address = (void *)al_control.bi2_address;
		*length = 0x2000;
		*offset = 0x440;
		invalidate_dcache_range(*address, *address + *length);

		al_control.step++;
		break;
	case 6:
		disk_header_info =
			(struct gcm_disk_header_info *)(void *)al_control.bi2_address;

		lowmem->a_boot_magic = 0x0d15ea5e;
		lowmem->a_version = 1;
		lowmem->a_arena_hi = al_control.fst_address;
		lowmem->a_fst = (void *)al_control.fst_address;
		lowmem->a_fst_max_size = al_control.fst_size;
		lowmem->a_debug_monitor = (void *)0x81800000;
		lowmem->a_simulated_memory_size = disk_header_info->simulated_memory_size;
		lowmem->a_bi2 = (void *)al_control.bi2_address;
		flush_dcache_range(lowmem, lowmem + 1);

		*length = 0;
		need_more = 0;
		al_control.step++;
		break;
	default:
		al_control.step++;
		break;
	}

	return need_more;
}

static void *al_exit(void)
{
	return al_control.entry_point;
}
