#ifndef _MyIR_
#define _MyIR_

class MyIR {
  private:
	const int measureloopcount = 1000;
	int measureloopmicrosec;

  public:
	MyIR();
	void Begin();
	void SendData(const int *pData, const int pNum);
	void SendSpace(const int pMicroSec);
	void SendPulse(const int pMicroSec);
};
#endif