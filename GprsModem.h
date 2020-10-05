#ifndef gprsModem_h
#define gprsModem_h

//TODO: add sockets. AT+CIPMUX=1. Theoretically this should improve speed.

#include <Arduino.h>
#include <Stream.h>
#include <Client.h>
#include <SoftwareSerial.h>

#include "IPAddress.h"
// TODO: force client to work on ip addresses, not domain names. This
// should improve speed even more.
//#include "Dns.h"

constexpr char* START = "AT+CIPSTART=";
// Pass through mode. Pretty hard to implement Client class
// with this. You expected to pass 0x1A at the end, which
// is hard to predict when implementing print() funcs through
// write() funcs.
//constexpr char* PASS_THRU_MODE = "AT+CIPTMODE=1";

// A9 modem is ridiculously slow.
#define GPRS_TIMEOUT 4000
#define SER_TIMEOUT 4000

// modem class, for initialization.
class GprsModem {
	public:
		GprsModem(const HardwareSerial& serial):
			_serial(serial),
			_native_serial(true) {;}
		GprsModem(const SoftwareSerial& serial):
			_s_serial(serial),
			_native_serial(false) {;}
		bool begin();
	private:
		uint32_t _checkRate(const bool&);
		bool _native_serial;
		bool _begin(const bool&);

		// had to create two fields and this whole class because
		// begin() funcs are absent in the Stream class.
		const HardwareSerial& _serial;
		const SoftwareSerial& _s_serial;
};

// client class.
class GprsClient: public Client {
	friend class GprsModem;
	public:
		GprsClient(const HardwareSerial& serial): _serial(serial) {_timeout = GPRS_TIMEOUT;};
		GprsClient(const SoftwareSerial& serial): _serial(serial) {_timeout = GPRS_TIMEOUT;};
		int connect(const char* host, uint16_t port);
		int connect(IPAddress ip, uint16_t port);
		int connect(const char* host, uint16_t port, const char* protocol);
		int available();
		size_t write(uint8_t);
		size_t write(const uint8_t *buf, size_t size);
		int read();
		void stop();
		bool begin();
		int read(uint8_t* buf, size_t size);

		// empty functions:
		int availableForWrite() { return 0;}
		void flush() {};
		int peek() { return 0; }
		uint8_t connected() { return 0; }
		operator bool() { return false; }

	private:
		String _protocol = "TCP";

		// this field becomes SoftwareSerial or HardwareSerial.
		const Stream& _serial;

		// function for waiting modem response. I had troubles getting
		// it to work as a global function (for some reason arduino
		// included everything two times, despite ifndef-guards. So here it
		// is: static and friended. Probably shouldn't friended the whole
		// class. But eh...
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

			if (aresp != nullptr)
				return buf.indexOf((String)aresp) > -1 ? true : false;
			else
				return true;
		}
};

#endif
