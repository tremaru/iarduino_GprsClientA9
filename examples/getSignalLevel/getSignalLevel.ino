#include <GprsModem.h>
#include <SoftwareSerial.h>

//#define mySerial Serial1
#define RX 7
#define TX 8

SoftwareSerial mySerial(RX, TX);
GprsModem myModem(mySerial);

void setup()
{
	Serial.begin(115200);

	while(!Serial);

	Serial.println("Ждём инициализации Shield'а");
	if (!myModem.begin()) {
		Serial.println("GSM Shield не найден.");
		while(1);
	}

	uint8_t level = myModem.getSignalLevel();
	Serial.print("Уровень сигнала: ");
	Serial.println(level);
}

void loop()
{
}
