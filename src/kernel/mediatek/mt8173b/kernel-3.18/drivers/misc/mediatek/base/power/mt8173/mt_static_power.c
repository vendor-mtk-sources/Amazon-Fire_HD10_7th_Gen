/*
 * Copyright (C) 2016 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#if defined(__KERNEL__)		/* || !defined (__CTP__) */
#include <linux/export.h>
#include <linux/module.h>
#endif				/* #if !defined (__CTP__) */

#if 0				/* L318_Need_Related_File */
#include <asm/system.h>
#endif				/* L318_Need_Related_File */
#include "mt_spower_data.h"
#include "mt_static_power.h"

/*********************************
 * macro for log
 **********************************/
#define SPOWER_LOG_NONE                                0
#define SPOWER_LOG_WITH_XLOG                           1
#define SPOWER_LOG_WITH_PRINTK                         2

#define SPOWER_LOG_PRINT SPOWER_LOG_WITH_PRINTK

#if (SPOWER_LOG_PRINT == SPOWER_LOG_NONE)
#define SPOWER_INFO(fmt, args...)
#elif (SPOWER_LOG_PRINT == SPOWER_LOG_WITH_XLOG)
#define SPOWER_INFO(fmt, args...)
#elif (SPOWER_LOG_PRINT == SPOWER_LOG_WITH_PRINTK)
#define SPOWER_INFO(fmt, args...)
#endif



static sptbl_t sptab[MT_SPOWER_MAX];

/****************************************************************
 * this table is generated by scramble function.                *
 * (plz refer to DE team.)                                      *
 ****************************************************************/
int devinfo_table[] = {
	71, 10, 148, 10, 16, 10, 34, 11, 1349, 11, 2818, 11, 309, 11, 646, 12,
	69, 12, 145, 12, 16, 13, 33, 13, 1318, 13, 2754, 13, 302, 14, 631, 14,
	74, 14, 155, 15, 17, 15, 35, 15, 1413, 16, 2951, 16, 324, 17, 676, 17,
	72, 17, 151, 18, 17, 18, 35, 19, 1380, 19, 2884, 19, 316, 20, 661, 20,
	65, 21, 135, 21, 15, 22, 31, 22, 1230, 23, 2570, 23, 282, 24, 589, 25,
	63, 25, 132, 26, 14, 26, 30, 27, 1202, 28, 2512, 28, 275, 29, 575, 30,
	68, 30, 141, 31, 15, 32, 32, 32, 1288, 33, 2692, 34, 295, 35, 617, 35,
	66, 36, 138, 37, 15, 38, 32, 39, 1259, 40, 2630, 41, 288, 42, 603, 43,
	85, 44, 178, 45, 19, 46, 41, 47, 1622, 48, 3388, 49, 372, 50, 776, 51,
	83, 52, 174, 54, 19, 55, 40, 56, 1585, 58, 3311, 59, 363, 60, 759, 62,
	89, 63, 186, 65, 20, 66, 43, 68, 1698, 69, 3548, 71, 389, 72, 813, 74,
	87, 76, 182, 78, 20, 79, 42, 81, 1660, 83, 3467, 85, 380, 87, 794, 89,
	78, 91, 162, 93, 18, 95, 37, 98, 1479, 100, 3090, 102, 339, 105, 708, 107,
	76, 110, 158, 112, 17, 115, 36, 117, 1445, 120, 3020, 123, 331, 126, 692, 129,
	81, 132, 170, 135, 19, 138, 39, 141, 1549, 145, 3236, 148, 355, 151, 741, 155,
	79, 158, 166, 162, 18, 166, 38, 170, 1514, 174, 3162, 178, 347, 182, 724, 186,
	49, 191, 102, 195, 11, 200, 23, 204, 933, 209, 1950, 214, 214, 219, 447, 224,
	48, 229, 100, 234, 11, 240, 23, 245, 912, 251, 1905, 257, 209, 263, 437, 269,
	51, 275, 107, 282, 12, 288, 25, 295, 977, 302, 2042, 309, 224, 316, 468, 324,
	50, 331, 105, 339, 11, 347, 24, 355, 955, 363, 1995, 372, 219, 380, 457, 389,
	45, 398, 93, 407, 10, 417, 21, 427, 851, 437, 1778, 447, 195, 457, 407, 468,
	44, 479, 91, 490, 10, 501, 21, 513, 832, 525, 1738, 537, 191, 550, 398, 562,
	47, 575, 98, 589, 11, 603, 22, 617, 891, 631, 1862, 646, 204, 661, 427, 676,
	46, 692, 95, 708, 10, 724, 22, 741, 871, 759, 1820, 776, 200, 794, 417, 813,
	59, 832, 123, 851, 13, 871, 28, 891, 1122, 912, 2344, 933, 257, 955, 537, 977,
	58, 1000, 120, 1023, 13, 1047, 28, 1072, 1096, 1096, 2291, 1122, 251, 1148, 525, 1175,
	62, 1202, 129, 1230, 14, 1259, 30, 1288, 1175, 1318, 2455, 1349, 269, 1380, 562, 1413,
	60, 1445, 126, 1479, 14, 1514, 29, 1549, 1148, 1585, 2399, 1622, 263, 1660, 550, 1698,
	54, 1738, 112, 1778, 12, 1820, 26, 1862, 1023, 1905, 2138, 1950, 234, 1995, 490, 2042,
	52, 2089, 110, 2138, 12, 2188, 25, 2239, 1000, 2291, 2089, 2344, 229, 2399, 479, 2455,
	56, 2512, 117, 2570, 13, 2630, 27, 2692, 1072, 2754, 2239, 2818, 245, 2884, 513, 2951,
	55, 3020, 115, 3090, 13, 3162, 26, 3236, 1047, 3311, 2188, 3388, 240, 3467, 501, 3548,
};


int interpolate(int x1, int x2, int x3, int y1, int y2)
{
	/* BUG_ON(x1==x2); */
	if (x1 == x2)
		return (y1 + y2) / 2;

	return (x3 - x1) * (y2 - y1) / (x2 - x1) + y1;
}

int interpolate_2d(sptbl_t *tab, int v1, int v2, int t1, int t2, int voltage, int degree)
{
	int c1, c2, p1, p2, p;

	if (v1 == v2 && t1 == t2) {
		p = mA(tab, v1, t1);
		return p;
	} else if (v1 == v2) {
		c1 = mA(tab, v1, t1);
		c2 = mA(tab, v1, t2);
		p = interpolate(deg(tab, t1), deg(tab, t2), degree, c1, c2);
		return p;
	} else if (t1 == t2) {
		c1 = mA(tab, v1, t1);
		c2 = mA(tab, v2, t1);
		p = interpolate(mV(tab, v1), mV(tab, v2), voltage, c1, c2);
		return p;
	}

	c1 = mA(tab, v1, t1);
	c2 = mA(tab, v1, t2);
	p1 = interpolate(deg(tab, t1), deg(tab, t2), degree, c1, c2);

	c1 = mA(tab, v2, t1);
	c2 = mA(tab, v2, t2);
	p2 = interpolate(deg(tab, t1), deg(tab, t2), degree, c1, c2);

	p = interpolate(mV(tab, v1), mV(tab, v2), voltage, p1, p2);
	return p;
}

void interpolate_table(sptbl_t *spt, int c1, int c2, int c3, sptbl_t *tab1, sptbl_t *tab2)
{
	int v, t;

	/* avoid divid error, if we have bad raw data table */
	if (unlikely(c1 == c2)) {
		*spt = *tab1;
		SPOWER_INFO("sptab equal to tab1:%d/%d\n", c1, c3);
		return;
	}

	SPOWER_INFO("make sptab %d, %d, %d\n", c1, c2, c3);
	for (t = 0; t < tsize(spt); t++) {
		for (v = 0; v < vsize(spt); v++) {
			int *p = &mA(spt, v, t);

			p[0] = interpolate(c1, c2, c3, mA(tab1, v, t), mA(tab2, v, t));

			SPOWER_INFO("%d ", p[0]);
		}
		SPOWER_INFO("\n");
	}
	SPOWER_INFO("make sptab done!\n");
}


int sptab_lookup(sptbl_t *tab, int voltage, int degree)
{
	int x1, x2, y1, y2, i;
	int mamper;

	/** lookup voltage **/
	for (i = 0; i < vsize(tab); i++) {
		if (voltage <= mV(tab, i))
			break;
	}

	if (unlikely(voltage == mV(tab, i))) {
		x1 = x2 = i;
	} else if (unlikely(i == vsize(tab))) {
		x1 = vsize(tab) - 2;
		x2 = vsize(tab) - 1;
	} else if (i == 0) {
		x1 = 0;
		x2 = 1;
	} else {
		x1 = i - 1;
		x2 = i;
	}


	/** lookup degree **/
	for (i = 0; i < tsize(tab); i++) {
		if (degree <= deg(tab, i))
			break;
	}

	if (unlikely(degree == deg(tab, i))) {
		y1 = y2 = i;
	} else if (unlikely(i == tsize(tab))) {
		y1 = tsize(tab) - 2;
		y2 = tsize(tab) - 1;
	} else if (i == 0) {
		y1 = 0;
		y2 = 1;
	} else {
		y1 = i - 1;
		y2 = i;
	}

	mamper = interpolate_2d(tab, x1, x2, y1, y2, voltage, degree);

	return mamper;
}


int mt_spower_make_table(sptbl_t *spt, spower_raw_t *spower_raw, int wat, int voltage, int degree)
{
	int i;
	int c1, c2, c = -1;
	sptbl_t tab[MAX_TABLE_SIZE], *tab1, *tab2, *tspt;

	/** FIXME, test only; please read efuse to assign. **/
	/* wat = 80; */
	/* voltage = 1150; */
	/* degree = 30; */

	BUG_ON(spower_raw->table_size < 3);

	/** structurize the raw data **/
	spower_tab_construct(&tab, spower_raw);

	/** lookup tables which the chip type locates to **/
	for (i = 0; i < spower_raw->table_size; i++) {
		c = sptab_lookup(&tab[i], voltage, degree);
		/** table order: ff, tt, ss **/
		if (wat >= c)
			break;
	}

	/** FIXME,
	 * There are only 2 tables are used to interpolate to form SPTAB.
	 * Thus, sptab takes use of the container which raw data is not used anymore.
	 **/

	if (wat == c) {
		/** just match **/
		tab1 = tab2 = &tab[i];
		/** pointer duplicate  **/
		tspt = tab1;

		SPOWER_INFO("sptab equal to tab:%d/%d\n", wat, c);

	} else if (i == spower_raw->table_size) {
		/** above all **/
#if defined(EXTER_POLATION)
		tab1 = &tab[spower_raw->table_size - 2];
		tab2 = &tab[spower_raw->table_size - 1];

		/** occupy the free container**/
		tspt = &tab[spower_raw->table_size - 3];
#else				/* #if defined (EXTER_POLATION) */
		tspt = tab1 = tab2 = &tab[spower_raw->table_size - 1];
#endif				/* #if defined (EXTER_POLATION) */

		SPOWER_INFO("sptab max tab:%d/%d\n", wat, c);
	} else if (i == 0) {
#if defined(EXTER_POLATION)
		/** below all **/
		tab1 = &tab[0];
		tab2 = &tab[1];

		/** occupy the free container**/
		tspt = &tab[2];
#else				/* #if defined (EXTER_POLATION) */
		tspt = tab1 = tab2 = &tab[0];
#endif				/* #if defined (EXTER_POLATION) */

		SPOWER_INFO("sptab min tab:%d/%d\n", wat, c);
	} else {
		/** anyone **/
		tab1 = &tab[i - 1];
		tab2 = &tab[i];

		/** occupy the free container**/
		tspt = &tab[(i + 1) % spower_raw->table_size];

		SPOWER_INFO("sptab interpolate tab:%d/%d\n", wat, c);
	}


	/** sptab needs to interpolate 2 tables. **/
	if (tab1 != tab2) {
		c1 = sptab_lookup(tab1, voltage, degree);
		c2 = sptab_lookup(tab2, voltage, degree);

		interpolate_table(tspt, c1, c2, wat, tab1, tab2);
	}

	/** update to global data **/
	*spt = *tspt;

	return 0;
}




/* #define MT_SPOWER_UT 1 */

#if defined(MT_SPOWER_UT)
void mt_spower_ut(void)
{
	int v, t, p, i;

	for (i = 0; i < MT_SPOWER_MAX; i++) {
		sptbl_t *spt = &sptab[i];

		v = 750;
		t = 22;
		p = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);

		v = 750;
		t = 25;
		p = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);

		v = 750;
		t = 28;
		p = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);

		v = 750;
		t = 82;
		p = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);

		v = 750;
		t = 120;
		p = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);

		v = 820;
		t = 22;
		p = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);

		v = 820;
		t = 25;
		p = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);

		v = 820;
		t = 28;
		p = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);

		v = 820;
		t = 82;
		p = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);


		v = 820;
		t = 120;
		p = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);

		v = 1200;
		t = 22;
		p = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);

		v = 1200;
		t = 25;
		p = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);

		v = 1200;
		t = 28;
		p = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);

		v = 1200;
		t = 82;
		p = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);


		v = 1200;
		t = 120;
		p = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);


		v = 950;
		t = 80;
		p = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);

		v = 1000;
		t = 85;
		p = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);


		v = 1150;
		t = 105;
		p = sptab_lookup(spt, v, t);
		SPOWER_INFO("v/t/p: %d/%d/%d\n", v, t, p);

	}
}


#endif				/* #if defined (MT_SPOWER_UT) */

int mt_spower_init(void)
{
#define DEVINFO_IDX0 (17)
#define DEVINFO_IDX1 (5)
#define DEVINFO_MP1_BIT (0)
#define DEVINFO_MP0_BIT (8)
#define DEVINFO_GPU_BIT (16)
#define DEVINFO_VCORE_BIT (24)

	int devinfo;
	int devinfo1 = 0;
	int mp1;
	int mp0;
	int gpu;
	int vcore;

	/* avoid side effect from multiple invocation */
	if (tab_validate(&sptab[MT_SPOWER_CA7]))
		return 0;

	devinfo = (int)get_devinfo_with_index(DEVINFO_IDX0);
	mp1 = (((devinfo >> DEVINFO_MP1_BIT) & 0x0ff) << 1) | devinfo1;
	mp0 = (((devinfo >> DEVINFO_MP0_BIT) & 0x0ff) << 1) | devinfo1;
	gpu = (((devinfo >> DEVINFO_GPU_BIT) & 0x0ff) << 1) | devinfo1;
	vcore = (((devinfo >> DEVINFO_VCORE_BIT) & 0x0ff) << 1) | devinfo1;

	/* SPOWER_INFO("LEAK95_NOSW VERIFY 1:%x %x(%x)\n", devinfo, t, devinfo1); */

	mp1 = (int)devinfo_table[mp1];
	mp0 = (int)devinfo_table[mp0];
	gpu = (int)devinfo_table[gpu];
	vcore = (int)devinfo_table[vcore];

	mt_spower_make_table(&sptab[MT_SPOWER_CA7], &ca7_spower_raw, mp0, 1150, 30);
	mt_spower_make_table(&sptab[MT_SPOWER_CA17], &ca15l_spower_raw, mp1, 1150, 30);
	mt_spower_make_table(&sptab[MT_SPOWER_GPU], &gpu_spower_raw, gpu, 1150, 30);
	mt_spower_make_table(&sptab[MT_SPOWER_VCORE], &vcore_spower_raw, vcore, 1150, 30);

	return 0;
}

/* late_initcall(mt_spower_init); */


/** return 0, means sptab is not yet ready. **/
int mt_spower_get_leakage(int dev, int vol, int deg)
{
	BUG_ON(!(dev < MT_SPOWER_MAX));

	if (!tab_validate(&sptab[dev]))
		return 0;

	return sptab_lookup(&sptab[dev], vol, deg);
}
EXPORT_SYMBOL(mt_spower_get_leakage);
