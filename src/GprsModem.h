#ifndef __GPRSMODEM_H__
#define __GPRSMODEM_H__
#include <Arduino.h>

#ifdef ECHO_ON
#define echo(A) Serial.print((String)"["+millis()+"] "+__func__+": ");\
       	Serial.println(A);
#else
#define echo(A) do {} while(0)
#endif

//TODO: add sockets. AT+CIPMUX=1. Theoretically this should improve speed.
#include <Arduino.h>
#include <Stream.h>
#include <Client.h>

#if defined(__SOFTWARE_SERIAL_H__) || defined(SoftwareSerial_h)
	#include <SoftwareSerial.h>
	#define incSWSerial
#endif

#include "IPAddress.h"

#define SERIAL_CLEAR 5000
#define SPEED_CHANGE_WAIT 500
#define VERY_LONG_WAIT 10000
#define CONNECT_TIMEOUT 8000

// TODO: force client to work on ip addresses, not domain names. This
// should improve speed even more.
//#include "Dns.h"

enum {
	GPRS_OK		,
	GPRS_SPEED_ERR	,
	GPRS_UNAVAILABLE,
	GPRS_UNKNOWN	,
	GPRS_SLEEP	,
	GPRS_SIM_PIN	,
	GPRS_SIM_PUK	,
	GPRS_SIM_PIN2	,
	GPRS_SIM_PUK2	,
	GPRS_SIM_NO	,
	GPRS_SIM_FAULT	,
	GPRS_SIM_ERR	,
	GPRS_REG_NO	,
	GPRS_REG_FAULT	,
	GPRS_REG_ERR	,
	GPRS_NO_INIT
};

// modem class, for hardware initialization.
class GprsModem {
	public:
		GprsModem(HardwareSerial& serial, int pinPWR):
			_pinPWR(pinPWR),
			_h_serial(&serial),
			_s_serial(nullptr),
       			_serial(serial)	{}
		#ifdef incSWSerial
		GprsModem(SoftwareSerial& serial, int pinPWR):
			_pinPWR(pinPWR),
			_h_serial(nullptr),
			_s_serial(&serial),
       			_serial(serial)	{}
		#endif
		bool begin();
		void coldReboot();
		uint8_t getSignalLevel();
		uint8_t updateStatus();
		uint8_t status()
		{
			return _status;
		}

	private:
		int32_t _checkRate();

		// had to create two fields and this whole class because
		// begin() funcs are absent in the Stream class.
		int _pinPWR;
		// for begin only
		HardwareSerial* _h_serial;
		#ifdef incSWSerial
		SoftwareSerial* _s_serial;
		#else
		HardwareSerial* _s_serial;
		#endif
		// more generic
		Stream& _serial;
		bool _speed = false;
		int _status = GPRS_NO_INIT;
};

// client class. Should work with everithing that Arduino client does
class GprsClient: public Client {
	public:
		GprsClient(HardwareSerial& serial): _serial(serial) {
		}

		#ifdef incSWSerial
		GprsClient(SoftwareSerial& serial): _serial(serial) {
		}
		#endif

		bool begin();
		int connect(const char* host, uint16_t port, const char* protocol);

		virtual int connect(const char* host, uint16_t port) override;
		virtual int connect(IPAddress ip, uint16_t port) override;
		virtual int available() override;
		virtual size_t write(uint8_t B) override
		{
			return _serial.write(B);
		}
		virtual size_t write(const uint8_t *buf, size_t size) override
		{
			return _serial.write(buf, size);
		}
		virtual int read() override
		{
			return _serial.read();
		}
		virtual void stop() override;
		virtual int read(uint8_t* buf, size_t size) override;

		// empty functions (cannot be implemented for this hardware):
		virtual int availableForWrite() override { return 0;}
		virtual void flush() override {};
		virtual int peek() override { return 0; }
		virtual uint8_t connected() override { return 0; }
		virtual operator bool() override { return false; }

	private:
		String _protocol = "TCP";

		// this field becomes SoftwareSerial or HardwareSerial.
		Stream& _serial;
};

// AT prefix to start IP
const char* START = "AT+CIPSTART=";
// default HardwareSerial speed
const unsigned long H_SPEED = 115200;
// default SoftwareSerial speed
const unsigned long S_SPEED = 9600;
// AT prefix for changing baud rate
const char* GPRS_IPR = "ATZ+IPR=";

const char* GPRS_AT = "AT";
const char* GPRS_STR_OK = "OK";

// MT attach command
const char* GPRS_ATTACH = "AT+CGATT=1";
// Default interval for waitResp() in ms
const unsigned long GPRS_WAIT = 200;
// Get the signal level and
const char* GPRS_SIGNAL = "AT+CSQ";
// its response
const char* GPRS_SIGNAL_RESP = "+CSQ:";
// Close the connection
const char* GPRS_CLOSE = "AT+CIPCLOSE";
// IP mode?
const char* GPRS_CIPTMODE = "AT+CIPTMODE=1";
// Successful connection response
const char* CONNECT_STATUS = "CONNECT";
// Ready after reboot
const char* GPRS_READY = "READY";
// Default delay for coldReboot() in ms
const unsigned long REBOOT_DLY = 2000;
// Default delay for begin() in ms
const unsigned long INIT_DLY = 100;
//const unsigned long CH_RATE_DLY = 10;
// Default tries for _checkRate()
const uint8_t NUM_TRIES = 10;

/****************************** UTILITY FUNCS ******************************/
// functions used by both classes
static void AT(
		String command,
		String& buf,
		Stream& stream,
		unsigned long timeout = GPRS_WAIT)
{
	buf = "";
	command += "\r";
	stream.print(command);
	echo("SENT TO SHEILD: ");
	echo(command);

	unsigned long timer = millis();
	while (millis() - timer < timeout) {
		if (stream.available()) {
			timer = millis();
			int c = stream.read();
			if (c > 0)
				buf += (char)c;
		}
		else {
			delay(10);
		}
	}
	echo("RESPONSE: ");
	echo(buf);
}

static bool AT(
		String command,
		const char* resp,
		String& buf,
		Stream& stream,
		unsigned long timeout = GPRS_WAIT)
{
	buf = "";
	command += "\r";
	stream.print(command);

	echo("SENT TO SHIELD: ");
	echo(command);
	echo("WAITING FOR: ");
	echo(resp);

	String aresp = resp;

	unsigned long timer = millis();
	while (millis() - timer < timeout) {
		if (stream.available()) {
			timer = millis();
			int c = stream.read();
			if (c > 0)
				buf += (char)c;
		}
		else {
			if (buf.indexOf(aresp) > -1) {
				echo("RESPONSE: ");
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

static bool AT(
		String& command,
		const char* resp,
		Stream& stream,
		unsigned long timeout = GPRS_WAIT)
{
	command += "\r";
	stream.print(command);

	echo("SENT TO SHIELD: ");
	echo(command);
	echo("WAITING FOR: ");
	echo(resp);

	String buf = "";
	String aresp = resp;

	unsigned long timer = millis();
	while (millis() - timer < timeout) {
		if (stream.available()) {
			timer = millis();
			int c = stream.read();
			if (c > 0)
				buf += (char)c;
		}
		else {
			if (buf.indexOf(aresp) > -1) {
				echo("RESPONSE: ");
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

static inline void drainBuffer(Stream& serial)
{
	unsigned long timestamp = millis();
	while (millis() - timestamp < SERIAL_CLEAR) {
		if (serial.available() > 0) {
			timestamp = millis();
			serial.read();
		}
	}
}

/****************************** MODEM STARTS HERE ******************************/

// Modem module init.
bool GprsModem::begin()
{
	// reboot module
	coldReboot();

	// init serial
	if (_h_serial) {
		_h_serial->end();
		delay(INIT_DLY);
		_h_serial->begin(H_SPEED);
		while(!(*_h_serial));
	}
	else if (_s_serial) {
		#ifndef RENESAS_CORTEX_M4 // Библиотека SoftwareSerial для плат Arduino UNO R4 не имеет функции end(). По состоянию на март 2024г.
		_s_serial->end();
		delay(INIT_DLY);
		#endif
		_s_serial->begin(H_SPEED);
	}

	delay(INIT_DLY);

	drainBuffer(_serial);

	// Changing module speed for SoftwareSerial
	if (_s_serial) {
		String req = (String) GPRS_IPR + S_SPEED;
		AT(req, GPRS_STR_OK, _serial, SPEED_CHANGE_WAIT);
		#ifndef RENESAS_CORTEX_M4 // Библиотека SoftwareSerial для плат Arduino UNO R4 не имеет функции end(). По состоянию на март 2024г.
		_s_serial->end();
		delay(INIT_DLY);
		#endif
		_s_serial->begin(S_SPEED);
		delay(INIT_DLY);
	}

	// Change to hi speed if for some reason module started in 9600
	if (_h_serial) {
		_h_serial->end();
		delay(INIT_DLY);
		_h_serial->begin(S_SPEED);
		while(!(*_h_serial));
		String req = (String) GPRS_IPR + H_SPEED;
		AT(req, GPRS_STR_OK, _serial, SPEED_CHANGE_WAIT);
		_h_serial->end();
		delay(INIT_DLY);
		_h_serial->begin(H_SPEED);
		delay(INIT_DLY);
	}

	// checking if module responds...
	for (int i = 0; i < NUM_TRIES; i++) {
		String command = GPRS_AT;
		if (AT(command, GPRS_STR_OK, _serial)) {
			_speed = true;
			echo("HOORAY!");
			break;
		}
	}

	drainBuffer(_serial);

	// getting status
	switch (_status = updateStatus()) {
		default: break;
		case GPRS_SIM_NO:
			return false;
		case GPRS_SIM_FAULT:
			return false;
		case GPRS_SIM_ERR:
			return false;
		case GPRS_REG_FAULT:
			return false;
		case GPRS_UNAVAILABLE:
			return false;
		case GPRS_UNKNOWN:
			return false;
		case GPRS_SLEEP:
			return false;
	}

	// sending configs
	String buf;
	AT(F("AT+CSCS=\"HEX\""), buf, _serial);
	AT(F("AT+CPMS=\"SM\",\"SM\",\"SM\""), buf, _serial);
	AT(F("ATE0"), buf, _serial);
	AT(F("ATV1"), buf, _serial);
	AT(F("AT+CMEE=1"), buf, _serial);
	AT(F("AT+CREG=0"), buf, _serial);
	AT(F("AT+CMGF=0"), buf, _serial);
	AT(F("AT+CNMI=1,0,0,0,0"), buf, _serial);
	AT(F("AT+CMGD=1,3"), buf, _serial, VERY_LONG_WAIT);
	AT(F("AT+CSCS=\"HEX\""), buf, _serial);
	AT(F("AT+CPMS=\"SM\",\"SM\",\"SM\""), buf, _serial);

	return true;
}

uint8_t GprsModem::updateStatus() {
        int i;
	String buf;
        if (!_speed) {
                return GPRS_SPEED_ERR;
        }
        AT(F("AT+CPAS"), buf, _serial);
        if (buf.indexOf(F("+CPAS:1")) > -1) {
                return GPRS_UNAVAILABLE;
        }
        if (buf.indexOf(F("+CPAS:2")) > -1) {
                return GPRS_UNKNOWN;
        }
        if (buf.indexOf(F("+CPAS:5")) > -1) {
                return GPRS_SLEEP;
        }
        AT(F("AT+CPIN?"), buf, _serial);
        if (buf.indexOf(F("+CPIN:SIM PIN")) > -1) {
                return GPRS_SIM_PIN;
        }
        if (buf.indexOf(F("+CPIN:SIM PUK")) > -1) {
                return GPRS_SIM_PUK;
        }
        if (buf.indexOf(F("+CPIN:SIM PIN2")) > -1) {
                return GPRS_SIM_PIN2;
        }
        if (buf.indexOf(F("+CPIN:SIM PUK2")) > -1) {
                return GPRS_SIM_PUK2;
        }
        if (buf.indexOf(F("+CME ERROR:10")) > -1) {
                return GPRS_SIM_NO;
        }
        if (buf.indexOf(F("+CME ERROR:13")) > -1) {
                return GPRS_SIM_FAULT;
        }
        if (buf.indexOf(F("+CPIN:READY")) < 0) {
                return GPRS_SIM_ERR;
        }
        AT(F("AT+CREG?"), buf, _serial);
        if (buf.indexOf(F("+CREG:")) < 0) {
                return GPRS_REG_ERR;
        }
        i = buf.indexOf(F("+CREG:"));
        i = buf.indexOf(F(","), i) + 1;
        if (buf.charAt(i + 1) != 13) {
                return GPRS_REG_NO;
        }
        if (buf.charAt(i) == '0') {
                return GPRS_REG_NO;
        }
        if (buf.charAt(i) == '2') {
                return GPRS_REG_NO;
        }
        if (buf.charAt(i) == '3') {
                return GPRS_REG_FAULT;
        }
        if ((buf.charAt(i) != '1') &&
                (buf.charAt(i) != '5')) {
                return GPRS_REG_NO;
        }
        return GPRS_OK;
}


// Cold reboot thru power pin
void GprsModem::coldReboot()
{
	echo(F("Reboot"));
	pinMode(_pinPWR, OUTPUT);
	digitalWrite(_pinPWR, HIGH);
	delay(REBOOT_DLY);
	digitalWrite(_pinPWR, LOW);
	delay(REBOOT_DLY);
}


uint8_t GprsModem::getSignalLevel()
{
	String buf = "";

	if (!AT(GPRS_SIGNAL, GPRS_SIGNAL_RESP, buf, _serial))
		return 0;

	int index = buf.indexOf(':');
	index++;
	buf = buf.substring(index, index+3);
	buf.trim();
	return (uint8_t) buf.toInt();
}

/****************************** MODEM ENDS HERE ******************************/

// Enter IP mode and configure internet
bool GprsClient::begin()
{
	//_serial.setTimeout(4000);

	/*
	_serial.print(F("AT+CGATT=1"));
	_serial.print('\r');
	*/

	String command = GPRS_ATTACH;

	if (!AT(command, "+CGATT:1", _serial)) {
		return false;
	}

	// Used to connect to Megafon. Works without it. Hadn't been checked
	// with other operators.
	/*
	_serial.print("AT+CGDCONT=1,\"IP\",\"internet\"");
	_serial.print('\r');

	if (!waitResp(2000L, "OK", _serial))
		return false;
		*/

	/*
	_serial.print(F("AT+CGACT=1,1"));
	_serial.print('\r');
	*/

	command = "ATT+CGACT=1,1";
	if (!AT(command, GPRS_STR_OK, _serial))
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

	if (!AT(reqstr, CONNECT_STATUS, _serial, CONNECT_TIMEOUT))
		return 0;

	reqstr = GPRS_CIPTMODE;

	if (AT(reqstr, GPRS_STR_OK, _serial))
		return 1;

	return 0;
}

// Connect to host name with protocol
int GprsClient::connect(const char* host, uint16_t port, const char* protocol)
{
	_protocol = protocol;

	String reqstr = (String)START + "\""+ _protocol + "\""+ ",\"" + host + "\"," + port;

	if (!AT(reqstr, CONNECT_STATUS, _serial), CONNECT_TIMEOUT)
		return 0;

	reqstr = GPRS_CIPTMODE;

	if (AT(reqstr, GPRS_STR_OK, _serial))
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

	if (!AT(reqstr, CONNECT_STATUS, _serial, CONNECT_TIMEOUT))
		return 0;

	reqstr = String(GPRS_CIPTMODE);

	if (AT(reqstr, GPRS_STR_OK, _serial))
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
	drainBuffer(_serial);
	//String req = GPRS_CLOSE;
	//AT(req, GPRS_STR_OK, _serial, VERY_LONG_WAIT);
}

#endif // __GPRSMODEM_H__
