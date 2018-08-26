#include "STM32F4_SPI.h"
#include "STM32F4_GPIO.h"
#include "STM32F4_RCC.h"

// SPI module
SPI_Module_With_State_Typedef SPIModule;

// Initialize SPI
void spiInit(SPI_Module_With_State_Typedef* SPIx)
{
	// Current frequency
	double frequency;
	SystemCoreClockUpdate();
	frequency = SystemCoreClock;
	
	// Consider AHB prescaler
	frequency = frequency / rccGetAhbPrescaler();
	
	// Consider APBx prescalers
	if (SPIx->module == SPI1)
	{
		// For modules of APB2 bus
		frequency = frequency / rccGetApb2Prescaler();
	}
	else
	{
		// For modules of APB1 bus
		frequency = frequency / rccGetApb1Prescaler();
	}
	
	// Enable clocking for SPI module
	spiInitClock(SPIx);
	
	// Calculate divider
	uint8_t divider = (frequency/ 1000000);
	
	// Set BR bits
	SPIx->module->CR1 |= 5 << 3; // 64 divider
	
	// Choose one of four data transfer and clock signal modes
	SPIx->module->CR1 &= ~(SPI_CR1_CPHA | SPI_CR1_CPOL);
	
	// Set first bit MSB
	SPIx->module->CR1 &= ~SPI_CR1_LSBFIRST;
	
	// Set data frame formate (8 bits)
	SPIx->module->CR1 &= ~SPI_CR1_DFF;
	
	// Software slave select master mode
	SPIx->module->CR1 |= SPI_CR1_SSM;
	SPIx->module->CR1 |= SPI_CR1_SSI;
	
	// Set master mode
	SPIx->module->CR1 |= SPI_CR1_MSTR;
	
	return;
}

// Initialize clock for SPI module
void spiInitClock(SPI_Module_With_State_Typedef* SPIx)
{
	if (SPIx->module == SPI1)
	{
		// Enable clock signal for  SPI1
		RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
		return;
	}
	else if (SPIx->module == SPI2)
	{
		//Enable clock signal for  SPI2
		RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
		return;
	}
	else if (SPIx->module == SPI3)
	{
		//Enable clock signal for  SPI3
		RCC->APB1ENR |= RCC_APB1ENR_SPI3EN;
		return;
	}
	return;
}

// Initialize gpio for SPI module
void spiInitGpio(SPI_Module_With_State_Typedef* SPIx)
{
	if (SPIx->module == SPI1)
	{
		return;
	}
	else if (SPIx->module == SPI2)
	{
		// SCLK, MISO, MOSI initialization
		gpioInitPin(GPIOB, GPIO_Pin_13, GPIO_MODE_AF, GPIO_OUTPUT_MODE_OD, GPIO_PUPD_NOPULL);
		gpioInitPin(GPIOB, GPIO_Pin_14, GPIO_MODE_AF, GPIO_OUTPUT_MODE_OD, GPIO_PUPD_NOPULL);
		gpioInitPin(GPIOB, GPIO_Pin_15, GPIO_MODE_AF, GPIO_OUTPUT_MODE_OD, GPIO_PUPD_NOPULL);
		
		gpioInitPinAf(GPIOB, GPIO_Pin_13, GPIO_AF_SPI2);
		gpioInitPinAf(GPIOB, GPIO_Pin_14, GPIO_AF_SPI2);
		gpioInitPinAf(GPIOB, GPIO_Pin_15, GPIO_AF_SPI2);
		return;
	}
	else if (SPIx->module == SPI3)
	{

		return;
	}
	return;
}

// Enable SPI
void spiEnable(SPI_Module_With_State_Typedef* SPIx)
{
	SPIx->module->CR1 |= SPI_CR1_SPE;
	return;
}

// Wait for TXE bit
ErrorStatus spiWaitForTXE(SPI_Module_With_State_Typedef* SPIx)
{
	uint32_t startTime = getLocalTime();
	while (!(SPIx->module->SR & SPI_SR_TXE))
	{
		if (SPIx->status != SPI_ACTIVE_MODE)
		{
			SPIx->status = SPI_ERROR;
			return ERROR;
		}
		if (checkTimeout(startTime, SPI_TIMEOUT_VALUE_TENTH_OF_MS))
		{
			SPIx->status = SPI_TIMEOUT_ERROR;
			return ERROR;
		}
	}
	return SUCCESS;
}

// Wait for RXNE bit
ErrorStatus spiWaitForRXNE(SPI_Module_With_State_Typedef* SPIx)
{
	uint32_t startTime = getLocalTime();
	while (!(SPIx->module->SR & SPI_SR_RXNE))
	{
		if (SPIx->status != SPI_ACTIVE_MODE)
		{
			SPIx->status = SPI_ERROR;
			return ERROR;
		}
		if (checkTimeout(startTime, SPI_TIMEOUT_VALUE_TENTH_OF_MS))
		{
			SPIx->status = SPI_TIMEOUT_ERROR;
			return ERROR;
		}
	}
	return SUCCESS;
}

// Wait for BSY flag
ErrorStatus spiWaitForBSY(SPI_Module_With_State_Typedef* SPIx)
{
	uint32_t startTime = getLocalTime();
	while (SPIx->module->SR & SPI_SR_BSY)
	{
		if (SPIx->status != SPI_ACTIVE_MODE)
		{
			SPIx->status = SPI_ERROR;
			return ERROR;
		}
		if (checkTimeout(startTime, SPI_TIMEOUT_VALUE_TENTH_OF_MS))
		{
			SPIx->status = SPI_TIMEOUT_ERROR;
			return ERROR;
		}
	}
	return SUCCESS;
}

// Send data
ErrorStatus spiSendBytes(SPI_Module_With_State_Typedef* SPIx, uint8_t* data, uint8_t numberOfBytes)
{
	uint8_t i;
	for (i = 0; i < numberOfBytes; i++)
	{
		// Wait for the permission to write next byte
		if (spiWaitForTXE(&SPIModule) != SUCCESS)
		{
			return ERROR;
		}
		// Load data to data register
		SPIx->module->DR = *data;
		// Increment pointer
		data++;
	}
	if (spiWaitForTXE(&SPIModule) != SUCCESS)
	{
		return ERROR;
	}
	return SUCCESS;
}

// Read data
ErrorStatus spiReadBytes(SPI_Module_With_State_Typedef* SPIx, uint8_t* readBuffer, uint32_t numberOfBytes)
{
	// Wait for the last transfer to be finished
	if (spiWaitForBSY(&SPIModule) != SUCCESS)
	{
		return ERROR;
	}
	// Procedure to clear RXNE flag
	if (spiWaitForRXNE(&SPIModule) != SUCCESS)
	{
		return ERROR;
	}
	// Read data to clear RXNE
	*readBuffer = SPIx->module->DR;
	
	uint8_t i;
	for (i = 0; i < numberOfBytes; i++)
	{	
		// Send dummy command
		SPIx->module->DR = 0x00;
		
		// Wait for the next byte to be received
		if (spiWaitForRXNE(&SPIModule) != SUCCESS)
		{
			return ERROR;
		}
		// Read data from data register
		*readBuffer = SPIx->module->DR;
		
		// Increment pointer
		readBuffer++;
	}
	return SUCCESS;
}


