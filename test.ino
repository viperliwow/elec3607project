#include <Servo.h>
#define Moisture A8 //定义AO 引脚 为 IO-A0  
#include <LiquidCrystal.h>
#define blueToothSerial   Serial2    //set serial2(USART1) as blueToothSerial
char rainbuffer[] ="0123456789";
char recvChar;
int rainhour;
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);           // select the pins used on the LCD panel
Servo servo29;

int lcd_key     = 0;
int adc_key_in  = 0;
#define Moisture A8
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

const int TrigPin=22;
const int EchoPin=24;


float distance;
int a;
float leng=30.0;
float Water_remain;
float sum;
float cm;
int Position;
float Moisture_value;

int read_LCD_buttons(){               // read the buttons
    adc_key_in = analogRead(0);       // read the value from the sensor 
    if (adc_key_in > 1000) return btnNONE;
     else if (adc_key_in < 50)   return btnRIGHT;  
       else if (adc_key_in < 380)  return btnUP; 
        else if (adc_key_in < 790)  return btnDOWN;   
         else if (adc_key_in < 1000)  return btnLEFT; 

    return btnNONE;                // when all others fail, return this.
}

void setup() {
  lcd.begin(18, 2);               // start the library
  lcd.setCursor(0,0);             // set the LCD cursor   position 
  lcd.print("Push the buttons");  // print a simple message on the LCD
 Serial.begin(9600);
 pinMode(TrigPin,OUTPUT);
 pinMode(EchoPin,INPUT);
 servo29.attach(30);
 pinMode(Moisture, INPUT);//定义A0为输入模式  
  setupBlueToothConnection();
  //initalize TC
  pmc_set_writeprotect(false);
  pmc_enable_periph_clk(ID_TC0);
  TC_Configure(TC1, 0, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK4);
  const uint32_t rc = VARIANT_MCK / 128 / 9600;
  TC_SetRC(TC1, 0, rc);
  TC_Start(TC1, 0);
  //enable TC interupt
  TC1->TC_CHANNEL[0].TC_IER = TC_IER_CPCS;
  NVIC_EnableIRQ(TC0_IRQn); 
}

void loop() {
   Serial.print("rain ");
   Serial.println(rainhour);
   Serial.print("distance  ");
   Serial.println(distance); 
   Serial.print("moisture  ");
   Serial.println(Moisture_value);
  
   char recvChar;
        if(blueToothSerial.available())
        {//check if there's any data sent from the remote bluetooth shield
            recvChar = blueToothSerial.read();
            //Serial.print(recvChar);
          for (int i=0;i<10;i++)
             {if (recvChar ==  rainbuffer[i])
                rainhour=i;
               }

    // Serial.println(rainhour);
        }
      
   
    

  digitalWrite(TrigPin,LOW);
  delayMicroseconds(2);
  digitalWrite(TrigPin,HIGH);
  delayMicroseconds(10);
  digitalWrite(TrigPin,LOW);
  distance = (pulseIn (EchoPin,HIGH)/58 )-1;

  delay(1000);
   // Serial.println(distance); 
  
  Water_remain=1-distance/leng; //剩余水量%

 
if (Moisture_value < 512  && rainhour>3)
 {if (Water_remain<0.3) //90度全开
    {Position= 30; }    
      else if (Water_remain<0.6)
      {Position= 20;}
       else
       Position=10;
          
 }
 else
   Position= 0;
   servo29.write(Position);
   
   lcd.setCursor(0,1);             // move to the begining of the second line
   lcd_key = read_LCD_buttons();   // read the buttons
 switch (lcd_key){               // depending on which button was pushed, we perform an action

       case btnRIGHT:{             //  push button "RIGHT" and show the word on the screen
            lcd.print("Water   ");
              lcd.setCursor(9,1);             // move cursor to second line "1" and 9 spaces over
            lcd.print(Water_remain);       // display seconds elapsed since power-up
               delay(10);
            break;
       }
       case btnLEFT:{
             lcd.print("LEFT   "); //  push button "LEFT" and show the word on the screen
             break;
       }    
       case btnUP:{
             lcd.print("UP    ");  //  push button "UP" and show the word on the screen
             break;
       }
       case btnDOWN:{
             lcd.print("DOWN  ");  //  push button "DOWN" and show the word on the screen
             break;
       }
       case btnSELECT:{
             lcd.print("SELECT");  //  push button "SELECT" and show the word on the screen
             break;
       }
       case btnNONE:{
             lcd.print("Moisture  ");  //  No action  will show "None" on the screen
              lcd.setCursor(9,1);             // move cursor to second line "1" and 9 spaces over
            lcd.print(Moisture_value);       // display seconds elapsed since power-up
               delay(10);
             break;
             
       }
   }
   
}

void setupBlueToothConnection()
{
    blueToothSerial.begin(38400);                           // Set BluetoothBee BaudRate to default baud rate 38400
    blueToothSerial.print("\r\n+STWMOD=0\r\n");             // set the bluetooth work in slave mode
    blueToothSerial.print("\r\n+STNA=garden\r\n");            // set the bluetooth name as "yuy"
    blueToothSerial.print("\r\n+STOAUT=1\r\n");             // Permit Paired device to connect me
    blueToothSerial.print("\r\n+STAUTO=0\r\n");             // Auto-connection should be forbidden here
    blueToothSerial.print("\r\n+STPIN=1234\r\n");           // set pin as 1234(default value is 0000 or 1234)
    delay(2000);                                            // This delay is required.
    blueToothSerial.print("\r\n+INQ=1\r\n");                // make the slave bluetooth inquirable
    Serial.println("The slave bluetooth is inquirable!");
    delay(2000);                                            // This delay is required.
    blueToothSerial.flush();
}

void TC1_Handler() {
  TC_GetStatus(TC1, 0);
  Moisture_value= analogRead(Moisture);
}
