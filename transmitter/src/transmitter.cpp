#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CHAN_NUM 0x71
#define THRUST_IN 2
#define ROLL_IN 3
#define PITCH_IN 4
#define YAW_IN 5

RF24 radio(9, 10); // "создать" модуль на пинах 9 и 10 Для Уно
byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; // возможные номера труб

byte transmit_data[4]; // массив, хранящий передаваемые данные
byte latest_data[4]; // массив, хранящий последние переданные данные
boolean flag; // флажок отправки данных

void setup()
{
  Serial.begin(9600); //открываем порт для связи с ПК

  radio.begin(); // активировать модуль
  radio.setAutoAck(1); // режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 15); // (время между попыткой достучаться, число попыток)
  radio.enableAckPayload(); // разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(32); // размер пакета, в байтах

  radio.openWritingPipe(address[0]); // мы - труба 0, открываем канал для передачи данных
  radio.setChannel(CHAN_NUM); // выбираем канал (в котором нет шумов!)

  radio.setPALevel(RF24_PA_MAX); // уровень мощности передатчика. На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
  radio.setDataRate(RF24_250KBPS); // скорость обмена. На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
  // должна быть одинакова на приёмнике и передатчике!
  // при самой низкой скорости имеем самую высокую чувствительность и дальность!!

  radio.powerUp(); // начать работу
  radio.stopListening(); // не слушаем радиоэфир, мы передатчик
}

void loop()
{
  int thrust_val = map(analogRead(THRUST_IN), 0, 1023, 0, 255);
  transmit_data[0] = thrust_val;

  int roll_val = analogRead(ROLL_IN);
  roll_val = map(roll_val, 0, 1023, 40, 140);
  transmit_data[1] = roll_val;

  int pitch_val = analogRead(PITCH_IN);
  pitch_val = map(pitch_val, 0, 1023, 145, 35);
  transmit_data[2] = pitch_val;

  int yaw_val = analogRead(YAW_IN);
  yaw_val = map(yaw_val, 0, 1023, 150, 30);
  transmit_data[3] = yaw_val;

  for (int i = 0; i <= 3; i++)
  { // в цикле от 0 до числа каналов
    if (transmit_data[i] != latest_data[i])
    { // если есть изменения в transmit_data
      flag = true; // поднять флаг отправки по радио
      latest_data[i] = transmit_data[i]; // запомнить последнее изменение
    }
  }

  if (flag)
  {
    radio.powerUp(); // включить передатчик
    radio.write(&transmit_data, sizeof(transmit_data)); // отправить по радио
    flag = false; // опустить флаг
    radio.powerDown(); // выключить передатчик
  }
}