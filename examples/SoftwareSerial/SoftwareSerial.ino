// Скетч-пример работы с модулем при помощи программного последовательного порта

// Подключаем бибилиотеки
#include <GprsModem.h>
#include <SoftwareSerial.h>

// Определяем выводы RX, TX
#define RX 8
#define TX 9
// Определяем вывод PWR
#define PWR 7

// Данные для получения информации от удалённого узла
char host[] = "www.google.com";
char req[] = "GET /search?q=iarduino HTTP/1.1";
int port = 80;

// Создаём объекты библиотек
SoftwareSerial mySerial(RX, TX);
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
	Serial.println("Готово!");

	// Инициируем объект клиента
	myClient.begin();

	// Подключаемся к серверу
	Serial.println("Ждём подключения к удалённому узлу");
	if (!myClient.connect(host, port)) {
		Serial.println("Не удалось подключиться к удалённому узлу");
		return;
	}

	// Делаем запрос на сервер.
	myClient.println((String)req);
	myClient.println((String)"Host: " + host);
	myClient.println(F("Connection: close"));
	myClient.println();
}

void loop()
{
	// Получаем данные с сервера
	if (myClient.available() > 0) {
		Serial.write(myClient.read());
	}
}
