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

#ifndef _SEC_CIPHER_FORMAT_CORE_H
#define _SEC_CIPHER_FORMAT_CORE_H

#include "sec_signfmt_def.h"

/******************************************************************************
 *  EXPORT FUNCTION
 ******************************************************************************/
int sec_cipherfmt_check_cipher(ASF_FILE fp, unsigned int start_off, unsigned int *img_len);
int sec_cipherfmt_decrypted(ASF_FILE fp, unsigned int start_off, char *buf, unsigned int buf_len,
			    unsigned int *data_offset);


#endif
