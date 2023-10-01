#include <iostream>
#include "Buffer.h"
#include "Definition.h"
#include "Matrix.h"

using namespace std;

void test() {
	cout << "BUFF_SIZE = " << BUFFER_SIZE << endl;
	cout << "MATRIX_SIZE = " << MATRIX_SIZE << endl << endl;
	Matrix A(MATRIX_SIZE, MATRIX_SIZE, "A.bin", "random");
	Matrix B(MATRIX_SIZE, MATRIX_SIZE, "B.bin", "random");
	Matrix IKJ = A.ikj_multiple(B);

	cout << "==========================   IKJ   ==========================" << endl;
	cout << "     ======================   A   ======================" << endl;
	A.printStats();
	cout << "     ======================   B   ======================" << endl;
	B.printStats();
	cout << "     =====================   IKJ   =====================" << endl;
	IKJ.printStats();
	cout << "     ====================   TOTAL   ====================" << endl;
	cout << "visit: " << A.buf.visit + B.buf.visit + IKJ.buf.visit << endl;
	cout << "miss: " << A.buf.miss + B.buf.miss + IKJ.buf.miss << endl;
	cout << "hit rate: " << (double)(A.buf.visit + B.buf.visit + IKJ.buf.visit - A.buf.miss - B.buf.miss - IKJ.buf.miss) / (A.buf.visit + B.buf.visit + IKJ.buf.visit) << endl;
	cout << endl;

	Matrix C(MATRIX_SIZE, MATRIX_SIZE, "C.bin", "random");
	Matrix D(MATRIX_SIZE, MATRIX_SIZE, "D.bin", "random");
	Matrix IJK = C.ijk_multiple(D);
	cout << "==========================   IJK   ==========================" << endl;
	cout << "     ======================   C   ======================" << endl;
	C.printStats();
	cout << "     ======================   D   ======================" << endl;
	D.printStats();
	cout << "     =====================   IJK   =====================" << endl;
	IJK.printStats();
	cout << "     ====================   TOTAL   ====================" << endl;
	cout << "visit: " << C.buf.visit + D.buf.visit + IJK.buf.visit << endl;
	cout << "miss: " << C.buf.miss + D.buf.miss + IJK.buf.miss << endl;
	cout << "hit rate: " << (double)(C.buf.visit + D.buf.visit + IJK.buf.visit - C.buf.miss - D.buf.miss - IJK.buf.miss) / (C.buf.visit + D.buf.visit + IJK.buf.visit) << endl;

//	cout << endl;
//	cout << "==========================   ANS   ==========================" << endl;
//	A.print();
//	cout << endl;
//	B.print();
//	cout << endl;
//	IKJ.print();
}

int main()
{
	srand(time(NULL));//随机数种子
	test();
	return EXIT_SUCCESS;
}
