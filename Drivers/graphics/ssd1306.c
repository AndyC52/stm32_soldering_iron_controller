#include "ssd1306.h"
#define SH1106_FIX




static uint8_t OledContrast;
volatile uint8_t OledBuffer[128*8]; // 128x64 1BPP OLED
volatile uint8_t *OledDmaBf;
volatile oled_status_t oled_status=oled_idle;
static SPI_HandleTypeDef *spi_device;

#ifdef Soft_SPI

void Enable_Soft_SPI_SPI(void){
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	HAL_SPI_MspDeInit(&SPI_DEVICE);
	 /*Configure GPIO pins : SCK_Pin */
	 GPIO_InitStruct.Pin = 	SCK_Pin;
	 GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	 GPIO_InitStruct.Pull = GPIO_NOPULL;
	 GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	 HAL_GPIO_Init(SCK_GPIO_Port, &GPIO_InitStruct);

	 /*Configure GPIO pins : SDO_Pin */
	 GPIO_InitStruct.Pin = 	SDO_Pin;
	 GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	 GPIO_InitStruct.Pull = GPIO_NOPULL;
	 GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	 HAL_GPIO_Init(SDO_GPIO_Port, &GPIO_InitStruct);
}

#define spidelay() asm("NOP")
//#define spidelay() HAL_Delay(1)
void spi_send(uint8_t SPIData){
	unsigned char SPICount;                               // Counter used to clock out the data

	for (SPICount = 0; SPICount < 8; SPICount++)          // Prepare to clock out the Address byte
		  {
		    if (SPIData & 0x80)                                 // Check for a 1
		    	HAL_GPIO_WritePin(SDO_GPIO_Port, SDO_Pin, GPIO_PIN_SET);
		    else
		    	HAL_GPIO_WritePin(SDO_GPIO_Port, SDO_Pin, GPIO_PIN_RESET);
		    HAL_GPIO_WritePin(SCK_GPIO_Port, SCK_Pin, GPIO_PIN_SET);
		    HAL_GPIO_WritePin(SCK_GPIO_Port, SCK_Pin, GPIO_PIN_RESET);
		    SPIData <<= 1;                                      // Rotate to get the next bit

		  }
}


void write_data(uint8_t *data) {
	uint8_t spiBytes=0;

	Oled_Set_CS();
	Oled_Set_DC();
	Oled_Clear_CS();
	while(spiBytes<128){
		spi_send(*data++);
		spiBytes++;
	}

	Oled_Set_CS();
}


void write_byte(uint8_t data) {

	Oled_Set_CS();
	Oled_Set_DC();
	Oled_Clear_CS();
	spi_send(data);

	Oled_Set_CS();
}


void write_cmd(uint8_t data) {
	Oled_Set_CS();	spidelay();
	Oled_Clear_DC();	spidelay();
	Oled_Clear_CS();	spidelay();
	spi_send(data);
	Oled_Set_CS();
}


void update_display( void )
{
	unsigned int p;
	for(p=0;p<8;p++){
		write_cmd(0xB0|p);

#ifdef SH1106_FIX
		write_cmd(0x02);

#else
		write_cmd(0x00);
#endif
		write_cmd(0x10);
		write_data(OledBuffer + p * 128);
   }
}


void send_display_bf(uint8_t *oled_buffer)
{
   unsigned int p;
   for(p=0;p<8;p++){
	   write_cmd(0xB0|p);

#ifdef SH1106_FIX
      write_cmd(0x02);
#else
      write_cmd(0x00);
#endif
      write_cmd(0x10);
      write_data(oled_buffer + p * 128);
   }
}

#else

// Send data in blocking mode (Not used in screen update)
void write_data(uint8_t *data) {
	while(oled_status!=oled_idle);	// Wait for DMA to finish
	Oled_Set_CS();
	Oled_Set_DC();
	Oled_Clear_CS();
	HAL_SPI_Transmit(spi_device, data, 128, 1000);
	Oled_Set_CS();
}

// Send command in blocking mode
void write_cmd(uint8_t data) {
	while(oled_status==oled_sending_data);	//Wait for DMA to finish
	// Now, else we are in idle (oled_idle) or DMA wants to send a cmd (oled_sending_cmd)

	Oled_Set_CS();
	Oled_Clear_DC();
	Oled_Clear_CS();
	HAL_SPI_Transmit(spi_device, &data, 1, 1000);
	Oled_Set_CS();
}

// Trigger DMA
void update_display( void ){
		if(oled_status!=oled_idle) { return; }		// If OLED busy, skip update
		HAL_SPI_TxCpltCallback(spi_device); // Call the DMA callback function to send the frame
}

#endif


void pset(UG_S16 x, UG_S16 y, UG_COLOR c){
   unsigned int p;

   if ( x > 127 ) return;
   p = y>>3; // :8
   p = p<<7; // *128
   p +=x;

   if( c )
   {
      OledBuffer[p] |= 1<<(y%8);
   }
   else
   {
      OledBuffer[p] &= ~(1<<(y%8));
   }
}



void setContrast(uint8_t value) {
	write_cmd(0x81);         // Set Contrast Control
	write_cmd(value);         //   Default => 0xFF
	OledContrast = value;
}


uint8_t getContrast() {
	return OledContrast;
}


#ifdef Soft_SPI

void ssd1306_init(void){

#else

void ssd1306_init(SPI_HandleTypeDef *hspi){
	spi_device = hspi;
#endif

	Oled_Clear_RES();//RST
	HAL_Delay(100);
	Oled_Set_RES();//RST
	HAL_Delay(100);

   write_cmd(0xAE| 0x00);  // Display Off (0x00/0x01)

   write_cmd(0xD5);         // Set Display Clock Divide Ratio / Oscillator Frequency
   //write_cmd(0x80);         // Set Clock as 100 Frames/Sec
   write_cmd(0b11110000);         // Set Clock as 100 Frames/Sec

   write_cmd(0xA8);         // Set Multiplex Ratio
   write_cmd(0x3F);         //   Default => 0x3F (1/64 Duty)

   write_cmd(0xD3);         // Set Display Offset
   write_cmd(0x00);         //   Default => 0x00

   write_cmd(0x40|0x00);   // Set Display Start Line

   write_cmd(0x8D);         // Set Charge Pump
   write_cmd(0x10|0x04);   //   Default => 0x10
  // write_cmd_2(0x10|0x04);   //   Default => 0x10

   write_cmd(0x20);         // Set Memory Addressing Mode
   write_cmd(0x02);         //   Default => 0x02

   write_cmd(0xA0|0x01);   // Set Segment Re-Map

   write_cmd(0xC0|0x08);   // Set COM Output Scan Direction

   write_cmd(0xDA);         // Set COM Pins Hardware Configuration
   write_cmd(0x02|0x10);   //   Default => 0x12 (0x10)

   setContrast(0xFF);		// Init in max contrast

   write_cmd(0xD9);         // Set Pre-Charge Period
   write_cmd(0x22);         //   Default => 0x22 (2 Display Clocks [Phase 2] / 2 Display Clocks [Phase 1])
 //  write_cmd_2(0xF1);         //   Default => 0x22 (2 Display Clocks [Phase 2] / 2 Display Clocks [Phase 1])

   write_cmd(0xDB);         // Set VCOMH Deselect Level
   write_cmd(0x30);         //   Default => 0x20 (0.77*VCC)

   write_cmd(0xA4|0x00);   // Set Entire Display On/Off

   write_cmd(0xA6|0x00);   // Set Inverse Display On/Off

   write_cmd(0xAE|0x01);   // Set Display On/Off
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *_hspi){
	static uint8_t OledRow=0;

	if(_hspi == spi_device){
		if(OledRow>7){
			OledRow=0;					// We sent the last row of the OLED buffer data
			oled_status=oled_idle;
			return;						// Return without retriggering DMA.
		}
		oled_status=oled_sending_cmd;
		write_cmd(0xB0|OledRow);
		#ifdef SH1106_FIX
		write_cmd(0x02);
		#else
		write_cmd(0x00);
		#endif
		write_cmd(0x10);
		oled_status=oled_sending_data;
		Oled_Clear_CS();
		Oled_Set_DC();

		// Send next OLED row
		if(HAL_SPI_Transmit_DMA(spi_device, (uint8_t*)&OledBuffer[0]+(OledRow++ * 128), 128) != HAL_OK){
			Error_Handler();
		}
	}
}
