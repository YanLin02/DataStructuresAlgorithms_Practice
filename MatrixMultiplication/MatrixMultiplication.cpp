// 
//

#include <iostream>
#include "Buffer.h"
#include "Definition.h"
#include "Matrix.h"

using namespace std;

void test() {
	cout << "BUFF_SIZE = " << BUFFER_SIZE << endl << endl;
	Matrix A(10, 10, "A.bin", "random");
	Matrix B(10, 10, "B.bin", "random");
	Matrix IKJ = A.ikj_multiple(B);

	cout << "==========================   IKJ   ==========================" << endl;
	cout << "     ======================   A   ======================" << endl;
	A.printStats();
	cout << "     ======================   B   ======================" << endl;
	B.printStats();
	cout << "     =====================   IKJ   =====================" << endl;
	IKJ.printStats();
	cout << endl;

	Matrix C(10, 10, "C.bin", "random");
	Matrix D(10, 10, "D.bin", "random");
	Matrix IJK = C.ijk_multiple(D);
	cout << "==========================   IJK   ==========================" << endl;
	cout << "     ======================   C   ======================" << endl;
	C.printStats();
	cout << "     ======================   D   ======================" << endl;
	D.printStats();
	cout << "     =====================   IJK   =====================" << endl;
	IJK.printStats();

	//cout << "====================   PRINT   ====================" << endl;
	//A.print();
	//cout << endl;
	//B.print();
	//cout << endl;
	//IKJ.print();
}

int main()
{
	srand(time(NULL));//随机数种子

	test();


	return EXIT_SUCCESS;
}
