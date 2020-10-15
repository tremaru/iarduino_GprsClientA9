/* Скетч-пример работы с модулем
 * в режиме AT команд
 */

// Подключаем бибилиотеки
#include <GprsModem.h>
#include <SoftwareSerial.h>

// Если модуль подлючён к Piranha ULTRA или Arduino MEGA
// Раскомментируйте эту следующую строку и удалите указанные
// ниже строки. Переключите Shield на работу с выводами 7 и 8.
//#define mySerial Serial1

// Определяем выводы RX, TX
#define RX 7
#define TX 8

// Создаём объекты библиотек
SoftwareSerial mySerial(RX, TX); // Эту сроку необходимо удалить для работы с аппаратным последовательным портом
GprsModem myModem(mySerial);
GprsClient myClient(mySerial);

void setup()
{
	Serial.begin(115200);

	while(!Serial);

	// Инициируем модем
	Serial.println("Ждём инициализации Shield'а");
	if (!myModem.begin()) {
		Serial.println("GSM Shield не найден.");
		while(1);
	}
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
