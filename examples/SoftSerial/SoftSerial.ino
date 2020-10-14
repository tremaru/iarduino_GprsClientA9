#include <GprsModem.h>
#include <SoftwareSerial.h>

#define RX 7
#define TX 8

constexpr char host[] = "www.google.com";
constexpr char req[] = "GET /search?q=iarduino HTTP/1.1";
constexpr int port = 80;

SoftwareSerial mySerial(RX, TX);
GprsModem myModem(mySerial);
GprsClient myClient(mySerial);

void setup()
{
	Serial.begin(115200);

	while(!Serial);

	if (!myModem.begin()) {
		Serial.println("GSM Shield не найден.");
		while(1);
	}

	myClient.begin();

	if (!myClient.connect(host, port)) {
		Serial.println("Не удалось подключиться к удалённому узлу");
		while(1);
	}

	myClient.println((String)req);
	myClient.println((String)"Host: " + host);
	myClient.println(F("Connection: close"));
	myClient.println();
}

void loop()
{
	if (myClient.available() > 0) {
		Serial.write(myClient.read());
	}
}
