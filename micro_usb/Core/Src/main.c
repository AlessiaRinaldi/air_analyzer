/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* Modifica le etichette dei PIN/PORT in base a come li hai nominati su CubeMX */
#define SENSOR_CS_PORT GPIOA
#define SENSOR_CS_PIN  GPIO_PIN_4

#define MEM_CS_PORT    GPIOB
#define MEM_CS_PIN     GPIO_PIN_12

#define DATA_SIZE      2 /* Dimensione in byte dei dati del sensore */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
extern SPI_HandleTypeDef hspi1; 

uint8_t sensor_tx_buf[DATA_SIZE] = {0x00, 0x00}; /* Buffer di trasmissione dummy per far generare il clock al DMA */
uint8_t sensor_rx_buf[DATA_SIZE] = {0};          /* Buffer in cui il DMA salva i dati letti dal sensore */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */

  /* 1. Assert del chip select del sensore di pressione (Attivo Basso) */
  HAL_GPIO_WritePin(SENSOR_CS_PORT, SENSOR_CS_PIN, GPIO_PIN_RESET);

  /* 2. Configurazione del sensore */
  /* (Esempio: invio un comando di setup iniziale, adatta i byte al datasheet del tuo sensore) */
  uint8_t config_cmd[2] = {0x20, 0x80}; 
  HAL_SPI_Transmit(&hspi1, config_cmd, 2, HAL_MAX_DELAY);

  /* 3. Inizio trasferimento SPI con DMA per la prima acquisizione */
  HAL_SPI_TransmitReceive_DMA(&hspi1, sensor_tx_buf, sensor_rx_buf, DATA_SIZE);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    /* Mette la CPU in Sleep Mode finché non si verifica un Interrupt (es. fine trasferimento DMA) */
    __WFI(); 
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USB;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/**
  * @brief  SPI Transfer Completed Callback
  * @param  hspi: SPI handle
  * @retval None
  */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
  /* Verifichiamo che l'interrupt provenga dal bus SPI corretto */
  if (hspi->Instance == SPI1) 
  {
    /* I dati acquisiti sono già disponibili in 'sensor_rx_buf' */

    /* 1. Deassert del chip select del sensore di pressione */
    HAL_GPIO_WritePin(SENSOR_CS_PORT, SENSOR_CS_PIN, GPIO_PIN_SET);

    /* 2. Assert del chip select della memoria esterna */
    HAL_GPIO_WritePin(MEM_CS_PORT, MEM_CS_PIN, GPIO_PIN_RESET);

    /* 3. Scrivo i dati in memoria */
    /* (Formattazione del comando di scrittura per la memoria SPI) */
    uint8_t mem_write_cmd[3];
    mem_write_cmd[0] = 0x02; /* Codice operativo di Write fittizio */
    mem_write_cmd[1] = sensor_rx_buf[0];
    mem_write_cmd[2] = sensor_rx_buf[1];
    
    HAL_SPI_Transmit(hspi, mem_write_cmd, 3, 10); 

    /* 4. Deassert del chip select della memoria */
    HAL_GPIO_WritePin(MEM_CS_PORT, MEM_CS_PIN, GPIO_PIN_SET);

    /* 5. Assert del chip select del sensore di pressione per preparare la prossima lettura */
    HAL_GPIO_WritePin(SENSOR_CS_PORT, SENSOR_CS_PIN, GPIO_PIN_RESET);

    /* 6. Restart del trasferimento DMA per la successiva acquisizione */
    HAL_SPI_TransmitReceive_DMA(hspi, sensor_tx_buf, sensor_rx_buf, DATA_SIZE);
  }
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  * where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */