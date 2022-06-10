#include "yqy1.1_bmi088.h"
#include "stm32f4xx.h"
#include "main.h"
#include "stdio.h"				//使用了里面的printf。编译的时候记得勾上microLib
#include "BMI088driver.h"

////#include "bmi08x.h"
////#include "bmi088.h"
////#include "bmi088_stm32.h"
///*结构体初始化配置*/
//struct bmi08x_dev dev = {
//        .accel_id = CSB1_Pin,
//        .gyro_id = CSB2_Pin,
//        .intf = BMI08X_SPI_INTF,
//        .read = &stm32_spi_read,//user_spi_read
//        .write = &stm32_spi_write,//user_spi_write
//        .delay_ms = &HAL_Delay//user_delay_milli_sec 
//		//.accel_cfg.odr = BMI08X_ACCEL_ODR_400_HZ,
//		//.accel_cfg.bw = BMI08X_ACCEL_BW_NORMAL,
//		//.accel_cfg.range = BMI088_ACCEL_RANGE_3G
//};


//struct bmi08x_int_cfg int_config;
//struct bmi08x_data_sync_cfg sync_cfg;
//int8_t rslt;
//struct bmi08x_sensor_data user_accel_bmi088;
//struct bmi08x_sensor_data user_gyro_bmi088;
//uint8_t data = 0;
//int count2=0;
//int init_flag=0;


//void yqy_bmi088_init(void)
//{
//	//初始化
//  /* Initialize bmi085 sensors (accel & gyro)*/
//  rslt = bmi088_init(&dev);
//  /* Reset the accelerometer and wait for 1 ms - delay taken care inside the function */
//  rslt = bmi08a_soft_reset(&dev);


//  /*! Max read/write length (maximum supported length is 32).
//   To be set by the user */
//  dev.read_write_len = 32;
//  /*set accel power mode */
//  dev.accel_cfg.power = BMI08X_ACCEL_PM_ACTIVE;
//  rslt = bmi08a_set_power_mode(&dev);

//  dev.gyro_cfg.power = BMI08X_GYRO_PM_NORMAL;
//  bmi08g_set_power_mode(&dev);

//  /* API uploads the bmi08x config file onto the device and wait for 150ms
//     to enable the data synchronization - delay taken care inside the function */
//  rslt = bmi088_apply_config_file(&dev);

//  /*assign accel range setting*/
//  dev.accel_cfg.range = BMI088_ACCEL_RANGE_3G;
//  /*assign gyro range setting*/
//  dev.gyro_cfg.range = BMI08X_GYRO_RANGE_2000_DPS;
//  /*! Mode (0 = off, 1 = 400Hz, 2 = 1kHz, 3 = 2kHz) */
//  sync_cfg.mode = BMI08X_ACCEL_DATA_SYNC_MODE_2000HZ;
//  rslt = bmi088_configure_data_synchronization(sync_cfg, &dev);


//  /*set accel interrupt pin configuration*/
//  /*configure host data ready interrupt */
//  int_config.accel_int_config_1.int_channel = BMI08X_INT_CHANNEL_1;
//  //int_config.accel_int_config_1.int_type = BMI08X_ACCEL_SYNC_INPUT;
//	int_config.accel_int_config_1.int_type = BMI08X_ACCEL_SYNC_DATA_RDY_INT;
//  int_config.accel_int_config_1.int_pin_cfg.output_mode = BMI08X_INT_MODE_PUSH_PULL;
//  int_config.accel_int_config_1.int_pin_cfg.lvl = BMI08X_INT_ACTIVE_HIGH;
//  int_config.accel_int_config_1.int_pin_cfg.enable_int_pin = BMI08X_ENABLE;

//  /*configure Accel syncronization input interrupt pin */
//  int_config.accel_int_config_2.int_channel = BMI08X_INT_CHANNEL_2;
//  int_config.accel_int_config_2.int_type = BMI08X_ACCEL_SYNC_DATA_RDY_INT;
//  int_config.accel_int_config_2.int_pin_cfg.output_mode = BMI08X_INT_MODE_PUSH_PULL;
//  int_config.accel_int_config_2.int_pin_cfg.lvl = BMI08X_INT_ACTIVE_HIGH;
//  int_config.accel_int_config_2.int_pin_cfg.enable_int_pin = BMI08X_ENABLE;

//  /*set gyro interrupt pin configuration*/
//  int_config.gyro_int_config_1.int_channel = BMI08X_INT_CHANNEL_3;
//  int_config.gyro_int_config_1.int_type = BMI08X_GYRO_DATA_RDY_INT;
//  int_config.gyro_int_config_1.int_pin_cfg.enable_int_pin = BMI08X_ENABLE;
//  int_config.gyro_int_config_1.int_pin_cfg.lvl = BMI08X_INT_ACTIVE_HIGH;
//  int_config.gyro_int_config_1.int_pin_cfg.output_mode = BMI08X_INT_MODE_PUSH_PULL;

//  int_config.gyro_int_config_2.int_channel = BMI08X_INT_CHANNEL_4;
//  int_config.gyro_int_config_2.int_type = BMI08X_GYRO_DATA_RDY_INT;
//  int_config.gyro_int_config_2.int_pin_cfg.enable_int_pin = BMI08X_DISABLE;
//  int_config.gyro_int_config_2.int_pin_cfg.lvl = BMI08X_INT_ACTIVE_HIGH;
//  int_config.gyro_int_config_2.int_pin_cfg.output_mode = BMI08X_INT_MODE_PUSH_PULL;

//  /* Enable synchronization interrupt pin */
//  rslt = bmi088_set_data_sync_int_config(&int_config, &dev);


//  init_flag=1;
//}

uint16_t count = 0;
#define ZERO_RANGE 200

int16_t YQYgyro[3] = { 0 };
void yqy_bmi088_exti_call_back(void)				//把这个函数放到EXTI中断回调函数内部
{
	if(count>500)
		{
			HAL_GPIO_TogglePin(LED_G_GPIO_Port, LED_G_Pin);
			count=0;
			printf("x = %3d y = %3d\n", YQYgyro[0], YQYgyro[1]);
		}
		count++;
		get_BMI088_gyro(YQYgyro);
}





