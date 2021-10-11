#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>
#include<Wire.h>
#include <TimerOne.h>
#include <SoftwareSerial.h> //BT Library

const int SERVO_PIN = 6;
Servo myservo;


SoftwareSerial mySerial(8, 7); 
char myChar = 0;
String myString="";
////////////////////////////////
#define SS_PIN 10
#define RST_PIN 9
MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
byte nuidPICC[4];
boolean state = false;

#define    MPU9250_ADDRESS                0x68
#define    MPU9250_RA_WHO_AM_I            0x75   // 0x71 return
#define    MPU9250_RA_INT_PIN_CFG         0x37
#define    MPU9250_RA_CONFIG              0x1A
#define    MPU9250_RA_GYRO_CONFIG         0x1B
#define    MPU9250_RA_ACCEL_CONFIG        0x1C
#define    MPU9250_RA_FF_THR              0x1D
#define    MPU9250_RA_ACCEL_XOUT_H        0x3B

#define    AK8963_ADDRESS                 0x0C
#define    AK8963_RA_WHO_AM_I             0x00   // 0x48 return
#define    AK8963_RA_ST1                  0x02
#define    AK8963_RA_HXL                  0x03
#define    AK8963_RA_CNTL1                0x0A
#define    AK8963_RA_ST2                  0x0B
#define    AK8963_RA_ASAX                 0x10

#define    ACC_FULL_SCALE_2_G             0x00
#define    ACC_FULL_SCALE_4_G             0x08
#define    ACC_FULL_SCALE_8_G             0x10
#define    ACC_FULL_SCALE_16_G            0x18

#define    GYRO_FULL_SCALE_250_DPS        0x00
#define    GYRO_FULL_SCALE_500_DPS        0x08
#define    GYRO_FULL_SCALE_1000_DPS       0x10
#define    GYRO_FULL_SCALE_2000_DPS       0x18

#define    MAX_COUNT                      (0x0f + 1)
///////////////////////////////////////////////////////////


typedef struct MAG {
  int maximum;
  int average;
  int minimum;
} mag_t;

typedef struct MEASUREMENT {
  int x;
  int y;
  int z;
} measurement_t;

uint8_t ASA[3];
mag_t mag[3];
measurement_t m_data[MAX_COUNT];
int m_count = 0;
long count = 0;

// This function read Nbytes bytes from I2C device at address Address.
// Put read bytes starting at register Register in the Data array.
void I2Cread(uint8_t Address, uint8_t Register, uint8_t Nbytes, uint8_t* Data)
{
  // Set register address
  Wire.beginTransmission(Address);
  Wire.write(Register);
  Wire.endTransmission();

  // Read Nbytes
  Wire.requestFrom(Address, Nbytes);
  uint8_t index = 0;
  while (Wire.available())
    Data[index++] = Wire.read();
}

// Write a byte (Data) in device (Address) at register (Register)
void I2CwriteByte(uint8_t Address, uint8_t Register, uint8_t Data)
{
  // Set register address
  Wire.beginTransmission(Address);
  Wire.write(Register);
  Wire.write(Data);
  Wire.endTransmission();
}

volatile bool intFlag = false;

int getHeading(int x, int y) {
  float heading;

// 대한민국 서울:  -8º4' W = -8.067º W = -0.1408 radian
  heading = 180 * (atan2(y, x) - 0.1408) / PI;
  
  if (heading < 0) heading += 360;
  return (int)heading;
}


float NewCount[2] = {0};  // 스위치 입력이 들어갔을 때, 당시의 x값을 받아주는 친구. 
int CheckGetCount = 0;
int UpCount = 0;
int OutCounter = 0; // 역 구분용 변수
int InCounter = 0;  // 출구 입력후에 count를 통해 다음 RFID위치 알려주는 친구(출구 구분하는 변수필요)
int arrive = 1;

////////////////////////////////////////////////////////////////////////////////////////

  
void setup() {
  Serial.begin(9600);
  mySerial.begin(9600); //Bluetooth COMM speed 
  Wire.begin();
  SPI.begin();
  rfid.PCD_Init();
  ///////////////////////////////////////rfid부분
  for(byte i = 0; i < 6; i++){
    key.keyByte[i] = 0xFF;
    }
    
  myservo.attach(SERVO_PIN);
  myservo.write(0);
  delay(1000);
  ///////////////////////////////////////서보모터부분
  
  
  ///////////////////////////////////////////////////// 가속도센서부분
  mag[0].maximum = mag[1].maximum = mag[2].maximum = -32768;
  mag[0].minimum = mag[1].minimum = mag[2].minimum = 32767;

    for (int i; i < MAX_COUNT; i++) {
    m_data[i].x = m_data[i].y = m_data[i].z = 0;
  }

 
  uint8_t mode;
  I2Cread(MPU9250_ADDRESS, MPU9250_RA_INT_PIN_CFG, 1, &mode);
  mode |= 0b00000010;
  I2CwriteByte(MPU9250_ADDRESS, MPU9250_RA_INT_PIN_CFG, mode);

   unsigned char who;

  //Serial.print("MPU9250 WHO_AM_I: ");
  I2Cread(MPU9250_ADDRESS, MPU9250_RA_WHO_AM_I, 1, &who);
  //Serial.println(who, HEX);
  //Serial.print("AKA8963 WHO_AM_I: ");
  I2Cread(AK8963_ADDRESS, AK8963_RA_WHO_AM_I, 1, &who);
  //Serial.println(who, HEX);

  // set Fuse ROM access mode
  I2CwriteByte(AK8963_ADDRESS, AK8963_RA_CNTL1, 0x0F);
  delay(10);
  // read Sensitivity adjust registers
  I2Cread(AK8963_ADDRESS, AK8963_RA_ASAX, 3, (uint8_t *)&ASA);
  // set Power down mode
  I2CwriteByte(AK8963_ADDRESS, AK8963_RA_CNTL1, 0x00);
  delay(10);
  // Request continuous magnetometer measurements mode 2 in 16 bits
  //  7    6    5    4     3     2     1     0
  //  0    0    0   BIT  MODE3 MODE2 MODE1 MODE0
  //
  I2CwriteByte(AK8963_ADDRESS, AK8963_RA_CNTL1, 0x16);
  delay(10);
  
  // print out sensitivity adjust data
  //Serial.print("ASA X:");
  //Serial.print(ASA[0], DEC);
  //Serial.print("\t");
  //Serial.print("Y:");
  //Serial.print(ASA[1], DEC);
  //Serial.print("\t");
  //Serial.print("Z:");
  //Serial.println(ASA[2], DEC);

  //delay(3000);

  Timer1.initialize(10000);     // magnetometer measures at the 100Hz cycles
  Timer1.attachInterrupt(tick); // interrupt at 100Hz
}

void tick() {
  intFlag = true;
  digitalWrite(13, digitalRead(13) ^ 1);
}

void loop() {
  
  long duration, distance;
  
  //////////////////////////////////////////////////
  if (mySerial.available()) {
    Serial.write(mySerial.read());
  }
  if (Serial.available()) {
    mySerial.write(Serial.read());
  }
  digitalWrite(2,LOW);
  //////////////////////////////////////
  while (!intFlag);
  intFlag = false;

  // Read register Status 1 and wait for the DRDY: Data Ready
  uint8_t ST1;
  do {
    I2Cread(AK8963_ADDRESS, AK8963_RA_ST1, 1, &ST1);
  } while (!(ST1 & 0x01));

    do{
    // Read magnetometer data
    uint8_t Mag[7];
    I2Cread(AK8963_ADDRESS, AK8963_RA_HXL, 7, Mag);
    // Create 16 bits values from 8 bits data

    // if ST1 overrun or ST2 overflow, then exit
    if (ST1 & 0x02 || Mag[6] & 0x08)
      continue;

    // Display count
    //Serial.print (++count, DEC);
    //Serial.print ("\t");

    // Magnetometer
    int16_t mx = -((int16_t)(Mag[1] << 8 | Mag[0]));
    int16_t my = -((int16_t)(Mag[3] << 8 | Mag[2]));
    int16_t mz = ((int16_t)(Mag[5] << 8 | Mag[4]));

    // adjust the sensivitiy with the adjustment value
    float x, y, z;
    x = mx * ((ASA[0] - 128) * .5 / 128. + 1);
    y = my * ((ASA[1] - 128) * .5 / 128. + 1);
    z = mz * ((ASA[2] - 128) * .5 / 128. + 1);

    // adjust the measured magnetometer data
    // set max, min, average of x, y, z
    mag[0].maximum = max(mag[0].maximum, (int16_t)x);
    mag[0].minimum = min(mag[0].minimum, (int16_t)x);
    mag[0].average = (mag[0].maximum + mag[0].minimum) / 2;

    mag[1].maximum = max(mag[1].maximum, (int16_t)y);
    mag[1].minimum = min(mag[1].minimum, (int16_t)y);
    mag[1].average = (mag[1].maximum + mag[1].minimum) / 2;

    mag[2].maximum = max(mag[2].maximum, (int16_t)z);
    mag[2].minimum = min(mag[2].minimum, (int16_t)z);
    mag[2].average = (mag[2].maximum + mag[2].minimum) / 2;

    // store the measured magnetometer data into buffer
    m_data[m_count].x = (int16_t)x - mag[0].average;
    m_data[m_count].y = (int16_t)y - mag[1].average;
    m_data[m_count].z = (int16_t)z - mag[2].average;
 
    m_count = (m_count + 1) & (MAX_COUNT - 1);

    // smoothing the measured magnetometer data
    x = y = z = 0;
    for (int i = 0; i < MAX_COUNT; i++) {
      x += m_data[i].x;
      y += m_data[i].y;
      z += m_data[i].z;
    };

    x /= MAX_COUNT;
    y /= MAX_COUNT;
    z /= MAX_COUNT;
   delay(100);
    int heading = getHeading(x, y); 
    Serial.print("Heading:");
    Serial.println(heading, DEC);
    //mySerial.println(heading,DEC);
    //mySerial.print("Heading:");
    //Serial.println(heading);


    
     if(((heading >= 337) && (heading <= 360))||((heading >= 0) && (heading <22))){
          mySerial.println('a');
          Serial.println('a');
      }
     else if((heading >= 22) && (heading < 67)){
          mySerial.println('b');
          Serial.println('b');
      }
     else if((heading >= 67) && (heading < 112)){
          mySerial.println('c');
          Serial.println('c');
      }
     else if((heading >= 112) && (heading < 157)){
          mySerial.println('d');
          Serial.println('d');
      }
     else if((heading >= 157) && (heading < 202)){
          mySerial.println('e');
          Serial.println('e');
      }
     else if((heading >= 202) && (heading < 247)){
          mySerial.println('f');
          Serial.println('f');
      }
     else if((heading >= 247) && (heading < 292)){
          mySerial.println('g');
          Serial.println('g');
      }
     else if((heading >= 292) && (heading < 337)){
          mySerial.println('h');
          Serial.println('h');
      }
      
  /////////////////////////////RFID////////////////////////////
   if(! rfid.PICC_IsNewCardPresent())
  return;
  
 //카드 읽힘이 제대로 되면 다음으로 넘어감
  if(! rfid.PICC_ReadCardSerial())
  return;
  
  delay(1000/4*1.30);
  
  Serial.print(F("PICC type : ")); // 이전과 다른 카드이면 새로운 카드다 알림
  if(rfid.uid.uidByte[0] != nuidPICC[0] ||
      rfid.uid.uidByte[1] != nuidPICC[1] ||
      rfid.uid.uidByte[2] != nuidPICC[2] ||
      rfid.uid.uidByte[3] != nuidPICC[3]){
     Serial.println(F("NEW CARD"));
      }

  for(byte i = 0; i < 4; i++){   //고유 아이디 값 저장
    nuidPICC[i] = rfid.uid.uidByte[i];
   }

   printDec(rfid.uid.uidByte,rfid.uid.size);
   Serial.println();
   
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

 if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

    else Serial.println(F("Card read previously"));
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    
  ///////////////////////////역 구분 ///////////////////////////////////////////////////////
  //정왕역 RFID
    if(rfid.uid.uidByte[0] == 186 &&
         rfid.uid.uidByte[1] == 23 &&
          rfid.uid.uidByte[2] == 169 &&
           rfid.uid.uidByte[3] == 21){
      
      mySerial.print('J');  
      Serial.print('J');   
      }
  //A-1
       if(rfid.uid.uidByte[0] == 145 && 
           rfid.uid.uidByte[1] == 91 &&
            rfid.uid.uidByte[2] == 90 &&
             rfid.uid.uidByte[3] == 26){//A번 출구 루트의 첫번째 rfid
                mySerial.print('A');
                Serial.print('A');
            }
        
  //A-2
         if(rfid.uid.uidByte[0] == 5 && //A번 출구 루트의 두번째 rfid
             rfid.uid.uidByte[1] == 49 &&
              rfid.uid.uidByte[2] == 255 &&
               rfid.uid.uidByte[3] == 40){
                mySerial.print('B');
                Serial.print('B');
            }
         
  //A-3 
         if(rfid.uid.uidByte[0] == 244 && //A번 출구 루트의 세번째 rfid
             rfid.uid.uidByte[1] == 195 &&
              rfid.uid.uidByte[2] == 241 &&
               rfid.uid.uidByte[3] == 40){
                mySerial.print('C');
                Serial.print('C');
            }
  //B-1 
         if(rfid.uid.uidByte[0] == 218 && //A번 출구 루트의 세번째 rfid
             rfid.uid.uidByte[1] == 57 &&
              rfid.uid.uidByte[2] == 57 &&
               rfid.uid.uidByte[3] == 179){
                mySerial.print('D');
                Serial.print('D');
            }
  //B-2 
         if(rfid.uid.uidByte[0] == 250 && //A번 출구 루트의 세번째 rfid
             rfid.uid.uidByte[1] == 8 &&
              rfid.uid.uidByte[2] == 209 &&
               rfid.uid.uidByte[3] == 128){
                mySerial.print('E');
                Serial.print('E');
            }
  //B-3
         if(rfid.uid.uidByte[0] == 122 && //A번 출구 루트의 세번째 rfid
             rfid.uid.uidByte[1] == 247 &&
              rfid.uid.uidByte[2] == 222 &&
               rfid.uid.uidByte[3] == 128){
                mySerial.print('F');
                Serial.print('F');
            }

  //////////////////////////////////////////////////////////////////////////////// 
      
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();   
     }while(0); 
   }
   
 
void printDec(byte *buffer, byte bufferSize){
  for(byte i = 0;i< bufferSize; i++){
      Serial.print(buffer[i] < 0x10 ? " 0" : " ");
      Serial.print(buffer[i], DEC);
    }
  }