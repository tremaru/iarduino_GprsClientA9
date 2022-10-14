// tabstop=8
#ifndef __GPRSMODEM_H__
#define __GPRSMODEM_H__
#include <Arduino.h>

#ifdef ECHO_ON
#define echo(A) Serial.print(A);
#else
#define echo(FMT, ...) do {} while(0)
#endif

//TODO: add sockets. AT+CIPMUX=1. Theoretically this should improve speed.

#include <Arduino.h>
#include <Stream.h>
#include <Client.h>
#include <SoftwareSerial.h>

#include "IPAddress.h"

// TODO: force client to work on ip addresses, not domain names. This
// should improve speed even more.
//#include "Dns.h"

// A9 modem is ridiculously slow. This sets Stream class timeout before aborting
// the read of next char (Client:Stream _timeout protected field)
constexpr unsigned long STREAM_TIMEOUT = 4000;

// modem class, for hardware initialization.
class GprsModem {
	public:
		GprsModem(HardwareSerial& serial, int pinPWR):
			_pinPWR(pinPWR),
			_serial(&serial),
			_s_serial(nullptr) {}
		GprsModem(SoftwareSerial& serial, int pinPWR):
			_pinPWR(pinPWR),
			_serial(nullptr),
			_s_serial(&serial) {}
		bool begin();
		void coldReboot();
		uint8_t getSignalLevel();
	private:
		int32_t _checkRate();

		// had to create two fields and this whole class because
		// begin() funcs are absent in the Stream class.
		int _pinPWR;
		HardwareSerial* _serial;
		SoftwareSerial* _s_serial;

};

// client class. Should work with everithing that Arduino client does
class GprsClient: public Client {
	public:
		GprsClient(HardwareSerial& serial): _serial(serial) {
			//_timeout = STREAM_TIMEOUT;
		}

		GprsClient(SoftwareSerial& serial): _serial(serial) {
			//_timeout = STREAM_TIMEOUT;
		}

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

#endif // __GPRSMODEM_H__
