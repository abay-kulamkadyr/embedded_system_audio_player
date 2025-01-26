/*
 * The module initializes the SeeSaw chip for reading and writing to its registers.
 */
#ifndef SEE_SAW_H
#define SEE_SAW_H

#include <stdint.h>
#include <stdlib.h>

void SeeSaw_Init(void);
int  SeeSaw_Write(size_t bytes_num, ...);
int  SeeSaw_Read(unsigned char module_base_reg,
                 unsigned char module_func_reg,
                 unsigned char *buff,
                 size_t buff_size);
void SeeSaw_Destroy(void);
unsigned char SeeSaw_byteRead(unsigned char base, unsigned char reg);

#endif

