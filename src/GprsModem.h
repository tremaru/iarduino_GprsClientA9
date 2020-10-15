// tabstop=8
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

// A9 modem is ridiculously slow.
constexpr unsigned long GPRS_TIMEOUT = 4000;
//constexpr unsigned long SER_TIMEOUT = 4000;
constexpr uint8_t DEFAULT_PIN_PWR = 9;

// modem class, for initialization.
class GprsModem {
	public:
		GprsModem(const HardwareSerial& serial):
			_serial(&serial),
			_s_serial(nullptr) {}
		GprsModem(const SoftwareSerial& serial):
			_serial(nullptr),
			_s_serial(&serial) {}
		bool begin();
		void coldReboot(uint8_t pinPWR = DEFAULT_PIN_PWR);
		uint8_t getSignalLevel();
	private:
		uint32_t _checkRate();

		// had to create two fields and this whole class because
		// begin() funcs are absent in the Stream class.
		const HardwareSerial* _serial;
		const SoftwareSerial* _s_serial;
};

// client class.
class GprsClient: public Client {
	public:
		GprsClient(const HardwareSerial& serial): _serial(serial) {
			_timeout = GPRS_TIMEOUT;
		}

		GprsClient(const SoftwareSerial& serial): _serial(serial) {
			_timeout = GPRS_TIMEOUT;
		}

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
};

#endif
