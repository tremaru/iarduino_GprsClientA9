// Скетч-пример получения статуса Shield'а

// #define ECHO_ON // раскомментируйте для вывода всех AT запросов и ответов в монитор последовательного порта
// Подключаем бибилиотеки
#include <SoftwareSerial.h> // Если используется библиотека <SoftwareSerial.h>, то она должна быть подключена до <GprsModem.h>
#include <GprsModem.h>

// Определяем выводы RX, TX
#define RX 8
#define TX 9
// Определяем вывод PWR
#define PWR 7

// Создаём объекты библиотек
SoftwareSerial mySerial(RX, TX);
GprsModem myModem(mySerial, PWR);

void setup()
{
	Serial.begin(115200);

	while(!Serial);

	// Инициируем модем
	Serial.println("Ждём инициализации Shield'а..");
	while (myModem.status() != GPRS_OK) {
		Serial.print(".");
		myModem.begin();
	}
	Serial.println();
	Serial.println("Готово!");

	switch(myModem.updateStatus()){
		case GPRS_OK          : Serial.println("Модуль готов к работе."); break;
		case GPRS_REG_NO      : Serial.println("На данный момент модем не зарегистрирован в сети оператора связи."); break;
		case GPRS_SPEED_ERR   : Serial.println("Не удалось согласовать скорость UART."); break;
		case GPRS_UNKNOWN     : Serial.println("Статус неизвестен"); break;
		case GPRS_SLEEP       : Serial.println("Режим ограниченной функциональности."); break;
		case GPRS_SIM_NO      : Serial.println("Нет SIM-карты."); break;
		case GPRS_SIM_FAULT   : Serial.println("SIM-карта неисправна."); break;
		case GPRS_SIM_ERR     : Serial.println("SIM-карта не прошла проверку."); break;
		case GPRS_REG_FAULT   : Serial.println("Оператор отклонил регистрацию модема."); break;
		case GPRS_SIM_PIN     : Serial.println("Требуется ввод PIN-кода"); break;
		case GPRS_SIM_PUK     : Serial.println("Требуется ввод PUK1"); break;
		default               : break;
	}
}

void loop()
{
}
