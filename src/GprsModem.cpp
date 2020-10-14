#include "GprsModem.h"

//#define H_CAST (*(HardwareSerial*)_serial)
//#define S_CAST (*(SoftwareSerial*)_serial)

constexpr unsigned long H_SPEED = 115200;
constexpr unsigned long S_SPEED = 9600;

/*
bool GprsModem::begin()
{
	return _begin();
}
*/

bool GprsModem::begin()
{

	uint32_t rate = _checkRate();


	if (rate == -1)
		return false;
	else {
		if (_serial && (rate != H_SPEED)) {
			_serial->println((String)"ATZ+IPR=" + H_SPEED);
			GprsClient::waitResp(2000UL, "OK", *_serial);
			_serial->end();
			delay(100);
			_serial->begin(H_SPEED);
		}
		else if (_s_serial && (rate != S_SPEED)) {
			_s_serial->println((String)"ATZ+IPR=" + S_SPEED);
			GprsClient::waitResp(2000UL, "OK", *_s_serial);
			_s_serial->end();
			delay(100);
			_s_serial->begin(S_SPEED);
		}
	}
	return true;
}

// Cold reboot thru power pin
void GprsModem::coldReboot(uint8_t pinPWR)
{
	pinMode(pinPWR, OUTPUT);
	digitalWrite(pinPWR, HIGH);
	delay(2000);
	digitalWrite(pinPWR, LOW);
	delay(2000);
}

/*
 * Checking for current baud rate. Based on TinyGSM code
 * by Volodymyr Shymanskyy. Thank you, dude.
 */

uint32_t GprsModem::_checkRate()
{
	static uint32_t rates[] = {
		115200, 9600, 57600, 38400, 19200, 74400, 74880,
		230400, 460800, 2400, 4800, 14400, 28800
	};

	for (uint8_t i = 0; i < sizeof(rates) / sizeof(rates[0]); i++) {
		uint32_t rate = rates[i];
		if (_serial)
			_serial->begin(rate);
		else
			_s_serial->begin(rate);

		delay(10);

		for (uint8_t j = 0; j < 5; j++) {

			if (_serial) {
				_serial->println("AT");
				delay(10);
				if (GprsClient::waitResp(2000UL, "OK", *_serial))
					return rate;
			}
			else {
				_s_serial->println("AT");
				delay(10);
				if (GprsClient::waitResp(2000UL, "OK", *_s_serial)) {
					return rate;
				}
			}
		}
	}
	return -1;
}

bool GprsClient::begin()
{
	_serial.println("AT+CGATT=1");

	if (!waitResp(2000L, "+CGATT:1", _serial)) {
		return false;
	}

	_serial.println("AT+CGDCONT=1,\"IP\",\"internet\"");

	if (!waitResp(2000L, "OK", _serial))
		return false;

	_serial.println("AT+CGACT=1,1");

	if (!waitResp(2000L, "OK", _serial))
		return false;

	// use this to enable sockets. Whole code should be rewritten
	// if those are enabled.
	/*
	_serial.println("AT+CIPMUX=1");
	if (!waitResp(2000L, "OK", _serial))
		return false;
		*/
	return true;
}

#define CONNECT_STATUS "CONNECT OK"

int GprsClient::connect(const char* host, uint16_t port)
{
	String reqstr = (String)START
		+ "\"" + _protocol + "\""
		+ ",\"" + host + "\"," + port;

	_serial.println(reqstr);

	while(!_serial.available());

	if (!waitResp(2000ul, String(CONNECT_STATUS), _serial))
		return 0;

	reqstr = "AT+CIPTMODE=1";
	_serial.println(reqstr);

	while(!_serial.available());


	if (waitResp(2000ul, "OK", _serial))
		return 1;

	return 0;
}

int GprsClient::connect(const char* host, uint16_t port, const char* protocol)
{
	_protocol = protocol;

	String reqstr = (String)START + "\""+ _protocol + "\""+ ",\"" + host + "\"," + port;

	_serial.println(reqstr);

	while(!_serial.available()); // is it needed?


	if (!waitResp(2000ul, String(CONNECT_STATUS), _serial))
		return 0;

	reqstr = "AT+CIPTMODE=1";
	_serial.println(reqstr);

	while(!_serial.available()); // is it needed?


	if (waitResp(2000ul, "OK", _serial))
		return 1;
	return 0;
}

int GprsClient::connect(IPAddress ip, uint16_t port)
{
// following ifdefs are copied from TinyGSM by Volodymyr Shymanskyy. Thank you dude.
#if defined(ESP8266) || defined(ESP32)
	if (ip == IPAddress((uint32_t)0) || ip == IPAddress(0xFFFFFFFFul)) return 0;
#else
	if (ip == IPAddress(0ul) || ip == IPAddress(0xFFFFFFFFul)) return 0;
#endif

	String addr = "";
	for (size_t i = 0; i < 4; i++) {
		if (i)
			addr += '.';
		addr += ip[i];
	}

	String reqstr = (String)START + "\""+ _protocol + "\""+ ",\"" + addr + "\"," + port;
	_serial.println(reqstr);

	while(!_serial.available());

	if (!waitResp(2000ul, String(CONNECT_STATUS), _serial))
		return 0;

	reqstr = "AT+CIPTMODE=1";
	_serial.println(reqstr);

	while(!_serial.available());


	if (waitResp(2000ul, "OK", _serial))
		return 1;
	return 0;
}


size_t GprsClient::write(uint8_t B)
{
	return _serial.write(B);
}

size_t GprsClient::write(const uint8_t* buf, size_t size)
{
	return _serial.write(buf, size);
}

// this func had to be implemented with wait.
int GprsClient::available()
{
	unsigned long timer = micros();

	while(micros() - timer < 1000UL) {
		if (_serial.available())
			return _serial.available();
	}

	return -1;
}

int GprsClient::read()
{
	return _serial.read();
}

int GprsClient::read(uint8_t* buf, size_t size)
{
	for (size_t i = 0; i < size; i++) {
		buf[i] = (uint8_t)_serial.read();
	}

	return (int)size;
}

void GprsClient::stop()
{
	_serial.println("AT+CIPCLOSE");
	while (!_serial.available());
	waitResp(2000ul, "OK", _serial);
}
