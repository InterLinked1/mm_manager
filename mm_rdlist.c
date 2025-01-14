/*
 * Code to dump RDLIST table from Nortel Millennium Payphone
 * Table 55 (0x37)
 *
 * www.github.com/hharte/mm_manager
 *
 * Copyright (c) 2020-2022, Howard M. Harte
 *
 * Reference: https://wiki.millennium.management/dlog:dlog_mt_carrier_table
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include "./mm_manager.h"

rdlist_table_entry_t rdlist[10];

int main(int argc, char *argv[]) {
    FILE *instream;
    FILE *ostream = NULL;
    int   rdlist_index;
    char  phone_number[17];
    char  display_prompt_string[21];
    char  display_prompt_string2[21];
    int   ret = 0;

    dlog_mt_rdlist_table_t *prdlist_table;
    uint8_t* load_buffer;

    if (argc <= 1) {
        printf("Usage:\n" \
               "\tmm_rdlist mm_table_37.bin [outputfile.bin]\n");
        return -1;
    }

    printf("Nortel Millennium RDLIST Table (Table 55) Dump\n\n");

    prdlist_table = (dlog_mt_rdlist_table_t *)calloc(1, sizeof(dlog_mt_rdlist_table_t));

    if (prdlist_table == NULL) {
        printf("Failed to allocate %zu bytes.\n", sizeof(dlog_mt_rdlist_table_t));
        return -ENOMEM;
    }

    if ((instream = fopen(argv[1], "rb")) == NULL) {
        printf("Error opening %s\n", argv[1]);
        free(prdlist_table);
        return -ENOENT;
    }

    load_buffer = ((uint8_t*)prdlist_table) + 1;
    if (fread(load_buffer, sizeof(dlog_mt_rdlist_table_t) - 1, 1, instream) != 1) {
        printf("Error reading RDLIST table.\n");
        free(prdlist_table);
        fclose(instream);
        return -EIO;
    }

    fclose(instream);

    printf("+-----------------------------------------------------------------------------------------------+\n" \
           "|  # | Pad            | Number           | Display Prompt       |  Pad2                         |\n" \
           "+----+----------------+------------------+----------------------+-------------------------------+");

    for (rdlist_index = 0; rdlist_index < RDLIST_MAX; rdlist_index++) {
        callscrn_num_to_string(phone_number, sizeof(phone_number), prdlist_table->rd[rdlist_index].phone_number, 8);

        if (prdlist_table->rd[rdlist_index].display_prompt[0] >= 0x20) {
            memcpy(display_prompt_string, prdlist_table->rd[rdlist_index].display_prompt, 20);
            display_prompt_string[20] = '\0';
        } else {
            snprintf(display_prompt_string, sizeof(display_prompt_string), "%s", "                    ");
        }

        if (prdlist_table->rd[rdlist_index].display_prompt[20] >= 0x20) {
            memcpy(display_prompt_string2, &(prdlist_table->rd[rdlist_index].display_prompt[20]), 20);
            display_prompt_string2[20] = '\0';
        } else {
            snprintf(display_prompt_string2, sizeof(display_prompt_string2), "%s", "                    ");
        }

        printf("\n| %2d | 0x%02x,0x%02x,0x%02x | %16s | %s | 0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x |",
               rdlist_index,
               prdlist_table->rd[rdlist_index].pad[0],
               prdlist_table->rd[rdlist_index].pad[1],
               prdlist_table->rd[rdlist_index].pad[2],
               phone_number,
               display_prompt_string,
               prdlist_table->rd[rdlist_index].pad2[0],
               prdlist_table->rd[rdlist_index].pad2[1],
               prdlist_table->rd[rdlist_index].pad2[2],
               prdlist_table->rd[rdlist_index].pad2[3],
               prdlist_table->rd[rdlist_index].pad2[4],
               prdlist_table->rd[rdlist_index].pad2[5]);

        printf("\n|    |                |                  | %s |                               |",
               display_prompt_string2);
    }

    printf("\n+-----------------------------------------------------------------------------------------------+\n");

    if (argc > 2) {
        if ((ostream = fopen(argv[2], "wb")) == NULL) {
            printf("Error opening output file %s for write.\n", argv[2]);
            return -ENOENT;
        }
    }

    /* If output file was specified, write it. */
    if (ostream != NULL) {
        printf("\nWriting new table to %s\n", argv[2]);

        if (fwrite(load_buffer, sizeof(dlog_mt_rdlist_table_t) - 1, 1, ostream) != 1) {
            printf("Error writing output file %s\n", argv[2]);
            ret = -EIO;
        }
        fclose(ostream);
    }

    if (prdlist_table != NULL) {
        free(prdlist_table);
    }

    return ret;
}
