// address
#define C_TSL2561_ADDR (0x72)

// command register
#define C_TSL2561_CMD_REG (1u << 7u)
#define C_TSL2561_CMD_CLEAR (1u << 6u)
#define C_TSL2561_CMD_WORD (1u << 5u)
#define C_TSL2561_CMD_BLOCK (1u << 4u)
#define C_TSL2561_CMD_ADDR_CR (0x0u)
#define C_TSL2561_CMD_ADDR_TM (0x1u)
#define C_TSL2561_CMD_ADDR_TLL (0x2u)
#define C_TSL2561_CMD_ADDR_TLH (0x3u)
#define C_TSL2561_CMD_ADDR_THL (0x4u)
#define C_TSL2561_CMD_ADDR_THH (0x5u)
#define C_TSL2561_CMD_ADDR_INT (0x6u)
#define C_TSL2561_CMD_ADDR_ID (0xAu)
#define C_TSL2561_CMD_ADDR_D0L (0xCu)
#define C_TSL2561_CMD_ADDR_D0H (0xDu)
#define C_TSL2561_CMD_ADDR_D1L (0xEu)
#define C_TSL2561_CMD_ADDR_D1H (0xFu)

#define C_TSL2561_CR_OFF (0x00u)
#define C_TSL2561_CR_ON (0x03u)

#define C_TSL2561_TM_GAIN_16 (1u << 4u)
#define C_TSL2561_TM_MANUAL (1u << 3u)
#define C_TSL2561_INTG_FAST (0b00u)
#define C_TSL2561_INTG_MED (0b01u)
#define C_TSL2561_INTG_SLOW (0b10u)

#define C_TSL2561_INT_INTO_DIS (0b00u << 4u)
#define C_TSL2561_INT_INTO_LVL (0b01u << 4u)
#define C_TSL2561_INT_INTO_SMB (0b10u << 4u)
#define C_TSL2561_INT_INTO_TST (0b11u << 4u)
#define C_TSL2561_INT_PST_0 (0u)

// loop constants
#define C_SLOWER_CYCLE (4U)

// modbus timer constants 
#define C_MODBUS_CLEAR (4U)

// breakout board
//#define BOB = 1
// PCB
#define HMI_PCB = 1

#if defined(BOB) && defined(HMI_PCB)
#error "Only one board is supposed to be defined"
#endif // BOB && HMI_PCB

#if !defined(BOB) && !defined(HMI_PCB)
#error "At least one board type must be defined"
#endif

// debug pins
#if defined(BOB)
#define M_DBG_0_ACT() GPIOD->BSHR = GPIO_Pin_0;
#define M_DBG_0_INA() GPIOD->BSHR = GPIO_Pin_0 << 16U;
#define M_DBG_1_ACT() GPIOC->BSHR = GPIO_Pin_0;
#define M_DBG_1_INA() GPIOC->BSHR = GPIO_Pin_0 << 16U;
#define M_DBG_2_ACT() GPIOC->BSHR = GPIO_Pin_3;
#define M_DBG_2_INA() GPIOC->BSHR = GPIO_Pin_3 << 16U;
#define M_DBG_3_ACT() GPIOC->BSHR = GPIO_Pin_5;
#define M_DBG_3_INA() GPIOC->BSHR = GPIO_Pin_5 << 16U;
#define M_DBG_4_ACT() GPIOC->BSHR = GPIO_Pin_6;
#define M_DBG_4_INA() GPIOC->BSHR = GPIO_Pin_6 << 16U;
#define M_DBG_5_ACT() GPIOC->BSHR = GPIO_Pin_7;
#define M_DBG_5_INA() GPIOC->BSHR = GPIO_Pin_7 << 16U;
#define M_DBG_6_ACT() GPIOD->BSHR = GPIO_Pin_2;
#define M_DBG_6_INA() GPIOD->BSHR = GPIO_Pin_2 << 16U;
#define M_DBG_7_ACT() GPIOD->BSHR = GPIO_Pin_3;
#define M_DBG_7_INA() GPIOD->BSHR = GPIO_Pin_3 << 16U;
#endif // BOB

// debug output functions
#if defined(HMI_PCB)
#define M_T1_START()
#define M_T1_END()
#define M_USART_START()
#define M_USART_END()
#define M_LOOP_OVER()
#define M_MAIN_START()
#define M_MAIN_END()
#endif // HMI_PCB

#if defined(BOB)
#define M_T1_START() M_DBG_2_ACT()
#define M_T1_END() M_DBG_2_INA()
#define M_USART_START() M_DBG_1_ACT()
#define M_USART_END() M_DBG_1_INA()
#define M_LOOP_OVER() M_DBG_7_ACT()
#define M_MAIN_START() M_DBG_0_ACT()
#define M_MAIN_END() M_DBG_0_INA()
#endif // BOB