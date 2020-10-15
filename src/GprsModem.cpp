// tabstop=8
#include "GprsModem.h"

//#define H_CAST (*(HardwareSerial*)_serial)
//#define S_CAST (*(SoftwareSerial*)_serial)

// AT prefix to start ip
constexpr char* START = "AT+CIPSTART=";

// Pass through mode. Pretty hard to implement Client class
// with this. You expected to pass 0x1A at the end, which
// is hard to predict when implementing print() funcs through
// write() funcs.
//constexpr char* PASS_THRU_MODE = "AT+CIPTMODE=1";

constexpr unsigned long H_SPEED = 115200;
constexpr unsigned long S_SPEED = 9600;
constexpr char* GPRS_IPR = "ATZ+IPR=";
constexpr char* GPRS_AT = "AT";
constexpr char* GPRS_OK = "OK";
constexpr char* GPRS_SIGNAL = "AT+CSQ";
constexpr char* GPRS_SIGNAL_RESP = "+CSQ:";
constexpr unsigned long GPRS_WAIT = 2000;
constexpr unsigned long REBOOT_DLY = 2000;
constexpr unsigned long INIT_DLY = 100;
constexpr unsigned long CH_RATE_DLY = 10;
constexpr uint8_t NUM_TRIES = 2;

/****************************** UTILITY FUNCS ******************************/

// Function for waiting a response from the module
static bool waitResp(unsigned long time, const String& aresp, String& buf, const Stream& stream)
{
	unsigned long timer = millis();

	buf = "";

	while (millis() - timer < time) {
		if (stream.available()) {
			timer = millis();
			char c = stream.read();
			buf += String(c);
		}
	}

	if (aresp.length()) {
		return (buf.indexOf(aresp) > -1) ? true : false;
	}
	else
		return true;
}

// Same as before, overloaded
static bool waitResp(unsigned long time, const String& aresp, const Stream& stream)
{
	unsigned long timer = millis();

	String buf = "";

	while (millis() - timer < time) {
		if (stream.available()) {
			timer = millis();
			char c = stream.read();
			buf += String(c);
		}
	}

	if (aresp.length()) {
		return (buf.indexOf(aresp) > -1) ? true : false;
	}
	else
		return true;
}

/****************************** MODEM STARTS HERE ******************************/

// Modem module init.
bool GprsModem::begin()
{

	// Checking rate
	uint32_t rate = _checkRate();

	// if checking rate was unsuccessful
	if (rate == -1)
		// No point to do anything else
		return false;
	else {
		// Changing BAUD if necessary for HardwareSerial
		if (_serial && (rate != H_SPEED)) {
			_serial->println((String)GPRS_IPR + H_SPEED);
			waitResp(GPRS_WAIT, GPRS_OK, *_serial);
			_serial->end();
			delay(INIT_DLY);
			_serial->begin(H_SPEED);
		}
		// Changing BAUD if necessary for SoftwareSerial
		else if (_s_serial && (rate != S_SPEED)) {
			_s_serial->println((String)GPRS_IPR + S_SPEED);
			waitResp(GPRS_WAIT, GPRS_OK, *_s_serial);
			_s_serial->end();
			delay(INIT_DLY);
			_s_serial->begin(S_SPEED);
		}
	}
	return true;
}

/*
 * Checking for current baud rate. Based on TinyGSM code
 * by Volodymyr Shymanskyy. Thank you, dude.
 */

uint32_t GprsModem::_checkRate()
{
	// reboot module
	coldReboot();
	static const uint32_t rates[] = {
		115200, 9600, 57600, 38400, 19200, 74400, 74880,
		230400, 460800, 2400, 4800, 14400, 28800
	};

	size_t rate_size = sizeof(rates) / sizeof(rates[0]);

	for (size_t i = 0; i < rate_size; i++) {
		uint32_t rate = rates[i];
		// if HardwareSerial
		if (_serial)
			_serial->begin(rate);
		// if SoftwareSerial
		else
			_s_serial->begin(rate);

		//delay(CH_RATE_DLY);

		for (uint8_t j = 0; j < NUM_TRIES; j++) {

			if (_serial) {
				_serial->println(GPRS_AT);
				//delay(CH_RATE_DLY);
				if (waitResp(GPRS_WAIT, GPRS_OK, *_serial)) {
					//Serial.println(rate);
					return rate;
				}
			}
			else {
				_s_serial->println(GPRS_AT);
				//delay(CH_RATE_DLY);
				if (waitResp(GPRS_WAIT, GPRS_OK, *_s_serial)) {
					return rate;
				}
			}
		}
	}
	return -1;
}

// Cold reboot thru power pin
void GprsModem::coldReboot(uint8_t pinPWR)
{
	pinMode(pinPWR, OUTPUT);
	digitalWrite(pinPWR, HIGH);
	delay(REBOOT_DLY);
	digitalWrite(pinPWR, LOW);
	delay(REBOOT_DLY);
}


uint8_t GprsModem::getSignalLevel()
{
	String resp = "";
	if (_serial) {
		_serial->println(GPRS_SIGNAL);
		//delay(10);
		if (waitResp(
					GPRS_WAIT,
					GPRS_SIGNAL_RESP,
					resp,
					*_serial
					)
				) {
			int index = resp.indexOf(':');
			index++;
			resp = resp.substring(index, index + 3);
			resp.trim();
			return (uint8_t) resp.toInt();
		}

	}
	else {
		_s_serial->println(GPRS_SIGNAL);
		//delay(10);
		if (waitResp(
					GPRS_WAIT,
					GPRS_SIGNAL_RESP,
					resp,
					*_s_serial
					)
				) {
			int index = resp.indexOf(':');
			index++;
			resp = resp.substring(index, index + 3);
			resp.trim();
			return (uint8_t) resp.toInt();
		}
	}
	return 0;
}

/****************************** MODEM ENDS HERE ******************************/

// Enter IP mode and configure internet
bool GprsClient::begin()
{
	_serial.println("AT+CGATT=1");

	if (!waitResp(2000L, "+CGATT:1", _serial)) {
		return false;
	}

	/*
	_serial.println("AT+CGDCONT=1,\"IP\",\"internet\"");

	if (!waitResp(2000L, "OK", _serial))
		return false;
		*/

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

/**/

