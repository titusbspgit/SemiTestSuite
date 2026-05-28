
#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "pcie_dbi_dsp_reg_wr_rd_test.h"

#define CNT 4

unsigned long addr_array[CNT] = {
    0x004,  // CMD_STATUS
    0x008,  // CFG_SETUP
    0x030,  // OB_SIZE
    0x050   // IRQ_EOI
};

unsigned int default_value_array[CNT] = {0,0,0,0};

unsigned int read_mask_array[CNT] = {
    0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF};

unsigned int write_mask_array[CNT] = {
    0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF};

unsigned int skip_array[CNT]={0,0,0,0};

int data_rd,data_wr;
int def_fail_cnt=0,wr_fail_cnt=0;

int test_case()
{
    chk_rst_val();
    chk_rd_wr();

    if(def_fail_cnt>0||wr_fail_cnt>0) finish(1);
    else finish(0);
}

void chk_rst_val()
{
    for(int i=0;i<CNT;i++)
    {
        data_rd = read_reg(addr_array[i]);
        if(data_rd != default_value_array[i])
        {
            def_fail_cnt++;
            printf("DSP_RST_FAIL 0x%x\n",addr_array[i]);
        }
    }
}

void chk_rd_wr()
{
    int patterns[3]={0xAAAAAAAA,0x55555555,0x0};

    for(int j=0;j<3;j++)
    {
        data_wr=patterns[j];

        for(int i=0;i<CNT;i++)
            write_reg(addr_array[i],data_wr);

        for(int i=0;i<CNT;i++)
        {
            data_rd=read_reg(addr_array[i]);
            if(data_rd!=data_wr)
            {
                wr_fail_cnt++;
                printf("DSP_WR_FAIL 0x%x\n",addr_array[i]);
            }
        }
    }
}
