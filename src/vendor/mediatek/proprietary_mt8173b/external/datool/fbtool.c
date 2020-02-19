/*
 * Copyright (c) 2014 MediaTek Inc.
 * Author: Tristan Shieh <tristan.shieh@mediatek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tty_usb.h"
#include "cmds.h"
#include "file_util.h"
#include "parse_ini.h"

typedef struct {
    char            m_identifier[12];   // including '\0'
    uint32_t        m_ver;
    uint32_t        m_dev_rw_unit;      // NAND: in page
                                        // NOR/eMMC/SFlash: in byte
} EMMC_HEADER_v1;

typedef struct {
    uint32_t        m_bl_exist_magic;
    uint8_t         m_bl_dev;               // 1B
    uint16_t        m_bl_type;              // 2B
    uint32_t        m_bl_begin_dev_addr;    // device addr that points to the beginning of BL
                                            // NAND: page addr
                                            //      SEQUENTIAL: phy page addr
                                            //      TTBL: TTBL logic page addr
                                            //      FDM5: FDM5.0 logic page addr
                                            // NOR/eMMC/SFlash: byte addr

    uint32_t        m_bl_boundary_dev_addr; // device addr that points to the boundary of BL
                                            // boundary addr = BL addr + BL length
                                            // NAND: page addr
                                            //      SEQUENTIAL: phy page addr
                                            //      TTBL: TTBL logic page addr
                                            //      FDM5: FDM5.0 logic page addr
                                            // NOR/eMMC/SFlash: byte addr

    uint32_t        m_bl_attribute;         // refer to GFH_BL_INFO
} BL_Descriptor;

typedef struct {
    char            m_identifier[8];        // including '\0'
    uint32_t        m_ver;                  // this structure will directly export to others, version field is necessary
    uint32_t        m_boot_region_dev_addr; // device addr that points to the beginning of the boot region
                                            // NAND: page addr
                                            //      SEQUENTIAL: phy page addr
                                            //      TTBL: TTBL logic page addr
                                            //      FDM5: FDM5.0 logic page addr
                                            // NOR/eMMC/SFlash: byte addr

    uint32_t        m_main_region_dev_addr; // device addr that points to the beginning of the main code region
                                            // NAND: page addr
                                            //      SEQUENTIAL: phy page addr
                                            //      FDM5: FDM5.0 logic page addr
                                            // NOR/eMMC/SFlash: byte addr
    BL_Descriptor   m_bl_desc;
} BR_Layout_v1;

typedef struct GFH_FILE_INFO_v1 {
    uint32_t        m_magic_ver;
    uint16_t        m_size;
    uint16_t        m_type;
    char            m_identifier[12];       // including '\0'
    uint32_t        m_file_ver;
    uint16_t        m_file_type;
    uint8_t         m_flash_dev;
    uint8_t         m_sig_type;
    uint32_t        m_load_addr;
    uint32_t        m_file_len;
    uint32_t        m_max_size;
    uint32_t        m_content_offset;
    uint32_t        m_sig_len;
    uint32_t        m_jump_offset;
    uint32_t        m_attr;
} GFH_FILE_INFO_v1;

void strip_pl_hdr(void *pl, size_t len_pl, void **strip_pl, size_t *len_strip_pl)
{
    EMMC_HEADER_v1 *ehdr = (EMMC_HEADER_v1 *)pl;
    GFH_FILE_INFO_v1 *gfh = (GFH_FILE_INFO_v1 *)pl;
    *strip_pl = pl;
    *len_strip_pl = len_pl;

    // emmc header
    if((strcmp("EMMC_BOOT", ehdr->m_identifier) == 0) &&
       (ehdr->m_ver == 1))
    {
        BR_Layout_v1 *brlyt;

        if(ehdr->m_dev_rw_unit + sizeof(BR_Layout_v1) > len_pl)
        {
            printf("ERROR: EMMC HDR error. dev_rw_unit=%x, brlyt_size=%zx, len_pl=%zx\n",
                        ehdr->m_dev_rw_unit,
                        sizeof(BR_Layout_v1),
                        len_pl);
            exit(EXIT_FAILURE);
        }

        brlyt = (BR_Layout_v1 *)((char *)pl + ehdr->m_dev_rw_unit);
        if((strcmp("BRLYT", brlyt->m_identifier) != 0) ||
           (brlyt->m_ver != 1))
        {
            printf("ERROR: BRLYT error. m_ver=%x, m_identifier=%s\n",
                        brlyt->m_ver,
                        brlyt->m_identifier);
            exit(EXIT_FAILURE);
        }

        gfh = (GFH_FILE_INFO_v1 *)((char *)pl + brlyt->m_bl_desc.m_bl_begin_dev_addr);
    }

    // gfh
    if(((gfh->m_magic_ver & 0x00FFFFFF) == 0x004D4D4D) &&
       (gfh->m_type == 0) &&
       (strcmp("FILE_INFO", gfh->m_identifier) == 0))
    {

        if(gfh->m_file_len < (gfh->m_jump_offset + gfh->m_sig_len))
        {
            // gfh error
            printf("ERROR: GFH error. len_pl=%zx, file_len=%x, jump_offset=%x, sig_len=%x\n",
                        len_pl,
                        gfh->m_file_len,
                        gfh->m_jump_offset,
                        gfh->m_sig_len);
            exit(EXIT_FAILURE);
        }

        *strip_pl = ((char*)gfh + gfh->m_jump_offset);
        *len_strip_pl = gfh->m_file_len - gfh->m_jump_offset - gfh->m_sig_len;
    }
}

enum CHIP_ID {
    CHIP_ID_DEFAULT,
    CHIP_ID_MT8135,
    CHIP_ID_MT8127,
    CHIP_ID_MT6595,
    CHIP_ID_MT8173,
    CHIP_ID_MT8590,
    CHIP_ID_MT8163,
};

typedef struct {
    enum CHIP_ID chip;
    uint32_t pl_load_addr;
    uint32_t pl_jump_64;
    uint32_t lk_wrapper_addr;
} CHIP_DATA;

CHIP_DATA chip_tbl[] = {
    {CHIP_ID_DEFAULT, 0x00201000, 0, 0x40001000},
    {CHIP_ID_MT8135,  0x12001000, 0, 0x80001000},
    {CHIP_ID_MT8127,  0x00201000, 0, 0x80001000},
    {CHIP_ID_MT6595,  0x00201000, 0, 0x40001000},
    {CHIP_ID_MT8173,  0x000C1000, 0, 0x40001000},
    {CHIP_ID_MT8590,  0x00201000, 0, 0x80001000},
    {CHIP_ID_MT8163,  0x00201000, 0, 0x40001000},
};

int get_chip_data_1(uint16_t chip, CHIP_DATA *chip_data)
{
    CHIP_DATA *tmp;
    if (chip == 0x8135)
        tmp = &(chip_tbl[1]);
    else if (chip == 0x8127)
        tmp = &(chip_tbl[2]);
    else if (chip == 0x6595)
        tmp = &(chip_tbl[3]);
    else if ((chip == 0x8172) || (chip == 0x8176))
        tmp = &(chip_tbl[4]);
    else if ((chip == 0x8590))
        tmp = &(chip_tbl[5]);
    else if (chip == 0x8163)
        tmp = &(chip_tbl[6]);
    else
        tmp = &(chip_tbl[0]);

    *chip_data = *tmp;

    return 0;
}

char line_auth[128];
char line_da1_path[128];
char line_da2_path[128];

int get_chip_data_2(char *dl_addr_path, char **auth_path, char **pl_path, char **lk_path, CHIP_DATA *chip)
{
    char addr[128], *pl_addr=NULL, *lk_addr=NULL;

    *auth_path = getKeyValue(dl_addr_path, "[Auth]", "auth_path", line_auth);
    if (*auth_path != NULL)
        printf("auth file is %s\n", *auth_path);

    *pl_path = getKeyValue(dl_addr_path, "[DA1_Path_Addr]", "da1_path", line_da1_path);
    if (*pl_path != NULL) {
        printf("da1_path is %s\n", *pl_path);
    } else {
        printf("Please set da1_path in ini file\n");
            return -1;
    }
    pl_addr = getKeyValue(dl_addr_path, "[DA1_Load_Addr]", "da1_addr", addr);
    if (pl_addr != NULL) {
        chip->pl_load_addr = strtoul(pl_addr, NULL, 16);
        printf("da1 load addr is 0x%x\n", chip->pl_load_addr);
    } else {
        chip->pl_load_addr = 0x201000;
        printf("cannot find da1_addr from .ini, use defalut 0x201000\n");
    }
    pl_addr = getKeyValue(dl_addr_path, "[DA1_JUMP_64]", "da1_jump_64", addr);
    if (pl_addr != NULL) {
        chip->pl_jump_64 = strtoul(pl_addr, NULL, 16);
        printf("da1 jump 64 is 0x%x\n", chip->pl_jump_64);
    } else {
        chip->pl_jump_64 = 0;
        printf("cannot find da1_jump_64 from .ini, use defalut 0\n");
    }


    *lk_path = getKeyValue(dl_addr_path, "[DA2_Path_Addr]", "da2_path", line_da2_path);
    if (*lk_path == NULL) {
        printf("single DA\n");
        return 0;
    } else {
        printf("da2_path is %s\n", *lk_path);
        lk_addr = getKeyValue(dl_addr_path, "[DA2_Wrapper_Addr]", "da2_addr", addr);
        if (lk_addr != NULL) {
            chip->lk_wrapper_addr = strtoul(lk_addr, NULL, 16);
            printf("da2 wrapper addr is 0x%x\n", chip->lk_wrapper_addr);
        } else {
            chip->lk_wrapper_addr = 0x40001000;
            printf("cannot find da2_addr from .ini, use defalut 0x40001000\n");
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    tty_usb_handle *h = NULL;
    int r, is_brom = 0;
    void *lk=NULL, *sig_lk=NULL;
    size_t len_lk, len_sig_lk;
    char *dl_addr_path=NULL, *auth_path=NULL, *pl_path=NULL, *lk_path=NULL;
    uint16_t chip_code;
    CHIP_DATA chip;
    char lk_sig_path[4096];

    if ((argc!=2) && (argc!=3) && (argc!=5)) {
        printf("Version 1.0 Usage: %s [-a auth] preloader lk\n", argv[0]);
        printf("Version 2.0 Usage: %s mtxxxx_dl_addr.ini\n", argv[0]);
        return 1;
    }

    if (argc == 3) {
        pl_path = argv[1];
        lk_path = argv[2];
    } else if (argc == 5) {
        if (strcmp(argv[1], "-a") != 0) {
            printf("Version 1.0 Usage: %s [-a auth] preloader lk\n", argv[0]);
            return 1;
        }
        auth_path = argv[2];
        pl_path = argv[3];
        lk_path = argv[4];
    } else if (argc == 2) {
        dl_addr_path = argv[1];
    }

    h = tty_usb_open_auto();
    is_brom = tty_usb_is_target_brom();

    r = start_cmd(h);
    if (r) {
        printf("ERROR: start_cmd(%d)\n", r);
        return 1;
    }
    if (is_brom)
        printf("connect brom\n");
    else
        printf("connect preloader\n");

    if ((argc==3) || (argc==5)) {
        r = get_hw_code(h, &chip_code);
        get_chip_data_1(chip_code, &chip);
        printf("da1 load addr is 0x%x\n", chip.pl_load_addr);
		printf("da2 wrapper addr is 0x%x\n", chip.lk_wrapper_addr);
    } else { //argc==2
        if (get_chip_data_2(dl_addr_path, &auth_path, &pl_path, &lk_path, &chip))
            return 0;
    }

    if (is_brom) {
        uint32_t cfg;
        void *pl=NULL, *sig_pl=NULL, *strip_pl=NULL;
        size_t len_pl, len_sig_pl, len_strip_pl;

        r = get_target_config(h, &cfg);
        if (r) {
            printf("ERROR: get_target_config(%d)\n", r);
            return 1;
        }
        if (TGT_CFG_DAA_EN & cfg) {
            void *auth;
            size_t len_auth;

            if ((TGT_CFG_SBC_EN & cfg) == 0) {
                printf("ERROR: daa=1, sbc=0\n");
                return 1;
            }

            if (auth_path==NULL) {
                printf("ERROR: no auth file\n");
                return 1;
            }

            load_binary(auth_path, &auth, &len_auth);
            r = send_auth(h, auth, len_auth);
            free(auth);
            if (r) {
                printf("ERROR: send_auth(%d)\n", r);
                return 1;
            }
            printf("send %s\n", auth_path);
        }

        load_binary(pl_path, &pl, &len_pl);
        strip_pl_hdr(pl, len_pl, &strip_pl, &len_strip_pl);
        if (TGT_CFG_DAA_EN & cfg) {
            char sig_path[4096];
            strcpy(sig_path, pl_path);
            strcat(sig_path, ".sign");
            load_binary(sig_path, &sig_pl, &len_sig_pl);
        } else {
            len_sig_pl = 0;
        }
        r = send_da(h, chip.pl_load_addr, strip_pl, len_strip_pl, sig_pl, len_sig_pl);
        if (pl)
            free(pl);
        if (sig_pl)
            free(sig_pl);
        if (r) {
            printf("ERROR: send_da(%d)\n", r);
            return 1;
        }
        printf("send %s\n", pl_path);

        if (chip.pl_jump_64 == 0) {
            r = jump_da(h, chip.pl_load_addr);
            if (r) {
                printf("ERROR: jump_da(%d)\n", r);
                return 1;
            }
        } else {
            r = jump_da_ex(h, chip.pl_load_addr);
            if (r) {
                printf("ERROR: jump_da_ex(%d)\n", r);
                return 1;
            }
        }

        tty_usb_close(h);
        if (lk_path == NULL)
            return 0;

        h = NULL;

        // open preloader tty
        h = tty_usb_open_pl();

        r = start_cmd(h);
        if (r) {
            printf("ERROR: start_cmd(%d)\n", r);
            return 1;
        }
        printf("connect preloader\n");
    }

    // gen fastboot da from lk
    load_binary(lk_path, &lk, &len_lk);
    strcpy(lk_sig_path, lk_path);
    strcat(lk_sig_path, ".sign");
    load_binary(lk_sig_path, &sig_lk, &len_sig_lk);
    r = send_da(h, chip.lk_wrapper_addr, lk, len_lk, sig_lk, len_sig_lk);
    if (lk)
        free(lk);
    if (sig_lk)
        free(sig_lk);
    if (r) {
        printf("ERROR: send_da(%d)\n", r);
        return 1;
    }
    printf("send %s\n", lk_path);

    r = jump_da(h, chip.lk_wrapper_addr);
    if (r) {
        printf("ERROR: jump_da(%d)\n", r);
        return 1;
    }

    tty_usb_close(h);

    return 0;
}

