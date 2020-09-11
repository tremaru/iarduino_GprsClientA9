#include "GprsClient.h"


//GprsClient::_serial = &HardwareSerial;

bool GprsClient::begin()
{
	//_serial.begin(115200);

	_serial.setTimeout(SER_TIMEOUT);
	//delay(500);

	_serial.println("AT+CGATT=1");
	if (!waitResp(2000L, "+CGATT:1", _serial))
		return false;
	_serial.println("AT+CGDCONT=1,\"IP\",\"internet\"");
	if (!waitResp(2000L, "OK", _serial))
		return false;
	_serial.println("AT+CGACT=1,1");
	if (!waitResp(2000L, "OK", _serial))
		return false;
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
	//Serial.println(_serial.getTimeout());
	String reqstr = (String)START + "\"" + _protocol + "\""+ ",\"" + host + "\"," + port;
	//String reqstr = (String)START + ",\"" + host + "\"," + port;
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
	//Serial.println(_serial.getTimeout());

	_protocol = protocol;
	String reqstr = (String)START + "\""+ _protocol + "\""+ ",\"" + host + "\"," + port;
	//String reqstr = (String)START + ",\"" + host + "\"," + port;
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

int GprsClient::connect(IPAddress ip, uint16_t port)
{
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

int GprsClient::available()
{
	return _serial.available();
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
