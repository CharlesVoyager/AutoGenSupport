// This is a Slicer2.0DLL library demo application on Mac OS.
//
#include "stdafx.h"
#include <iostream>
#include "Slicer2.0DLL.h"

using namespace std;

int main()
{    
	cout << "--------------------------------------------" << endl;
    cout << "-  Starting Slicer2.0DLL Demo program      -" << endl;
    cout << "--------------------------------------------" << endl;

    wstring stlFile = L"../Cube.stl";
    
    wprintf(L"STL File: %ls\n", stlFile.c_str());
    
    int ret = fnSlicer20DLL(stlFile.c_str(), 0.1);
    
    printf("fnSlicer20DLL return value: %d\n", ret);
    
    printf("DemoAP Ends!\n");
	return 0;
}
