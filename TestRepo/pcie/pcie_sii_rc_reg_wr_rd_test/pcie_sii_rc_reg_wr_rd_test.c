
#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "pcie_sii_rc_reg_wr_rd_test.h"

#define CNT 3

unsigned long addr_array[CNT] = {
    0x300,  // IB_BAR0
    0x304,  // IB_START_LO
    0x30C   // IB_OFFSET
};

unsigned int default_value_array[CNT]={0,0,0};
unsigned int read_mask_array[CNT]={0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF};
unsigned int write_mask_array[CNT]={0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF};
unsigned int skip_array[CNT]={0,0,0};

int data_rd,data_wr;
int def_fail_cnt=0,wr_fail_cnt=0;

int test_case()
{
    chk_rst_val();
    chk_rd_wr();

    if(def_fail_cnt||wr_fail_cnt) finish(1);
    else finish(0);
}

void chk_rst_val()
{
    for(int i=0;i<CNT;i++)
        if(read_reg(addr_array[i])!=default_value_array[i])
            def_fail_cnt++;
}

void chk_rd_wr()
{
    int patterns[2]={0xFFFFFFFF,0x0};

    for(int j=0;j<2;j++)
    {
        data_wr=patterns[j];

        for(int i=0;i<CNT;i++)
            write_reg(addr_array[i],data_wr);

        for(int i=0;i<CNT;i++)
            if(read_reg(addr_array[i])!=data_wr)
                wr_fail_cnt++;
    }
}
