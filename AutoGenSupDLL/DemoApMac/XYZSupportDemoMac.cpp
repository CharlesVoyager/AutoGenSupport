// This is a XYZSupport library demo application on Mac OS.
//
#include "stdafx.h"
#include <iostream>
#include "Sp3ApiTest.h"
#include "MyTools.h"

using namespace std;

int main()
{    
	cout << "--------------------------------------------" << endl;
    cout << "-  Starting XYZSupport Module Demo program -" << endl;
    cout << "--------------------------------------------" << endl;

#if 0
    Sp3ApiTest test("Normal",       // edit mode. "Normal", "User", "Cone", "Tree"
                    "Normal",       // type code. "Mormal", "Mark"
                    "Cube.stl",  	// stl filename
                    "32 20 20",     // position (X Y Z). NOTE: Z is the lowest postion of the model
                    "1 1 1",        // scale (X Y Z)
                    "0 0 0",        // rotation (X Y Z)
                    "false",        // inner support
                    "PartPro150 xP",// printer name
                    "MED",          // density. "MED", "LOW"
                    "0.4",          // contactSize. 0.15mm ~ 0.65mm
                    "0.6"           // newBaseZScale. "0": old base. "0.6", "1.2": new base
                    );
#endif
    
#if 1
    Sp3ApiTest test("Normal",       // edit mode. "Normal", "User", "Cone", "Tree"
                    "Normal",       // type code. "Mormal", "Mark"
                    "10x10x1.stl",      // stl filename
                    "32 20 10",     // position (X Y Z). NOTE: Z is the lowest postion of the model
                    "1 1 1",        // scale (X Y Z)
                    "0 0 0",        // rotation (X Y Z)
                    "false",        // inner support
                    "PartPro150 xP",// printer name
                    "MED",          // density. "MED", "LOW"
                    "0.15",         // contactSize. 0.15mm ~ 0.65mm
                    "0.6"           // newBaseZScale. "0": old base. "0.6", "1.2": new base
                    );
#endif
    
#if 0
    Sp3ApiTest test("Normal",       // edit mode. "Normal", "User", "Cone", "Tree"
                    "Normal",       // type code. "Mormal", "Mark"
                    "Robot.stl", 	// stl filename
                    "32 20 20",     // position (X Y Z). NOTE: Z is the lowest postion of the model
                    "1 1 1",        // scale (X Y Z)
                    "0 60 0",       // rotation (X Y Z)
                    "false",        // inner support
                    "PartPro150 xP",// printer name
                    "MED",          // density. "MED", "LOW"
                    "0.4",          // contactSize. 0.15mm ~ 0.65mm
                    "0.6"           // newBaseZScale. "0": old base. "0.6", "1.2": new base
                    );
#endif
    test.Run();
    
    printf("DemoAP Ends!\n");
	return 0;
}
