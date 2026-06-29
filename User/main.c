/*
 *  main.c
 *
 *  This file was adopted a long time ago and spent a lot of time sitting in
 *  prototyping, so an accurate date of its modification is not available.
 *  Author: kirisaki
 */

/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2023/12/22
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 *This project is designed for the ch32v003JxMx SOP-8 package and the ch422 in
 *the SOP-16 package.
 *      Hardware connection:
 *            PC4 (7) -- INT 
 *            PC2 (6) -- SCL 
 *            PC1 (5) -- SDA 
 *
 * 			  PA1 (1) -- UTX_2
 * 			  PA2 (3) -- Dir
 *
 */

#include "debug.h"
#include "init.h"
#include "consts.h"
#include <PetitModbus.h>

/* Constants */

/* Macros */

/* Types */

/* Global variables */
volatile u32 t1_count = 0;
u32 last_t1_count = 0;
u8 loop_overrun = 0;
T_PETIT_MODBUS Petit;
volatile u8 modbus_arm = false;
volatile u32 modbus_timer;

/*********************************************************************
 * @fn      IIC_TX
 *
 * @brief   Transmits through I2C
 *
 * @return  none
 */
void IIC_TX(u8 addr, const u8* data, u8 count, u8 no_end)
{
	u32 start_time = t1_count;
	while( I2C_GetFlagStatus( I2C1, I2C_FLAG_BUSY ) != RESET )
	{
		if (t1_count - start_time > 1U)
		{
			I2C_GenerateSTOP(I2C1, ENABLE);
			return;
		}
	}

	I2C_GenerateSTART( I2C1, ENABLE);

	while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_MODE_SELECT ) )
	{
		if (t1_count - start_time > 1U)
		{
			I2C_GenerateSTOP(I2C1, ENABLE);
			return;
		}
	}
	I2C_Send7bitAddress( I2C1, addr, I2C_Direction_Transmitter);

	while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ) )
	{
		if (t1_count -  start_time > 1U)
		{
			I2C_GenerateSTOP(I2C1, ENABLE);
			return;
		}
	}

	while (count-- != 0)
	{
		I2C_SendData(I2C1, *(data++));
	
		while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED ) )
		{
			if (t1_count -  start_time > 1U)
			{
				break;
			}
		}
		if (t1_count - start_time > 1U)
		{
			break;
		}
	}

	if (no_end == 0)
	{
		I2C_GenerateSTOP( I2C1, ENABLE);
	}
}

/*********************************************************************
 * @fn      IIC_RX
 *
 * @brief   Receives through I2C
 *
 * @return  none
 */
void IIC_RX(u8 addr, u8 *data, u8 count, u8 stopped)
{
	u32 start_time = t1_count;
	// block commands require acknowledge
	I2C_AcknowledgeConfig(I2C1, ENABLE);
	if(stopped)
	{
		while(I2C_GetFlagStatus( I2C1, I2C_FLAG_BUSY ) != RESET )
		{
			if (t1_count - start_time > 1U)
			{
				I2C_GenerateSTOP(I2C1, ENABLE);
				return;
			}
		}
	}
	else
	{
		while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED ) )
		{
			if (t1_count -  start_time > 1U)
			{
				I2C_GenerateSTOP( I2C1, ENABLE);
				return;
			}
		}
	}

	I2C_GenerateSTART( I2C1, ENABLE);

	while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_MODE_SELECT ) )
	{
		if (t1_count - start_time > 1U)
		{
			I2C_GenerateSTOP(I2C1, ENABLE);
			return;
		}
	}
	I2C_Send7bitAddress( I2C1, addr, I2C_Direction_Receiver);

	while (!I2C_CheckEvent (I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
	{
		if (t1_count - start_time > 1U)
		{
			I2C_GenerateSTOP(I2C1, ENABLE);
			return;
		}
	}

	while(count-- != 0)
	{
		while (I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE) == RESET)
		{
			if (t1_count - start_time > 1U)
			{
				I2C_GenerateSTOP(I2C1, ENABLE);
				return;
			}
		}
		// nack on the last bit
		if (count == 0)
		{
			I2C_AcknowledgeConfig(I2C1, DISABLE);
		}
		*(data++) = I2C_ReceiveData(I2C1);
	}

	I2C_GenerateSTOP( I2C1, ENABLE);
}


void PetitPortDirTx(void)
{
	GPIOA->BSHR = GPIO_Pin_2;
}

void PetitPortDirRx(void)
{
	GPIOA->BSHR = GPIO_Pin_2 << 16U;
}

void PetitUserTxBegin(pu8_t data)
{
	PetitPortDirTx();
	// output the first octet
	USART1->DATAR = data;
	// enable the transmit empty interrupt
	USART1->CTLR1 |= USART_CTLR1_TXEIE;
}

void PetitT15TimerStart(void)
{
	modbus_arm = true;
	modbus_timer = t1_count;
}

void PetitT15TimerStop(void)
{
	modbus_arm = false;
}

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
	SystemCoreClockUpdate();

	APP_GPIO_Init();

	TIME_Init();

	UART_Init();

	PETIT_MODBUS_Init(&Petit);
	Petit.Timer_Start = &PetitT15TimerStart;
	Petit.Timer_Stop = &PetitT15TimerStop;
	Petit.Tx_Begin = &PetitUserTxBegin;

	// start time
	TIM_Cmd(TIM1, ENABLE);
	TIM_ClearFlag(TIM1, TIM_FLAG_Update);

	//printf("IIC Host mode\r\n");
	IIC_Init(400000u, C_TSL2561_ADDR);

	// turn the light sensor on
	IIC_TX(C_TSL2561_ADDR, (u8[]){C_TSL2561_CMD_REG | C_TSL2561_CMD_ADDR_CR, C_TSL2561_CR_ON}, 2, 0);
	// configure for "15" bit conversion
	IIC_TX(C_TSL2561_ADDR, (u8[]){C_TSL2561_CMD_REG | C_TSL2561_CMD_ADDR_TM, C_TSL2561_INTG_MED}, 2, 0);
	// configure the interrupt to fire each time
	IIC_TX(C_TSL2561_ADDR, (u8[]){C_TSL2561_CMD_REG | C_TSL2561_CMD_ADDR_INT, C_TSL2561_INT_INTO_LVL | C_TSL2561_INT_PST_0}, 2, 0);
	/*
	IIC_TX(C_TSL2561_ADDR, (u8[]){C_TSL2561_CMD_REG | C_TSL2561_CMD_ADDR_CR}, 1, 1);
	u8 r = 0;
	IIC_RX(C_TSL2561_ADDR, &r, 1);

	if (r != 0x3u)
	{
		while(1u) ;
	}
	*/
	while (1U)
	{
		u8 iic_act = false;

		// main loop timer overflow
		if (t1_count - last_t1_count != 0U)
		{
			loop_overrun = 1U;
			M_LOOP_OVER();
		}

		// wait
		while (t1_count - last_t1_count == 0U)
		{
			// wfi stops the t1 system timer from time to time, so do not use it
		}
		M_MAIN_START();
		// interrupt pin signals that the register should be read
		if ((GPIOC->INDR & GPIO_Pin_4) == 0)
		{
			u8 tmp[5u] = {0};
			IIC_TX(C_TSL2561_ADDR, (u8[]){C_TSL2561_CMD_REG | C_TSL2561_CMD_CLEAR | C_TSL2561_CMD_BLOCK | (C_TSL2561_CMD_ADDR_D0L - 1u)}, 1, 1);
			IIC_RX(C_TSL2561_ADDR, tmp, 5u, 0);
			// ADC order with block read is reversed?
			PetitInputRegisters[1u] = (u16)((u16)tmp[1u] | ((u16)tmp[2u] << 8u));;
			PetitInputRegisters[0u] = (u16)((u16)tmp[3u] | ((u16)tmp[4u] << 8u));
			iic_act = true;
			PetitRegisters[0u]++;
		}

		// process modbus
		PETIT_MODBUS_Process(&Petit);


		// increment by one to indicate one execution cycle
		last_t1_count += 1U;
		M_MAIN_END();
	}
}
