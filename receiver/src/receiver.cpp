#include <Arduino.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include <Servo.h>

#define CHAN_NUM 0x71
#define THRUST_OUT 6
#define ROLL_OUT_LEFT 2
#define ROLL_OUT_RIGHT 3
#define PITCH_OUT 4
#define YAW_OUT 5

RF24 radio(9, 10);                                                          // "создать" модуль на пинах 9 и 10 Для Уно
byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; //возможные номера труб

Servo engine;
Servo roll_left_servo;
Servo roll_right_servo;
Servo pitch_servo;
Servo yaw_servo;

byte recieved_data[4];

void setup()
{
  Serial.begin(9600); //открываем порт для связи с ПК

  engine.attach(THRUST_OUT);
  roll_left_servo.attach(ROLL_OUT_LEFT);
  roll_right_servo.attach(ROLL_OUT_RIGHT);
  pitch_servo.attach(PITCH_OUT);
  yaw_servo.attach(YAW_OUT);

  engine.writeMicroseconds(2300);
  delay(2000);
  engine.writeMicroseconds(800);
  delay(6000);

  radio.begin();            // активировать модуль
  radio.setAutoAck(1);      // режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 15);  // (время между попыткой достучаться, число попыток)
  radio.enableAckPayload(); // разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(32); // размер пакета, в байтах

  radio.openReadingPipe(1, address[0]); // хотим слушать трубу 0
  radio.setChannel(CHAN_NUM);           // выбираем канал (в котором нет шумов!)

  radio.setPALevel(RF24_PA_MAX);   // уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate(RF24_250KBPS); // скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
  // должна быть одинакова на приёмнике и передатчике!
  // при самой низкой скорости имеем самую высокую чувствительность и дальность!!

  radio.powerUp();        // начать работу
  radio.startListening(); // начинаем слушать эфир, мы приёмный модуль
}

void loop()
{
  byte pipeNo;
  while (radio.available(&pipeNo))
  {                                                    // слушаем эфир со всех труб
    radio.read(&recieved_data, sizeof(recieved_data)); // чиатем входящий сигнал
    int thrust_val = map(recieved_data[0], 0, 255, 800, 2270);
    engine.writeMicroseconds(thrust_val);

    int roll_val = recieved_data[1];
    roll_left_servo.write(roll_val);
    roll_right_servo.write(roll_val);

    pitch_servo.write(recieved_data[2]);
    yaw_servo.write(recieved_data[3]);
  }
}
