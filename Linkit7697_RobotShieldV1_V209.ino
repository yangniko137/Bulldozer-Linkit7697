// 最後編輯 2019-11-25 by Niko Yang 
// The pin on ROBOT Shield 
// P8:Left_servo,
// P11:Right_servo,
// P9:Bucket_servo,
// P2:Front_led,
// P7:Rear_led, 
// P16:Nobattery_led, 
// P15:Battery_sensor_pin,
// P14:Buzzer_pin, 
// P6: User_sw, 
// P7: Users LED,

// 最後編輯 2017-10-23 by ShinWei Chiou
// P2 : Left 左輪，P3 : Right 右輪，P4 : Bucket 鏟斗 P5 : Rolling Bucket
// 最後編輯 2017-11-03 by ShinWei Chiou
// 增加 EEPROM 紀錄校正參數

#include <Servo.h>
#include <EEPROM.h>
#include <LRemote.h>
#include <SoftwareSerial.h>

// Define Pins
#define Front_led 2
#define Rear_led 7
#define Nobattery_led 16
#define Buzzer_pin 14
#define User_sw 6 
#define Battery_sensor_pin 15

int Speed1 = 25;
int Speed2 = 50;
int Turbosw = 0;
int LeftServoCen = 90;
int RightServoCen = 90;
int BucketServoCen = 86;
int BucketServoCenmax = 86;

// Battery (Based on ROBOT shield that 3.9V is low battery)
const float BatteryLowVol = 3.9;
float BatteryVol = 0;
int BatteryCount = 0;
int BatteryLow = 0;

LRemoteLabel  Titlelabel;
LRemoteButton forwardbutton;
LRemoteButton backwardbutton;
LRemoteButton turnleftbutton;
LRemoteButton turnrightbutton;
LRemoteSwitch turboswitchButton;
LRemoteSwitch LightsButton;
LRemoteCircleButton BuzzerButton;
LRemoteCircleButton bucketup;
LRemoteCircleButton bucketdown;
LRemoteSlider leftslider;
LRemoteSlider rightslider;
LRemoteSlider bucketslider;
LRemoteLabel batterylabel;
LRemoteLabel appverlabel;

// Servo Trim Value
int Running_Servo_Trim [3];

// create Servo object to control a servo
Servo BucketServo;
Servo RightServo;
Servo LeftServo;

 void Move_Bucket()
{
   if(BucketServoCen > BucketServoCenmax){
      BucketServoCen = BucketServoCenmax;
   }
   else if(BucketServoCen <= 0){
      BucketServoCen = 0;
   }
      BucketServo.write(BucketServoCen);
}

void Move_Forward()
{
  if (Turbosw == 1)
  {
    LeftServo.write(LeftServoCen - Speed2);
    RightServo.write(RightServoCen + Speed2);
  }
  else
  {
    LeftServo.write(LeftServoCen - Speed1);
    RightServo.write(RightServoCen + Speed1);
  }
}

void Move_Backward()
{
  if (Turbosw == 1)
  {
    LeftServo.write(LeftServoCen + Speed2);
    RightServo.write(RightServoCen - Speed2);
  }
  else
  {
    LeftServo.write(LeftServoCen + Speed1);
    RightServo.write(RightServoCen - Speed1);
  }
}

void Move_TurnLeft()
{
  if (Turbosw == 1)
  {
    LeftServo.write(LeftServoCen - Speed2);
    RightServo.write(RightServoCen - Speed2);
  }
  else
  {
    LeftServo.write(LeftServoCen - Speed1);
    RightServo.write(RightServoCen - Speed1);
  }
}

void Move_TurnRight()
{
  if (Turbosw == 1)
  {
    LeftServo.write(LeftServoCen + Speed2);
    RightServo.write(RightServoCen + Speed2);
  }
  else
  {
    LeftServo.write(LeftServoCen + Speed1);
    RightServo.write(RightServoCen + Speed1);
  }
}

void Motor_Break()
{
  LeftServo.write(LeftServoCen);
  RightServo.write(RightServoCen);
}

// Buzzer Sound
void Buzzer_Sound()
{
  for (int i = 0; i < 400; i++)
  {
    digitalWrite(Buzzer_pin, HIGH);
    delayMicroseconds(i);
    digitalWrite(Buzzer_pin, LOW);
    delayMicroseconds(i);
  }
}

/*--------------------------------------------------------------------------------------------*/
void setup() {
// Initialize serial communications at 9600 bps:
   Serial.begin(9600);

// Servo PIN Set
  RightServo.attach(11);
  LeftServo.attach(8);
  BucketServo.attach(9);

  // PIN mode set
  pinMode(Front_led, OUTPUT);
  pinMode(Rear_led, OUTPUT);
  pinMode(Nobattery_led, OUTPUT);
  pinMode(Buzzer_pin, OUTPUT);
  pinMode(User_sw, INPUT);
  pinMode(Battery_sensor_pin, INPUT);
   
  // Read EEPROM Trim
  Running_Servo_Trim[0] = (int8_t)EEPROM.read(0); // LeftServo
  Running_Servo_Trim[1] = (int8_t)EEPROM.read(1); // RightServo
  Running_Servo_Trim[2] = (int8_t)EEPROM.read(2); // BucketServo
  
  // Initialize Servo
  LeftServoCen = 90 + Running_Servo_Trim[0];
  RightServoCen = 90 + Running_Servo_Trim[1];
  BucketServoCen = 86 + Running_Servo_Trim[2];
  BucketServoCenmax = 86 + Running_Servo_Trim[2];
  LeftServo.write(LeftServoCen);
  RightServo.write(RightServoCen);
  BucketServo.write (BucketServoCenmax);

/*--------------------------------------------------------------------------------------------*/
  // Initialize BLE subsystem & get BLE address
  LBLE.begin();
  while (!LBLE.ready()) {
    delay(100);
  }

  Serial.print("Device Address = [");
  LBLEAddress ble_address;
  String Bulldozer_address;

  ble_address = LBLE.getDeviceAddress();
  Bulldozer_address = ble_address.toString();
  Serial.print(Bulldozer_address);
  Serial.println("]");

  String Bulldozer_name;
  Bulldozer_name = "Bulldozer-" + Bulldozer_address.substring(0, 2) + Bulldozer_address.substring(3, 5);

  // Setup the Remote Control's Name
  LRemote.setName(Bulldozer_name);
/*--------------------------------------------------------------------------------------------*/

  LRemote.setOrientation(RC_PORTRAIT);
  LRemote.setGrid(3, 7);

  // Add a text Battery Voltage label
  batterylabel.setText("5.0V");
  batterylabel.setPos(1, 1);
  batterylabel.setSize(1, 1);
  batterylabel.setColor(RC_GREY);
  LRemote.addControl(batterylabel);

  // Add a Forward button
  forwardbutton.setText("前進");
  forwardbutton.setPos(1, 0);
  forwardbutton.setSize(1, 1);
  forwardbutton.setColor(RC_BLUE);
  LRemote.addControl(forwardbutton);

  // Add a Backward button
  backwardbutton.setText("後退");
  backwardbutton.setPos(1, 2);
  backwardbutton.setSize(1, 1);
  backwardbutton.setColor(RC_BLUE);
  LRemote.addControl(backwardbutton);

  // Add a TurnLeft button
  turnleftbutton.setText("左轉");
  turnleftbutton.setPos(0, 1);
  turnleftbutton.setSize(1, 1);
  turnleftbutton.setColor(RC_BLUE);
  LRemote.addControl(turnleftbutton);

  // Add a TurnRight button
  turnrightbutton.setText("右轉");
  turnrightbutton.setPos(2, 1);
  turnrightbutton.setSize(1, 1);
  turnrightbutton.setColor(RC_BLUE);
  LRemote.addControl(turnrightbutton);

  // Add Buzzer Switch 
  BuzzerButton.setText("喇叭");
  BuzzerButton.setPos(0, 3);
  BuzzerButton.setSize(1, 1);  BuzzerButton.setColor(RC_ORANGE);
  LRemote.addControl(BuzzerButton);
  
  // Add Lights Switch 
  LightsButton.setText("燈光");
  LightsButton.setPos(1, 3);
  LightsButton.setSize(1, 1);
  LightsButton.setColor(RC_YELLOW);
  LRemote.addControl(LightsButton);
     
  // Add an Turbo Switch 
  turboswitchButton.setText("加速");
  turboswitchButton.setPos(2, 3);
  turboswitchButton.setSize(1, 1);
  turboswitchButton.setColor(RC_PINK);
  LRemote.addControl(turboswitchButton);

  // Add a Bucket UP button 
  bucketup.setText("鏟钭上升");
  bucketup.setPos(0, 4);
  bucketup.setSize(1, 1);
  bucketup.setColor(RC_GREEN);
  LRemote.addControl(bucketup);

  // Add a Bucket DOWN button 
  bucketdown.setText("鏟钭下降");
  bucketdown.setPos(0, 5);
  bucketdown.setSize(1, 1);
  bucketdown.setColor(RC_GREEN);
  LRemote.addControl(bucketdown);

  // Add a Left slider (if you do the calibration by front side then change Text as 左輪校正)
  leftslider.setText("右輪校正(-20~20)");
  leftslider.setPos(1, 5);
  leftslider.setSize(2, 1);
  leftslider.setColor(RC_ORANGE);
  leftslider.setValueRange(-20, 20, Running_Servo_Trim[0]);
  LRemote.addControl(leftslider);

  // Add a Right slider (if you do the calibration by front side then change Text as 右輪校正)
  rightslider.setText("左輪校正(-20~20)");
  rightslider.setPos(1, 6);
  rightslider.setSize(2, 1);
  rightslider.setColor(RC_ORANGE);
  rightslider.setValueRange(-20, 20, Running_Servo_Trim[1]);
  LRemote.addControl(rightslider);

  // Add a Bucket slider
  bucketslider.setText("鏟钭校正(-8~8)");
  bucketslider.setPos(1, 4);
  bucketslider.setSize(2, 1);
  bucketslider.setColor(RC_GREEN);
  bucketslider.setValueRange(-8, 8, Running_Servo_Trim[2]);
  LRemote.addControl(bucketslider);

  // Add an APP verizon label
  appverlabel.setText("V0.9");
  appverlabel.setPos(0, 6);
  appverlabel.setSize(1, 1);
  appverlabel.setColor(RC_GREY);
  LRemote.addControl(appverlabel);

  // Start broadcasting our remote contoller
  LRemote.begin();
  Serial.println("LRemote begin ...");
}

/*------------------------------------------------------------*/
void loop() {
  // BLE central device, e.g. an mobile app
  if (!LRemote.connected())
  {
    // Serial.println("Connected");
    delay(15);
  }
  else
  {
    // Serial.println("Wait for connection");
    delay(15);
  }

  // Process the incoming BLE write request
  LRemote.process();

/*------------------------------------------------------------*/
  // Battery Voltage label (以三用電表量測超級電容和7697偵測到電壓計算倍率..ADC pin 最大2.5V 要注意)
  BatteryVol = (analogRead(Battery_sensor_pin)*0.000606566*2);

  if (BatteryCount >= 5)
  {
    BatteryCount = 0;
    // Serial.print("BatteryVol = ");
    // Serial.println(BatteryVol);

    if (BatteryVol < BatteryLowVol)
    {
      BatteryLow = 1;
      batterylabel.updateText("沒電啦~");
      digitalWrite (Nobattery_led, HIGH);
      delay(150);
    }
    else
    {
      BatteryLow = 0;
      batterylabel.updateText(String(BatteryVol, 1) + "V");
      digitalWrite (Nobattery_led, LOW);
    }
  }
  else
  {
   	BatteryCount++;
  }
  
/*------------------------------------------------------------*/
  // Move
  if (forwardbutton.getValue())
  {
    Move_Forward();
  }
  else if (backwardbutton.getValue())
  {
    Move_Backward();
  }
  else if (turnleftbutton.getValue())
  {
    Move_TurnLeft();
  }
  else if (turnrightbutton.getValue())
  {
    Move_TurnRight();
  }
  else
  {
    Motor_Break();
  }
 
 /*------------------------------------------------------------*/ 
   // Lights 
  if (LightsButton.isValueChanged())
  {
    if (LightsButton.getValue() == 1)
      {
    digitalWrite(Front_led, HIGH);
    digitalWrite(Rear_led, HIGH);
    delay(100);
      }
  else
     {
    digitalWrite(Front_led, LOW);
    digitalWrite(Rear_led, LOW);
    }
  }

/*------------------------------------------------------------*/
  // Bucket UP
  if (bucketup.getValue())
   {
     BucketServoCen = (BucketServoCen - 2);
     Move_Bucket();
     delay(10); 
    }
 
   // Bucket DOWN
   if (bucketdown.getValue())
   {
    BucketServoCen = (BucketServoCen + 2);
    Move_Bucket();
    delay(10); 
 }

/*------------------------------------------------------------*/
     // Buzzer_pin
  if ( BuzzerButton.isValueChanged())
  {
    if ( BuzzerButton.getValue() == 1)
      {
    Buzzer_Sound();
    }
  }
  
  // Left slider tunning
  if (leftslider.isValueChanged())
  {
    Running_Servo_Trim[0] = leftslider.getValue();
    EEPROM.write(0, Running_Servo_Trim[0]);
    LeftServoCen = 90 + Running_Servo_Trim[0];
    LeftServo.write(LeftServoCen);
  }

  // Right slider tunning
  if (rightslider.isValueChanged())
  {
    Running_Servo_Trim[1] = rightslider.getValue();
    EEPROM.write(1, Running_Servo_Trim[1]);
    RightServoCen = 90 + Running_Servo_Trim[1];
    RightServo.write(RightServoCen);
  }

  // Bucket slider tunning
  if (bucketslider.isValueChanged())
  {
    Running_Servo_Trim[2] = bucketslider.getValue();
    EEPROM.write(2, Running_Servo_Trim[2]);
    BucketServoCen = 86 + Running_Servo_Trim[2];
    BucketServoCenmax = 86 + Running_Servo_Trim[2];
    BucketServo.write(BucketServoCenmax);
  }
  
  // Turbo switch
  if (turboswitchButton.isValueChanged())
  {
    Turbosw = turboswitchButton.getValue();
  }

  // Test Button (P6)
  if (digitalRead(User_sw) == HIGH)
  {
    // Serial.println("Test User_sw include Buzzer & Lights");
    Buzzer_Sound();
    digitalWrite(Front_led, HIGH);
    delay(500);
    digitalWrite(Front_led, LOW);
    delay(500);
    Buzzer_Sound();
    digitalWrite(Rear_led, HIGH);
    delay(500);
    digitalWrite(Rear_led, LOW);
    delay(500);
    Buzzer_Sound();
    digitalWrite(Nobattery_led, HIGH);
    delay(500);
    digitalWrite(Nobattery_led, LOW);
    delay(500);
    }
}
