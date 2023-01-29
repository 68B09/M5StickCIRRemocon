#include "MyIR.h"
#include <M5StickC.h>

#define PIN_IR_RECV 26
#define readIR() digitalRead(PIN_IR_RECV)
#define isIROn() (readIR() == 0)

#define cls()                                                                  \
	{                                                                          \
		M5.Lcd.fillScreen(BLACK);                                              \
		M5.Lcd.setCursor(0, 0);                                                \
	}

enum {
	MODE_TOP_INITIAL,
	MODE_TOP_MAIN,
	MODE_RECV_INITIAL,
	MODE_RECV_MAIN,
	MODE_SEND_INITIAL,
	MODE_SEND_MAIN,
	MODE_DUMP_INITIAL,
	MODE_DUMP_MAIN,
};
int mode = MODE_TOP_INITIAL;

char strbuf[32];

const int irBufMaxNum = 1000;
int irBuf[irBufMaxNum];
int irBufDataNum = 0;

void setup() {
	M5.begin();
	M5.Lcd.setRotation(3);
	M5.Lcd.setTextSize(2);
	pinMode(PIN_IR_RECV, INPUT_PULLUP);
	Serial.begin(115200);
}

void RecvProc() {
	M5.Lcd.print("wait ");

	unsigned long blankmsec, blankmsecBefore, usec, usecBefore;
	int isIROn, isIROnBefore;

	irBufDataNum = 0;

	// 赤外線信号が受信出来ない状態になるまで待機
	int count = 0;
	while (true) {
		isIROn = isIROn();
		if (isIROn) {
			count = 0;
		} else {
			count++;
			if (count >= 100) {
				break;
			}
		}
		delay(10);
	}

	// 赤外線信号が受信開始
	isIROnBefore = isIROn;
	usecBefore = 0;
	blankmsecBefore = millis();

	M5.Lcd.println("please");

	while (true) {
		isIROn = isIROn();
		if (isIROn != isIROnBefore) {
			// 赤外線信号のOn/Off状態が変化した

			isIROnBefore = isIROn;
			usec = micros();
			if (usecBefore != 0) {
				irBuf[irBufDataNum++] = usec - usecBefore;
				if (irBufDataNum >= irBufMaxNum) {
					break;
				}
			}
			usecBefore = usec;
			blankmsecBefore = millis();
		} else {
			// 一定時間変化が無い場合は終了
			blankmsec = millis();
			if ((blankmsec - blankmsecBefore) >= 2000) {
				break;
			}
		}
	}
}

void loop() {
	M5.update();

	if (mode == MODE_TOP_INITIAL) {
		cls();
		M5.Lcd.println("(M5)IR recv");
		M5.Lcd.println("(R)IR send");
		M5.Lcd.println("(L)data send");
		mode = MODE_TOP_MAIN;
	} else if (mode == MODE_TOP_MAIN) {
		if (M5.BtnA.wasPressed()) {
			mode = MODE_RECV_INITIAL;
		} else if (M5.BtnB.wasPressed()) {
			mode = MODE_SEND_INITIAL;
		} else if (M5.Axp.GetBtnPress() == 2) {
			mode = MODE_DUMP_INITIAL;
		}
	} else if (mode == MODE_RECV_INITIAL) {
		cls();
		M5.Lcd.println("IR recv ready");
		mode = MODE_RECV_MAIN;
	} else if (mode == MODE_RECV_MAIN) {
		RecvProc();
		snprintf(strbuf, sizeof(strbuf), "%d datas recv", irBufDataNum);
		M5.Lcd.println(strbuf);
		delay(2000);
		mode = MODE_TOP_INITIAL;
	} else if (mode == MODE_SEND_INITIAL) {
		cls();
		if (irBufDataNum > 0) {
			mode = MODE_SEND_MAIN;
		} else {
			M5.Lcd.println("data empty");
			delay(1000);
			mode = MODE_TOP_INITIAL;
		}
	} else if (mode == MODE_SEND_MAIN) {
		M5.Lcd.println("IR send");

		MyIR ir;
		ir.Begin();
		ir.SendData(irBuf, irBufDataNum);

		mode = MODE_TOP_INITIAL;
	} else if (mode == MODE_DUMP_INITIAL) {
		cls();
		if (irBufDataNum > 0) {
			mode = MODE_DUMP_MAIN;
		} else {
			M5.Lcd.println("data empty");
			delay(1000);
			mode = MODE_TOP_INITIAL;
		}
	} else if (mode == MODE_DUMP_MAIN) {
		M5.Lcd.println("data send");

		Serial.println("// subcarrier on tick,subcarrier off tick,...");
		Serial.println("// tick is micro second.");
		snprintf(strbuf, sizeof(strbuf), "datas %d", irBufDataNum);
		Serial.println(strbuf);

		for (int i = 0; i < irBufDataNum; i++) {
			if ((i % 100) == 0) {
				M5.Lcd.print(".");
			}

			if (i == 0) {
				snprintf(strbuf, sizeof(strbuf), "%d", irBuf[i]);
			} else {
				snprintf(strbuf, sizeof(strbuf), ",%d", irBuf[i]);
			}
			Serial.print(strbuf);
		}
		Serial.println("");
		Serial.println("// end of data");

		M5.Lcd.println("complete.");
		delay(1000);
		mode = MODE_TOP_INITIAL;
	}
}
