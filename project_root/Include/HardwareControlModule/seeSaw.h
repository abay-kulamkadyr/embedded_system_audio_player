/*
 *The module initializes the SeeSaw chip for reading and writing to its registers
 */
#ifndef _SEESAW_H
#define _SEESAW_H
#include <stdint.h>

void SeeSaw_Init(void);                                                             /*initializes seesaw device*/
int SeeSaw_Write(size_t bytes_num, ...);    /*write */
int SeeSaw_Read(unsigned char module_base_reg,unsigned char module_func_reg, unsigned char *buff, size_t buff_size);
void SeeSaw_Destroy(void);
unsigned char SeeSaw_byteRead (unsigned char base, unsigned char reg);
#endif 