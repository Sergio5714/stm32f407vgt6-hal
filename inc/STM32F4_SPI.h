#ifndef STM32F4_SPI
 #define STM32F4_SPI
 
#include "stm32f4xx.h"
#include "Interrupts.h"

// Typedef for SPI module status
typedef enum
{
	SPI_ACTIVE_MODE,
	SPI_ERROR,
	SPI_TIMEOUT_ERROR
} SPI_Status_Typedef;


// Typedef for SPI module with state
typedef struct
{
	SPI_TypeDef* module;
	SPI_Status_Typedef status;
} SPI_Module_With_State_Typedef;

// Timeout 
#define SPI_TIMEOUT_VALUE_TENTH_OF_MS        0x32  // equals 5 millisecond affordable timeout between request and answer

//--------------------------------------------- Functions --------------------------------------------------//
// Initialize SPI
void spiInit(SPI_Module_With_State_Typedef* SPIx);
	
// Initialize clock for SPI module
void spiInitClock(SPI_Module_With_State_Typedef* SPIx);

// Initialize gpio for SPI module
void spiInitGpio(SPI_Module_With_State_Typedef* SPIx);

// Enable SPI
void spiEnable(SPI_Module_With_State_Typedef* SPIx);

// Wait for TXE flag
ErrorStatus spiWaitForTXE(SPI_Module_With_State_Typedef* SPIx);

// Wait for RXNE flag
ErrorStatus spiWaitForRXNE(SPI_Module_With_State_Typedef* SPIx);

// Wait for BSY flag
ErrorStatus spiWaitForBSY(SPI_Module_With_State_Typedef* SPIx);

// Send data
ErrorStatus spiSendBytes(SPI_Module_With_State_Typedef* SPIx, uint8_t* data, uint8_t numberOfBytes);

// Read data
ErrorStatus spiReadBytes(SPI_Module_With_State_Typedef* SPIx, uint8_t* readBuffer, uint32_t numberOfBytes);


#endif
