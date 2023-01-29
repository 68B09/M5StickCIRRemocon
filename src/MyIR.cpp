#include "MyIR.h"
#include <M5StickC.h>

#define IR_LED_PIN 9 // M5Stick-C IRLedピン番号
#define IR_LED_ON LOW // IR-Pin is ActiveLow
#define IR_LED_OFF HIGH

// 37.9kHz=26.3852nsec,半分=26.3852/2=13.1926
#define IR_SHORT_DURATION_MICRO_SEC 12
#define IR_1T 23 // 1T	※26μs/回 * 23回 = 598μs

MyIR::MyIR() {}

void MyIR::Begin() {
	pinMode(IR_LED_PIN, OUTPUT);
	digitalWrite(IR_LED_PIN, IR_LED_OFF);
}

void MyIR::SendSpace(const int pMicroSec) { delayMicroseconds(pMicroSec); }

void MyIR::SendPulse(const int pMicroSec) {
	int count = pMicroSec / (IR_1T - 8);
	for (int i = 0; i < count; i++) {
		digitalWrite(IR_LED_PIN, IR_LED_ON);
		delayMicroseconds(IR_SHORT_DURATION_MICRO_SEC);
		digitalWrite(IR_LED_PIN, IR_LED_OFF);
		delayMicroseconds(IR_SHORT_DURATION_MICRO_SEC);
	}
}

void MyIR::SendData(const int *pData, const int pNum) {
	for (int i = 0; i < pNum; i++) {
		if ((i & 1) == 0) {
			SendPulse(pData[i]);
		} else {
			SendSpace(pData[i]);
		}
	}
}
