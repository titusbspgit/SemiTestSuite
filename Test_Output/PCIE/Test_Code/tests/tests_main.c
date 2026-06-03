#include <stdio.h>
#include <string.h>
#include "vendor/unity/unity.h"

/* Declarations of all generated tests */
void pcie_cfg_wr_rd_test(void);
void pcie_dbi_dsp_reg_wr_rd_test(void);
void pcie_dbi_usp_reg_wr_rd_test(void);
void pcie_sii_rc_reg_wr_rd_test(void);

typedef struct { const char* name; void (*fn)(void); } test_desc_t;

static test_desc_t tests[] = {
    {"pcie_cfg_wr_rd_test", pcie_cfg_wr_rd_test},
    {"pcie_dbi_dsp_reg_wr_rd_test", pcie_dbi_dsp_reg_wr_rd_test},
    {"pcie_dbi_usp_reg_wr_rd_test", pcie_dbi_usp_reg_wr_rd_test},
    {"pcie_sii_rc_reg_wr_rd_test", pcie_sii_rc_reg_wr_rd_test},
};

int main(int argc, char** argv)
{
    const char* filter = (argc > 1) ? argv[1] : NULL;
    UnityBegin();

    for (unsigned i = 0; i < sizeof(tests)/sizeof(tests[0]); ++i) {
        if (filter && !strstr(tests[i].name, filter)) continue;
        printf("[ RUN      ] %s\n", tests[i].name);
        tests[i].fn();
    }

    return UnityEnd();
}
