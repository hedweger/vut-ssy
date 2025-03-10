/*
 * i2c.h
 *
 * Created: 3/10/2025 11:40:40
 *  Author: Student
 */ 


#ifndef I2C_H_
#define I2C_H_
void i2c_init(void);
void i2c_start(void);
void i2c_stop(void);
void i2c_write(uint8_t byte);
uint8_t i2c_readACK(void);
uint8_t i2c_readNACK(void);
uint8_t i2c_get_status(void);

#endif /* I2C_H_ */