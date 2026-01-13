/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// 엄지
#define F1_GPIO_Port GPIOC
#define F1_Pin       GPIO_PIN_0

// 검지
#define F2_GPIO_Port GPIOC
#define F2_Pin       GPIO_PIN_1

// 중지
#define F3_GPIO_Port GPIOC
#define F3_Pin       GPIO_PIN_2

// 약지+새끼
#define F4_GPIO_Port GPIOC
#define F4_Pin       GPIO_PIN_3
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
int __io_putchar(int ch) {
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}

uint32_t adc_raw = 0;        // 0 ~ 4095
float pot_angle_deg = 0.0f;  // 0.0 ~ 300.0도

// 엔코더 2 (TIM2) 변수
volatile uint32_t uhCapture_Period2 = 0;
volatile uint32_t uhCapture_Pulse2 = 0;
volatile uint8_t is_capture_done2 = 0;
float angle_deg2 = 0.0f;

// 엔코더 3 (TIM3) 변수
volatile uint32_t uhCapture_Period3 = 0;
volatile uint32_t uhCapture_Pulse3 = 0;
volatile uint8_t is_capture_done3 = 0;
float angle_deg3 = 0.0f;

// 아두이노 전송 패킷 (15바이트: 헤더2 + 각도6 + 손가락4 + RTT2 + 체크섬1)
uint8_t tx_packet[15];

//영점 조절용 오프셋 변수 -> B1 버튼을 눌렀을 때 현재 각도로 영점 설정
volatile float angle_offset1 = 0.0f;
volatile float angle_offset2 = 0.0f;
volatile float angle_offset3 = 0.0f;

// 패킷 전송 시작 상태 변수
volatile uint8_t start_tx = 0;

// [Full E2E] RTT 측정용 변수
volatile uint32_t rtt_sum = 0;
volatile uint32_t rtt_count = 0;
volatile uint16_t last_rtt = 0;  // 16비트로 변경 (패킷에 담기 위해)

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void ReadAllFingersDebounced(uint8_t *f1, uint8_t *f2, uint8_t *f3, uint8_t *f4)
{
    const uint8_t SAMPLE_COUNT = 3;   // 샘플링 횟수   10->3 ★★★
    const uint8_t LOW_THRESHOLD = 2;   // LOW가 몇 번 이상이면 "진짜 눌림"   7->2 ★★★

    uint8_t low_count[4] = {0, 0, 0, 0};

    for (uint8_t i = 0; i < SAMPLE_COUNT; i++)
    {
        // 4개 핀 동시 샘플링
        if (HAL_GPIO_ReadPin(F1_GPIO_Port, F1_Pin) == GPIO_PIN_RESET) low_count[0]++;
        if (HAL_GPIO_ReadPin(F2_GPIO_Port, F2_Pin) == GPIO_PIN_RESET) low_count[1]++;
        if (HAL_GPIO_ReadPin(F3_GPIO_Port, F3_Pin) == GPIO_PIN_RESET) low_count[2]++;
        if (HAL_GPIO_ReadPin(F4_GPIO_Port, F4_Pin) == GPIO_PIN_RESET) low_count[3]++;

         // HAL_Delay(1);  // 1ms 간격 (총 10ms) ★★★
    }

    // 결과 판정: LOW가 7회 이상이면 터치됨(1), 아니면 미터치(0)
    *f1 = (low_count[0] >= LOW_THRESHOLD) ? 1 : 0;
    *f2 = (low_count[1] >= LOW_THRESHOLD) ? 1 : 0;
    *f3 = (low_count[2] >= LOW_THRESHOLD) ? 1 : 0;
    *f4 = (low_count[3] >= LOW_THRESHOLD) ? 1 : 0;
}

// ADC 값 읽기 함수
uint32_t ReadADC(void)
{
	HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
    {
        return HAL_ADC_GetValue(&hadc1);
    }
    HAL_ADC_Stop(&hadc1);
    return 0;
}
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
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1); // 엔코더 2
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);
  HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_1); // 엔코더 3
  HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_2);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  // 가변저항 각도 계산-> 엔코더1 대용
	  adc_raw = ReadADC();
	  if (adc_raw > 4095) adc_raw = 4095;
	  pot_angle_deg = ((float)adc_raw * 300.0f) / 4095.0f;

	 // printf("ADC = %4lu, PotAngle = %6.2f deg\r\n", adc_raw, pot_angle_deg);

	// 엔코더 2 각도 계산
	if (is_capture_done2) {
	  is_capture_done2 = 0;
	  if (uhCapture_Period2 > 0) {
	  float useful_pulse_us = (float)uhCapture_Pulse2 - 260.0f;
	  float time_range_us = 8600.0f;
	  if (useful_pulse_us < 0.0f) useful_pulse_us = 0.0f;
	  if (useful_pulse_us > time_range_us) useful_pulse_us = time_range_us;
	  angle_deg2 = ((time_range_us - useful_pulse_us) / time_range_us) * 360.0f;
	  }
	}

	// 엔코더 3 각도 계산
	if (is_capture_done3) {
	  is_capture_done3 = 0;
	  if (uhCapture_Period3 > 0) {
	  float useful_pulse_us = (float)uhCapture_Pulse3 - 260.0f;
	  float time_range_us = 8600.0f;
	  if (useful_pulse_us < 0.0f) useful_pulse_us = 0.0f;
	  if (useful_pulse_us > time_range_us) useful_pulse_us = time_range_us;
	  angle_deg3 = ((time_range_us - useful_pulse_us) / time_range_us) * 360.0f;
	  }
	}

	if(start_tx)
	{
	// angle_offset을 이용해 최종 각도 계산
	float angle_rel1 = pot_angle_deg - angle_offset1;
	float angle_rel2 = angle_deg2 - angle_offset2;
	float angle_rel3 = angle_deg3 - angle_offset3;
	angle_rel3 = -angle_rel3;

	// 각도가 0~360도를 넘어가는 경우 보정
	/* *** 각도가 +-180도를 넘어갔을 때 값을 고정시킬지 또는 현재처럼 순환시킬지 결정 *** */
	if (angle_rel1 > 180.0f) angle_rel1 -= 360.0f;
	if (angle_rel1 < -180.0f) angle_rel1 += 360.0f;

	if (angle_rel2 > 180.0f) angle_rel2 -= 360.0f;
	if (angle_rel2 < -180.0f) angle_rel2 += 360.0f;

	if (angle_rel3 > 180.0f) angle_rel3 -= 360.0f;
	if (angle_rel3 < -180.0f) angle_rel3 += 360.0f;

	// 각도를 정수로 만들어 한 각도당 2바이트 씩 보낼 예정 (ex: 128.2 -> 1282)
	int16_t angle1_int = (int16_t)(angle_rel1 * 10.0f);
	int16_t angle2_int = (int16_t)(angle_rel2 * 10.0f);
	int16_t angle3_int = (int16_t)(angle_rel3 * 10.0f);

    // 손가락 상태 읽기 (0 또는 1) - [최적화] 4개 동시 디바운싱
    uint8_t finger1, finger2, finger3, finger4;
    ReadAllFingersDebounced(&finger1, &finger2, &finger3, &finger4);

	tx_packet[0] = 0xAA; // 시작 바이트 1 - 패킷 헤더(0b10101010)
	tx_packet[1] = 0x55; // 시작 바이트 2 - 패킷 헤더(0b01010101)

	// 엔코터 데이터 패킷
	tx_packet[2] = (angle1_int >> 8) & 0xFF; // 각도1 High
	tx_packet[3] = angle1_int & 0xFF;        // 각도1 Low
	tx_packet[4] = (angle2_int >> 8) & 0xFF; // 각도2 High
	tx_packet[5] = angle2_int & 0xFF;        // 각도2 Low
	tx_packet[6] = (angle3_int >> 8) & 0xFF; // 각도3 High
	tx_packet[7] = angle3_int & 0xFF;        // 각도3 Low

    // 손가락 5개 (1바이트씩)
    tx_packet[8]  = finger1;
    tx_packet[9]  = finger2;
    tx_packet[10] = finger3;
    tx_packet[11] = finger4;

    // [Full E2E] RTT 값 삽입 (이전 측정값, Big-endian)
    tx_packet[12] = (last_rtt >> 8) & 0xFF;  // RTT High
    tx_packet[13] = last_rtt & 0xFF;         // RTT Low

	// Checksum 계산 (Byte 2~7)
	uint8_t checksum = 0;
	for (int i = 2; i < 14; i++) {
		checksum += tx_packet[i];
	}
	tx_packet[14] = checksum; // 마지막 바이트에 Checksum 추가

	// === [Full E2E] RTT 측정 시작 ===
	uint32_t send_tick = HAL_GetTick();

	// printf("angle1 = %.1f deg, angle2 = %.1f deg, angle3 = %.1f deg | F[%d,%d,%d,%d]\r\n", angle_rel1, angle_rel2, angle_rel3, finger1, finger2, finger3, finger4);

	// --- USART1를 통해 아두이노로 패킷 전송 ---
	HAL_UART_Transmit(&huart1, tx_packet, 15, 100); // 15바이트 전송

	// === [Full E2E] ACK 수신 대기 (타임아웃 80ms) ===
	uint8_t ack = 0;
	HAL_StatusTypeDef status = HAL_UART_Receive(&huart1, &ack, 1, 80);

	if (status == HAL_OK && ack == 0x06) {
	// ACK 수신 성공 → RTT 계산
		uint32_t recv_tick = HAL_GetTick();
		last_rtt = (uint16_t)(recv_tick - send_tick);
		rtt_sum += last_rtt;
		rtt_count++;
	}
	}
	else
	{

	}

	HAL_Delay(10); // 10 -> 1 ★★★
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_84CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 83;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
  sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;
  sSlaveConfig.TriggerPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sSlaveConfig.TriggerFilter = 0;
  if (HAL_TIM_SlaveConfigSynchro(&htim2, &sSlaveConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
  sConfigIC.ICSelection = TIM_ICSELECTION_INDIRECTTI;
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 83;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
  sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;
  sSlaveConfig.TriggerPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sSlaveConfig.TriggerFilter = 0;
  if (HAL_TIM_SlaveConfigSynchro(&htim3, &sSlaveConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
  sConfigIC.ICSelection = TIM_ICSELECTION_INDIRECTTI;
  if (HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PC0 PC1 PC2 PC3
                           PC4 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
// Input Capture 콜백 함수 (모든 타이머가 이 함수를 공유)
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
  // 엔코더 2 (TIM2)
  if (htim->Instance == TIM2) {
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
      uhCapture_Period2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
      is_capture_done2 = 1;
    } else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
      uhCapture_Pulse2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
    }
  }

  // 엔코더 3 (TIM3)
  else if (htim->Instance == TIM3) {
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
      uhCapture_Period3 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
      is_capture_done3 = 1;
    } else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
      uhCapture_Pulse3 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
    }
  }
}

// B1 버튼 인터럽트 발생 시
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == B1_Pin) {
        // 버튼이 눌린 순간의 값을 기준값으로 설정
    	angle_offset1 = pot_angle_deg;
    	angle_offset2 = angle_deg2;
        angle_offset3 = angle_deg3;

        // 처음 눌렸을 때 송신 시작
        if (start_tx == 0) {
            start_tx = 1;
            printf("Zero set! offset1 = %.1f deg, offset2 = %.1f deg, offset3 = %.1f deg\r\n", pot_angle_deg, angle_offset2, angle_offset3);
            printf("TX START\r\n");

        } else {
            // 이후에 다시 눌렀을 때는 영점만 재설정 (원하는 동작)
        	printf("Zero re-set! offset1 = %.1f deg, offset2 = %.1f deg, offset3 = %.1f deg\r\n", pot_angle_deg, angle_offset2, angle_offset3);
        }
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
  *         where the assert_param error has occurred.
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
