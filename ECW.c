#include "ECW.h"

void stop_watchdog()
{
    WDTCTL=WDTPW|WDTHOLD;
}

void adc_init()
{
    ADC12CTL0 = ADC12SHT0_9|ADC12ON;       // Sampling time, ADC12 on
    ADC12CTL1 = ADC12SHP;                  // Use sampling timer
    ADC12MCTL0 = ADC12INCH_6;
    ADC12CTL0 |= ADC12ENC;
    P6SEL |= BIT6 ;                        // P6.6 ADC option select
    ADC12CTL0 &= ~ADC12SC;                 // Clear the ADC start bit
}

int adc_read()
{
    ADC12CTL0 |= ADC12SC + ADC12ENC;       // Start sampling/conversion
    while (ADC12CTL1 & ADC12BUSY);         // Wait until conversion is complete
    return ADC12MEM0 & 0x0FFF;             // Mask 4 upper bits of ADC12MEM0(12 bit ADC)
}

void usart_init()
{
    P4SEL = BIT4+BIT5;
    UCA1CTL1 |= UCSWRST;                      // **Put state machine in reset**
    UCA1CTL1 |= UCSSEL_1;                     // CLK = ACLK
    UCA1BR0 = 0x03;                           // 32kHz/9600=3.41 (see User's Guide)
    UCA1BR1 = 0x00;                           //
    UCA1MCTL = UCBRS_3+UCBRF_0;               // Modulation UCBRSx=3, UCBRFx=0
    UCA1CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
}

char usart_receive_char()
{
    while(!(UCA1IFG & UCRXIFG));              // Wait for a character to be received
    char ch = UCA1RXBUF;
    UCA1IFG &= ~UCRXIFG;
    return ch;
}

void usart_transmit_char(char ch)
{
    while(!(UCA1IFG & UCTXIFG));              // Wait until transmit buffer is empty
    UCA1TXBUF = ch;
}

void usart_transmit_string(char* str)
{
    while(*str)
    {
        usart_transmit_char(*str);
        ++ str;
    }
}

void usart_transmit_int_as_string(int i)
{
    char num_string[6];
    sprintf(num_string,"%d",i);
    usart_transmit_string(num_string);
}

short isCharReceived;

char receivedChar = 0;

void usart_init_async()
{
    usart_init();
    UCA1IE |= UCRXIE;  // Enable USCI_A1 RX interrupt
    isCharReceived = 0;
    __enable_interrupt();
}

short usart_receive_char_async(char* ch)
{
    if(isCharReceived)
    {
        *ch = receivedChar;
        isCharReceived = 0;
        return 1;
    }
    else return 0;
}

void delay_us(int us)
{
    while(us --) __delay_cycles(1);
}

void delay_ms(int ms)
{
    while(ms --) __delay_cycles(1087);
}

void delay_s(int s)
{
    while(s --) __delay_cycles(1086957);
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A1_VECTOR))) USCI_A1_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCA1IV,4))
  {
  case 0:break;                             // Vector 0 - no interrupt
  case 2:                                   // Vector 2 - RXIFG
      isCharReceived = 1;
      receivedChar = UCA1RXBUF;
      break;
  case 4:break;                             // Vector 4 - TXIFG
  default: break;
  }
}

void switch_init(){
	 __enable_interrupt();

	 Buttons_init(BUTTON_S1 );
	 Buttons_interruptEnable(BUTTON_S1 );

	 Buttons_init(BUTTON_S2 );
	 Buttons_interruptEnable(BUTTON_S2 );

	 buttonsPressed = 0;
}

void lcd_init(){
	Dogs102x64_UC1701Init();
	GrContextInit(&g_sContext, &g_sDogs102x64_UC1701);
	GrContextForegroundSet(&g_sContext, ClrBlack);
	GrContextBackgroundSet(&g_sContext, ClrWhite);
	GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
	GrClearDisplay(&g_sContext);
}

void eeprom_write_bytes(char* bytes, unsigned int num_bytes)
{
  char *Flash_ptr;                          // Initialize Flash pointer
  Flash_ptr = (char *) 0x1880;
  __disable_interrupt();
  FCTL3 = FWKEY;                            // Clear Lock bit
  FCTL1 = FWKEY+ERASE;                      // Set Erase bit
  *Flash_ptr = 0;                           // Dummy write to erase Flash seg
  FCTL1 = FWKEY+WRT;                        // Set WRT bit for write operation
  unsigned int i;
  for(i = 0 ; i < num_bytes ; ++ i)
      *Flash_ptr++ = bytes[i];              // Write value to flash
  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY+LOCK;                       // Set LOCK bit
  __enable_interrupt();
}

char eeprom_read_byte(unsigned int address)
{
  char *Flash_ptr;                          // Initialize Flash pointer
  Flash_ptr = (char *) 0x1880;
  FCTL3 = FWKEY;
  char byte = Flash_ptr[address];
  FCTL3 = FWKEY + LOCK;
  return byte;
}

void highlight_and_print_string(int column,int row,char *s){
    GrContextForegroundSet(&g_sContext, ClrWhite);
    GrContextBackgroundSet(&g_sContext, ClrBlack);

    GrStringDraw(&g_sContext,
    		s,
    		AUTO_STRING_LENGTH,
    		column,
    		row,OPAQUE_TEXT);
}

void print_char(int column,int row,const char s){
    GrContextForegroundSet(&g_sContext, ClrBlack);
    GrContextBackgroundSet(&g_sContext, ClrWhite);

    char S[2] = {s, 0};
    GrStringDraw(&g_sContext,
    		S,
    		AUTO_STRING_LENGTH,
    		column,
    		row,OPAQUE_TEXT);
}

void print_string(int column,int row,char *s){
    GrContextForegroundSet(&g_sContext, ClrBlack);
    GrContextBackgroundSet(&g_sContext, ClrWhite);

    GrStringDraw(&g_sContext,
    		s,
    		AUTO_STRING_LENGTH,
    		column,
    		row,OPAQUE_TEXT);
}

void print_int(int column,int row,int n){
    GrContextForegroundSet(&g_sContext, ClrBlack);
    GrContextBackgroundSet(&g_sContext, ClrWhite);

    char s[10];
    snprintf(s,10,"%d",n);

    GrStringDraw(&g_sContext,
    		s,
    		AUTO_STRING_LENGTH,
    		column,
    		row,TRANSPARENT_TEXT);
}


//Plots adc values on pin 6.6 on LCD
void plot(){
	unsigned int sample1,sample2,i=0;

	 Buttons_init(BUTTON_S2);
	 Buttons_interruptEnable(BUTTON_S2);


	 Buttons_init(BUTTON_S1);
	 Buttons_interruptEnable(BUTTON_S1);

     buttonsPressed = 0;

    Dogs102x6_init();
    Dogs102x6_backlightInit();
    Dogs102x6_setBacklight(11);
    Dogs102x6_setContrast(11);
    Dogs102x6_clearScreen();

    int buffer[104];
    int j;
    for(j=0; j<102; j++){
        buffer[j] = 0;
    }

    Dogs102x6_stringDraw(7, 0, "S2=Esc S1=P/R", DOGS102x6_DRAW_NORMAL);
    while(1)
    {
    	if(buttonsPressed & BUTTON_S2){
    		  _delay_ms(200);
    	      buttonsPressed=0;
    	      lcd_init(); //initialise grlib
    	      break;
    	 }
          if(i == 101)
          {
              Dogs102x6_clearScreen();
              int j;
              for(j=0; j<102; j++){
            	  buffer[j] = 0;
              }
              Dogs102x6_stringDraw(7, 0, "S2=Esc S1=P/R",DOGS102x6_DRAW_NORMAL);
              i = 0;
           }
           ADC12CTL0 |= ADC12SC + ADC12ENC;
           // Start sampling/conversion
           while (ADC12CTL1 & ADC12BUSY) __no_operation();
           sample1 = ADC12MEM0 & 0x0FFF;
           buffer[i]=sample1;
           __delay_cycles(1000000);
           ADC12CTL0 |= ADC12SC + ADC12ENC;
           // Start sampling/conversion
           while (ADC12CTL1 & ADC12BUSY) __no_operation();
           sample2 = ADC12MEM0 & 0x0FFF;
           buffer[i+1]=sample2;
           Dogs102x6_lineDraw(i,56-sample1/86,i+1,56-sample2/86,0);

           //Pause the screen
           if(buttonsPressed & BUTTON_S1){
        	   _delay_ms(200);
               buttonsPressed = 0;

               while(!(buttonsPressed & BUTTON_S1)){
            	   Dogs102x6_clearScreen();
            	   int j;
            	   for(j=0; j<104; j++){
            	       Dogs102x6_lineDraw(j,56-buffer[j]/86,j+1,56-buffer[j+1]/86,0);
            	   }
            	   Wheel_init();
            	   int pos = Wheel_getValue();
            	   Dogs102x6_lineDraw(0,(int)(pos*48)/4096+8,104,(int)(pos*48)/4096+8,0);

            	   char c1[10], c2[10];
            	   int reverse_adc = 86*(48 - (pos*48)/4096);
            	   snprintf(c1, 10,"%d", reverse_adc);
            	   Dogs102x6_stringDraw(56, 0, c1, DOGS102x6_DRAW_NORMAL);
            	   int voltage = reverse_adc*3.7/4096.0 * 1000; //Voltage in mV
            	   snprintf(c2, 10,"%d", voltage);
            	   Dogs102x6_stringDraw(0, 0, c2, DOGS102x6_DRAW_NORMAL);
            	   Dogs102x6_stringDraw(0, 28, "mV", DOGS102x6_DRAW_NORMAL);

            	   _delay_ms(500);
               }
               _delay_ms(200);
               buttonsPressed = 0;
               Dogs102x6_clearScreen();
               adc_init(); //init required to use 6.6
           }

           ++ i;


      }
}

//used to initialise clock
void Clock_init(void)
{
	UCS_setExternalClockSource(
                               32768,
                               0);

    // Set Vcore to accomodate for max. allowed system speed
    PMM_setVCore(
    		PMM_CORE_LEVEL_3
                );

    // Use 32.768kHz XTAL as reference
    UCS_LFXT1Start(
        UCS_XT1_DRIVE0,
        UCS_XCAP_3
        );


    // Set system clock to max (25MHz)
    UCS_initFLLSettle(
    	25000,
        762
        );

    SFR_enableInterrupt(
                        SFR_OSCILLATOR_FAULT_INTERRUPT
                       );

    // Globally enable interrupts
    __enable_interrupt();
}

