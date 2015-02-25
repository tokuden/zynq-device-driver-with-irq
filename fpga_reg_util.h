#ifndef FPGA_REG_UTIL_H
#define FPGA_REG_UTIL_H

#define XPAR_AXI_EXT_SLAVE_CONN_0_S_AXI_RNG00_BASEADDR 0xB8000000

void map_fpga_register(const unsigned long addr, const unsigned long size);
unsigned int read_fpga_register(const unsigned long offset);
void write_fpga_register(const unsigned long offset, const unsigned long value);
void unmap_fpga_register(const unsigned long addr, const unsigned long size);

void print_fpga_registers(void);

#endif