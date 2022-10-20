/* Скетч-пример работы с модулем
 * в режиме AT команд
 */

// #define ECHO_ON // раскомментируйте для вывода всех AT запросов и ответов в монитор последовательного порта
// Подключаем бибилиотеки
#include <GprsModem.h>
#include <SoftwareSerial.h>

// Если модуль подлючён к Piranha ULTRA или Arduino MEGA
// Раскомментируйте эту следующую строку и удалите указанные
// ниже строки.
//#define mySerial Serial1

// Определяем выводы RX, TX
#define RX 8
#define TX 9
// Определяем вывод PWR
#define PWR 7

// Создаём объекты библиотек
SoftwareSerial mySerial(RX, TX); // Эту сроку необходимо удалить для работы с аппаратным последовательным портом
GprsModem myModem(mySerial, PWR);
GprsClient myClient(mySerial);

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
	Serial.println("Готово, используйте поле ввода для команд");
}

void loop()
{
	// Используем объект клиента для работы в режиме AT команд
	// (Инициировать объект клиента функцией begin() при этом не обязательно)
	if (Serial.available() > 0)
		myClient.write(Serial.read());

	if (myClient.available() > 0)
		Serial.write(myClient.read());
}
