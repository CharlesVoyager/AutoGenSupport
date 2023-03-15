# AutoGenSupport
Automatically generate support fors for 3D modules in STL file format.

Compiling
=========
- Visual Studio 2022
- C++
- Windows SDK 10.0

Using it
========
- Compile Slicer2.0DLL. This a module that used to convert 3D objects in STL file format to vectors.
- Compile AutoGenSupDLL.
- Complie AutoGenSupTestAP. 
- Goto release folder and check the content of AutoGenSupTestAP.ini if you want to change some settings.
- Run AutoGenSupTestAP.exe.
- Then the output STL file with supports will be generated in the subforlder - .\Output.

Charles Chang
2023/3/15
