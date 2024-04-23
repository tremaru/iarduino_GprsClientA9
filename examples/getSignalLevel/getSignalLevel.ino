// Подключаем бибилиотеки

// #define ECHO_ON // раскомментируйте для вывода всех AT запросов и ответов в монитор последовательного порта
// Подключаем бибилиотеки
#include <SoftwareSerial.h> // Если используется библиотека <SoftwareSerial.h>, то она должна быть подключена до <GprsModem.h>
#include <GprsModem.h>

//#define mySerial Serial1 // Раскоменитеруйте строку для работы с аппаратным портом
#define RX 8
#define TX 9
// Определяем вывод PWR
#define PWR 7

// Создаём объекты библиотек
SoftwareSerial mySerial(RX, TX); // Удалите строку для использования с аппаратным портом
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

	// Получаем уровень сигнала
	uint8_t level = myModem.getSignalLevel();
	// Выводим уровень сигнала в последовательный порт
	Serial.print("Уровень сигнала: ");
	Serial.println(level);
}

void loop()
{
}
