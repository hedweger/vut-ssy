#include "at30.h"

uint8_t at30_set_precision(uint8_t precision) {
	uint_fast16_t config_register = 0;
	// R0 is 13?
	config_register |= (uint16_t) (precision << R0);
	i2c_start();
	i2c_write(TempSensorAddrW);
	if (i2c_get_status( ) != 0x18 ) {
		UART_SendString ("Error 18\n\r") ;
		return 0 ;
	} ;
	i2c_write(Temp_configRegister) ;
	if ( i2c_get_status( ) != 0x28 ) {
		UART_SendString ( "Error 28\n\r" ) ;
		return 0 ;
	} ;
	i2c_write(( uint8_t ) ( config_register>>8) ) ;
	if ( i2c_get_status( ) != 0x28 ) {
		UART_SendString ( "Error 28\n\r" ) ;
		return 0 ;
	} ;
	i2c_write(( uint8_t ) ( config_register ) ) ;
	if ( i2c_get_status( ) != 0x28 ) {
		UART_SendString ( "Error 28\n\r" ) ;
		return 0 ;
	} ;
	i2c_stop( ) ;
	return 1;
}
float at30_read_temp(void) {
	volatile uint8_t buffer[2];
	volatile uint16_t teplotaTmp;
	uint8_t status;
	// nastavim co chci cist
	i2c_start();
	i2c_write(TempSensorAddrW);
	if (i2c_get_status( ) != 0x18 ) {
		UART_SendString ("Error 18\n\r") ;
		return 0 ;
	} ;
	i2c_write(Temp_tempRegister) ;
	status = i2c_get_status( );
	if ( status != 0x28 ) {
		UART_SendString ( "Error 28\n\r" ) ;
		return 0 ;
	} ;
	i2c_stop( ) ;
	// ctu
	i2c_start();
	if (i2c_get_status( ) != 0x08 ) {
		UART_SendString ( " Error 08\n\r " ) ;
		return 0 ;
	} ;
	i2c_write(TempSensorAddrR) ;
	if ( i2c_get_status( ) != 0x40 ) {
		UART_SendString ( " Error 40\n\r " ) ;
		return 0 ;
	} ;
	buffer[0] = i2c_readACK();
	if ( i2c_get_status( ) != 0x50 ) {
		UART_SendString ( " Error 50\n\r " ) ;
		return 0 ;
	} ;
	buffer[1] = i2c_readNACK();
	if ( i2c_get_status( ) != 0x58 ) {
		UART_SendString ( " Error 58\n\r " ) ;
		return 0 ;
	} ;
	i2c_stop( ) ;
	teplotaTmp = (buffer[0]<<8)|buffer[1];
	return (float)teplotaTmp/256;
}