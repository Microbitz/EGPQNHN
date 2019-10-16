
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>
#include <ModbusRtu.h>

//------------LCD----------------------
#define I2C_ADDR    0x3F
LiquidCrystal_I2C   lcd(I2C_ADDR, 2, 1, 0, 4, 5, 6, 7);
//------------
SoftwareSerial mySerial(11, 12); // RX, TX
//------------RTC----------------------
#include "RTClib.h"
RTC_DS1307 rtc;
int segundo, minuto, hora, dia, mes;
long anio; //variable año
DateTime HoraFecha;
//------------18b20-------------------
const int pinDatosDQ = 4;
OneWire oneWireObjeto(pinDatosDQ);
DallasTemperature sensorDS18B20(&oneWireObjeto);
//------------Modbus------------------
#define TXEN  10
uint16_t au16data[16] = {
  3, 1415, 9265, 4, 2, 7182, 28182, 8, 0, 0, 0, 0, 0, 0, 1, -1
};
Modbus slave(1, 0, TXEN);

void setup() {
  pinMode(13, OUTPUT);
  Wire.begin();
  lcd.begin (16, 2);   // Inicializar el display con 16 caraceres 2 lineas
  lcd.setBacklightPin(3, POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home ();
  //---------RTC------------------
  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  //---------18b20------------------
  sensorDS18B20.begin();

  //------------Modbus------------------
  slave.begin( 9600 );
}

void loop() {
  //----------LCD------------
  lcd.setCursor(0, 0);
  //----------RTC------------
  HoraFecha = rtc.now();
  segundo = HoraFecha.second();
  minuto = HoraFecha.minute();
  hora = HoraFecha.hour();
  dia = HoraFecha.day();
  mes = HoraFecha.month();
  anio = HoraFecha.year();
  lcd.print(hora);
  lcd.print(":");
  lcd.print(minuto);
  //----------LCD------------
  lcd.setCursor(0, 1);
  //----------18b20------------
  sensorDS18B20.requestTemperatures();
  lcd.print("T:");
  lcd.print(sensorDS18B20.getTempCByIndex(0));
  //------------Modbus------------------
  digitalWrite(LED_BUILTIN, TXEN);
  union {
    float fval;
    byte bval[2];
  } floatAsBytes;

  floatAsBytes.fval = sensorDS18B20.getTempCByIndex(0);
  int TV1 = floatAsBytes.bval[0] * 256 + floatAsBytes.bval[1];
  int TV2 = floatAsBytes.bval[2] * 256 + floatAsBytes.bval[3];
  uint16_t au16data[16] = {
    hora , minuto , dia, mes, anio, TV1, TV2, 8, 0, 0, 0, 0, 0, 0, 1, -1
  };
  slave.poll( au16data, 16 );

  //-------------------------------------
}
