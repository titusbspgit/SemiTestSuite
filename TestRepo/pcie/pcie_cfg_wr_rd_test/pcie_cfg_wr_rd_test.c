
#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "pcie_cfg_wr_rd_test.h"

/************ CONFIG REGISTER DATA ************/
#define CNT 5

unsigned long addr_array[CNT] = {
    0x1000,  // VENDOR_DEVICE_ID
    0x1004,  // STATUS_COMMAND
    0x1008,  // CLASSCODE
    0x1010,  // BAR0
    0x103C   // INT_PIN
};

unsigned int default_value_array[CNT] = {
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000
};

unsigned int read_mask_array[CNT] = {
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFFF,
    0x000000FF
};

unsigned int write_mask_array[CNT] = {
    0x0,            // RO
    0xFFFFFFFF,
    0xFFFFFFFF,
    0xFFFFFFF0,
    0x0             // RO
};

unsigned int skip_array[CNT] = {0,0,0,0,0};

/************ TEST LOGIC ************/
int data_rd, data_wr;
int def_fail_cnt = 0, wr_fail_cnt = 0;

int test_case()
{
    chk_rst_val();
    chk_rd_wr();

    if(def_fail_cnt > 0 || wr_fail_cnt > 0)
        finish(1);
    else
        finish(0);
}

/************ DEFAULT CHECK ************/
void chk_rst_val()
{
    for(int i=0;i<CNT;i++)
    {
        if(read_mask_array[i] == 0) continue;

        data_rd = read_reg(addr_array[i]);

        if(data_rd != default_value_array[i])
        {
            def_fail_cnt++;
            printf("CFG_RST_FAIL Addr:0x%x\n", addr_array[i]);
        }
    }
}

/************ WR/RD CHECK ************/
void chk_rd_wr()
{
    int patterns[4]={0xAAAAAAAA,0x55555555,0xFFFFFFFF,0x00000000};

    for(int j=0;j<4;j++)
    {
        data_wr = patterns[j];

        for(int i=0;i<CNT;i++)
        {
            if(skip_array[i] || write_mask_array[i]==0) continue;
            write_reg(addr_array[i], data_wr);
        }

        for(int i=0;i<CNT;i++)
        {
            if(skip_array[i] || read_mask_array[i]==0) continue;

            data_rd = read_reg(addr_array[i]);

            unsigned int exp =
                ((data_wr & write_mask_array[i]) |
                (~write_mask_array[i] & default_value_array[i]));

            if(data_rd != exp)
            {
                wr_fail_cnt++;
                printf("CFG_WR_FAIL Addr:0x%x\n", addr_array[i]);
            }
        }
    }
}
