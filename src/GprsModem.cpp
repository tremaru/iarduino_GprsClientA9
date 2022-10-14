// tabstop=8
#define ECHO_ON
#include "GprsModem.h"

// AT prefix to start IP
const char* START = "AT+CIPSTART=";
// default HardwareSerial speed
const unsigned long H_SPEED = 115200;
// default SoftwareSerial speed
const unsigned long S_SPEED = 9600;
// AT prefix for changing baud rate
const char* GPRS_IPR = "ATZ+IPR=";

const char* GPRS_AT = "AT";
const char* GPRS_OK = "OK";

// Get the signal level and
const char* GPRS_SIGNAL = "AT+CSQ";
// echo off
const char* GPRS_ECHO_OFF = "ATE1";
// its response
const char* GPRS_SIGNAL_RESP = "+CSQ:";
// Close the connection
const char* GPRS_CLOSE = "AT+CIPCLOSE";
// IP mode?
const char* GPRS_CIPTMODE = "AT+CIPTMODE=1";
// Successful connection response
const char* CONNECT_STATUS = "CONNECT OK";
// Ready after reboot
const char* GPRS_READY = "READY";
// Default interval for waitResp() in ms
const unsigned long GPRS_WAIT = 8000;
// Default delay for coldReboot() in ms
const unsigned long REBOOT_DLY = 10000;
// Default delay for begin() in ms
const unsigned long INIT_DLY = 100;
//const unsigned long CH_RATE_DLY = 10;
// Default tries for _checkRate()
const uint8_t NUM_TRIES = 2;

/****************************** UTILITY FUNCS ******************************/

// Function for waiting a response from the module
static bool waitResp(unsigned long time, const char* req, String& buf, Stream& stream)
{
	unsigned long timer = millis();

	buf = "";
	String aresp = req;

	while (millis() - timer < time) {
		if (stream.available()) {
			timer = millis();
			int i = stream.read();
			char c = (uint8_t)i;
			//char c = stream.read();
			Serial.write(i);
			buf += String(c);
		}
	}

	aresp+="\0";

	echo(F("WAITING FOR: "));
	echo(aresp);
	echo("\n");
	echo(F("ECHO: "));
	echo(buf.c_str());

	if (aresp.length()) {
		return (buf.indexOf(aresp) > -1) ? true : false;
	}
	else
		return true;
}

// Same as before, overloaded
static bool waitResp(unsigned long time, const char* req, Stream& stream)
{
	delay(10);
	unsigned long timer = millis();

	String aresp = req;
	echo("WAITING FOR: ");
	echo(aresp);
	echo("\n");

	String buf;

	while (millis() - timer < time) {
		if (stream.available()) {
			timer = millis();
			//delay(3);
			int c = stream.read();
			if (c == -1)
				echo("***********WTF?**********");
			if (c == 0)
				echo("****************RECIEVED ZERO****************");
			buf += (char)c; //stream.read();
		}
		else {
			if (buf.indexOf(aresp) > -1) {
				echo(buf);
				return true;
			}
			delay(10);
		}
	}

	echo("Failed, buf: ");
	echo(buf);

	return false;
}

/****************************** MODEM STARTS HERE ******************************/

// Modem module init.
bool GprsModem::begin()
{
	// Checking rate
	int32_t rate = _checkRate();

	// if checking rate was unsuccessful
	if (rate == -1)
		// No point to do anything else
		return false;
	else {
		// Changing BAUD if necessary for HardwareSerial
		if (_serial && (rate != H_SPEED)) {
			_serial->print((String)GPRS_IPR + H_SPEED);
			_serial->print('\r');
			waitResp(GPRS_WAIT, GPRS_OK, *_serial);
			_serial->end();
			delay(INIT_DLY);
			_serial->begin(H_SPEED);
		}
		// Changing BAUD if necessary for SoftwareSerial
		else if (_s_serial && (rate != S_SPEED)) {
			_s_serial->print((String)GPRS_IPR + S_SPEED);
			_s_serial->print('\r');
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

int32_t GprsModem::_checkRate()
{
	// reboot module
	coldReboot();

	if (_serial) {
		_serial->end();
		_serial->begin(115200);
		waitResp(GPRS_WAIT*2, GPRS_READY, *_serial);
		_serial->print(GPRS_ECHO_OFF);
		_serial->print("\r");
		waitResp(GPRS_WAIT, GPRS_OK, *_serial);
	}
	else {
		_s_serial->end();
		_s_serial->begin(115200);
		waitResp(GPRS_WAIT*2, GPRS_READY, *_s_serial);
		_s_serial->print(GPRS_ECHO_OFF);
		_s_serial->print("\r");
		waitResp(GPRS_WAIT, GPRS_OK, *_s_serial);
	}

	static const uint32_t rates[] = {
		115200, 9600, 57600, 38400, 19200, 74400, 74880,
		230400, 460800, 2400, 4800, 14400, 28800
	};

	size_t rate_size = sizeof(rates) / sizeof(rates[0]);

	for (size_t i = 0; i < rate_size; i++) {
		uint32_t rate = rates[i];
		// if HardwareSerial
		if (_serial) {
			_serial->end();
			_serial->begin(rate);
			while(!_serial);
		}
		// if SoftwareSerial
		else {
			_s_serial->end();
			_s_serial->begin(rate);
			while(!_s_serial);
		}

		//delay(CH_RATE_DLY);

		for (uint8_t j = 0; j < NUM_TRIES; j++) {

			if (_serial) {
				_serial->print(GPRS_AT);
				_serial->print('\r');
				//delay(CH_RATE_DLY);
				if (waitResp(GPRS_WAIT, GPRS_OK, *_serial)) {
					return rate;
				}
			}
			else {
				_s_serial->print(GPRS_AT);
				_s_serial->print('\r');
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
void GprsModem::coldReboot()
{
	echo(F("Reboot\n"));
	pinMode(_pinPWR, OUTPUT);
	digitalWrite(_pinPWR, HIGH);
	delay(REBOOT_DLY);
	digitalWrite(_pinPWR, LOW);
	delay(REBOOT_DLY);
}

uint8_t GprsModem::getSignalLevel()
{
	String resp = "";
	if (_serial) {
		_serial->print(GPRS_SIGNAL);
		_serial->print('\r');
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
		_s_serial->print(GPRS_SIGNAL);
		_s_serial->print('\r');
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
	_serial.setTimeout(STREAM_TIMEOUT);

	_serial.print(F("AT+CGATT=1"));
	_serial.print('\r');

	if (!waitResp(GPRS_WAIT, "+CGATT:1", _serial)) {
		return false;
	}

	// Used to connect to Megafon. Works without it. Hadn't been checked
	// with other operators.
	_serial.print("AT+CGDCONT=1,\"IP\",\"internet\"");
	_serial.print('\r');

	if (!waitResp(2000L, "OK", _serial))
		return false;

	_serial.print(F("AT+CGACT=1,1"));
	_serial.print('\r');

	if (!waitResp(GPRS_WAIT, GPRS_OK, _serial))
		return false;

	// use this to enable sockets. Whole code should be rewritten
	// if those are enabled.
	/*
	_serial.print("AT+CIPMUX=1");
	_serial.print('\r');
	if (!waitResp(2000L, "OK", _serial))
		return false;
		*/
	return true;
}


// Connect to host name with default protocol (TCP)
int GprsClient::connect(const char* host, uint16_t port)
{
	String reqstr = (String)START
		+ "\"" + _protocol + "\""
		+ ",\"" + host + "\"," + port;

	echo("SENT TO SHIELD: ");
	echo(reqstr);
	echo("\r\n");

	_serial.print(reqstr);
	_serial.print('\r');

	//while(!_serial.available());

	if (!waitResp(GPRS_WAIT, CONNECT_STATUS, _serial))
		return 0;

	//reqstr = "AT+CIPTMODE=1";
	reqstr = GPRS_CIPTMODE;
	_serial.print(reqstr);
	_serial.print('\r');

	//while(!_serial.available());


	if (waitResp(GPRS_WAIT, GPRS_OK, _serial))
		return 1;

	return 0;
}

// Connect to host name with protocol
int GprsClient::connect(const char* host, uint16_t port, const char* protocol)
{
	_protocol = protocol;

	String reqstr = (String)START + "\""+ _protocol + "\""+ ",\"" + host + "\"," + port;

	_serial.print(reqstr);
	_serial.print('\r');

	//while(!_serial.available()); // is it needed?


	if (!waitResp(GPRS_WAIT, CONNECT_STATUS, _serial))
		return 0;

	//reqstr = "AT+CIPTMODE=1";
	reqstr = String(GPRS_CIPTMODE);
	_serial.print(reqstr);
	_serial.print('\r');

	//while(!_serial.available()); // is it needed?


	if (waitResp(GPRS_WAIT, GPRS_OK, _serial))
		return 1;
	return 0;
}

// Connect by IP address
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
	_serial.print(reqstr);
	_serial.print('\r');

	while(!_serial.available());

	if (!waitResp(GPRS_WAIT, CONNECT_STATUS, _serial))
		return 0;

	//reqstr = "AT+CIPTMODE=1";
	reqstr = String(GPRS_CIPTMODE);
	_serial.print(reqstr);
	_serial.print('\r');

	while(!_serial.available());


	if (waitResp(GPRS_WAIT, GPRS_OK, _serial))
		return 1;
	return 0;
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

int GprsClient::read(uint8_t* buf, size_t size)
{
	for (size_t i = 0; i < size; i++) {
		buf[i] = (uint8_t)_serial.read();
	}

	return (int)size;
}

void GprsClient::stop()
{
	_serial.print(GPRS_CLOSE);
	_serial.print('\r');
	while (!_serial.available());
	waitResp(GPRS_WAIT, GPRS_OK, _serial);
}

/**/

