// Скетч-пример работы с модулем при помощи аппаратного последовательного порта

// Подключаем библиотеку модуля A9 для работы с сетью Интернет
#include <GprsModem.h>

// Если модуль подключён к Piranha UNO
#define mySerial Serial

// Если модуль подлючён к Piranha ULTRA
// Раскомментируйте эту строку и закомментируйте или удалите
// предыдущую. Переключите Shield на работу с выводами 8 и 9.
//#define mySerial Serial1

// Определяем вывод PWR
#define PWR 7

// Данные для получения информации от удалённого узла
char host[] = "www.google.com";
char req[] = "GET /search?q=iarduino HTTP/1.1";
int port = 80;

// Создаём объекты библиотек
GprsModem myModem(mySerial, PWR);
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

	// Инициируем объект клиента
	myClient.begin();

	// Подключаемся к серверу
	Serial.println("Ждём подключения к удалённому узлу");
	if (!myClient.connect(host, port)) {
		Serial.println("Не удалось подключится к удалённому узлу");
		while(1);
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
