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

#ifndef __MT_LASTPC_H__
#define __MT_LASTPC_H__

/* public APIs for those want to dump lastpc to buf, the len of buf is len */
int lastpc_dump(char *buf, int len);

/* for backward-compatibility */
int mt_reg_dump(char *buf);

#endif /* end of __MT_LASTPC_H__ */
