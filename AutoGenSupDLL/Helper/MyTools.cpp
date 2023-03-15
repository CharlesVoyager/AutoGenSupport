#include "stdafx.h"
#ifdef _WINDOWS
#include <Windows.h>	// for OutputDebugString()
#elif __APPLE__
#include <iostream>
#include <fstream>
#endif
#include <algorithm>    // std::min
#include <codecvt>
#include "MyTools.h"

void DbgMsg(const char *zcFormat, ...)
{
	// initialize use of the variable argument array
	va_list vaArgs;
	va_start(vaArgs, zcFormat);

	// reliably acquire the size
	// from a copy of the variable argument array
	// and a functionally reliable call to mock the formatting
	va_list vaArgsCopy;
	va_copy(vaArgsCopy, vaArgs);
	const int iLen = std::vsnprintf(nullptr, 0, zcFormat, vaArgsCopy);
	va_end(vaArgsCopy);

	// return a formatted string without risking memory mismanagement
	// and without assuming any compiler or platform specific behavior
	std::vector<char> zc(iLen + 1);
	std::vsnprintf(zc.data(), zc.size(), zcFormat, vaArgs);
	va_end(vaArgs);
	std::string strText(zc.data(), iLen);

	strText.insert(0, "[XYZ]");
	strText += "\n";
#ifdef _WINDOWS
	OutputDebugStringA(strText.c_str());
#elif __APPLE__
	// Unmark the line below to output the debug message to stderr.
	//std::cerr << strText.c_str();
#ifdef DEBUG
		
	TEST TEST TEST

    std::ofstream outfile;
    outfile.open("/Users/cp23dsw/log.txt", std::ios_base::app);    //append instead of overwrite
    outfile << strText.c_str();
#endif
#endif
}

void DbgMsgW(const wchar_t * format, ...)
{
	// initialize use of the variable argument array
	va_list vaArgs;
	va_start(vaArgs, format);

	// reliably acquire the size
	// from a copy of the variable argument array
	// and a functionally reliable call to mock the formatting
	va_list vaArgsCopy;
	va_copy(vaArgsCopy, vaArgs);
	const int iLen = std::vswprintf(nullptr, 0, format, vaArgsCopy);
	va_end(vaArgsCopy);

	// return a formatted string without risking memory mismanagement
	// and without assuming any compiler or platform specific behavior
	std::vector<wchar_t> zc(iLen + 1);
	std::vswprintf(zc.data(), zc.size(), format, vaArgs);
	va_end(vaArgs);
	std::wstring strText(zc.data(), iLen);

	strText.insert(0, L"[XYZ]");
	strText += L"\n";
#ifdef _WINDOWS
	OutputDebugStringW(strText.c_str());
#elif __APPLE__
#ifdef DEBUG	// Unable to write wstring. Need to check...
    std::ofstream outfile;
    outfile.open("/Users/cp23dsw/log.txt", std::ios_base::app);    //append instead of overwrite
    outfile << strText.c_str();
#endif
#endif
}

namespace MyTools
{
	void PrintMatrix4(const float matrix[16])
	{
		printf("                      %f %f %f %f\n", matrix[0], matrix[4], matrix[8], matrix[12]);
		printf("                      %f %f %f %f\n", matrix[1], matrix[5], matrix[9], matrix[13]);
		printf("                      %f %f %f %f\n", matrix[2], matrix[6], matrix[10], matrix[14]);
		printf("                      %f %f %f %f\n", matrix[3], matrix[7], matrix[11], matrix[15]);
	}

    // reference from Clipper test program.
    wstring str2wstr(const std::string &s)
    {
#ifdef _WINDOWS
        int slength = (int)s.length() + 1;
        int len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
        wchar_t* buf = new wchar_t[len];
        MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
        std::wstring r(buf);
        delete[] buf;
        return r;
#elif __APPLE__
        wstring wstr(s.begin(), s.end());
        return wstr;
#endif
    }
    
	wstring str2wstr1(const string& s)
	{
#ifdef _WINDOWS
		std::string curLocale = setlocale(LC_ALL, "");
		const char* _Source = s.c_str();

		wchar_t *_Dest = new wchar_t[s.length() + 1];
		wmemset(_Dest, 0, s.length() + 1);
		size_t outSize;
		mbstowcs_s(&outSize, _Dest, s.length() + 1, _Source, s.length());
		std::wstring result = _Dest;
		delete[]_Dest;
		setlocale(LC_ALL, curLocale.c_str());
		return result;
#elif __APPLE__
        wstring wstr(s.begin(), s.end());
        return wstr;
#endif
	}

    string wstr2str(const std::wstring &wstr)
    {
        using convert_typeX = std::codecvt_utf8<wchar_t>;
        std::wstring_convert<convert_typeX, wchar_t> converterX;
        return converterX.to_bytes(wstr);
    }

	vector<string> StringSplit(string strWhole, const char *separator)
	{
		vector<string> vecStr;
		string strSingle;
		string printerName;

		std::size_t found;
		do {
			found = strWhole.find_first_of(separator);
			strSingle = strWhole.substr(0, found);
			strWhole = strWhole.substr(found + 1);

			if (!strSingle.empty())
				vecStr.push_back(strSingle);

		} while (!strWhole.empty() && found != std::string::npos);
		return vecStr;
    }
    
    bool StringToFloatXYZ(const string &str, float xyz[3])
    {
        vector<string> strArray = StringSplit(str, " ");
        if(strArray.size() >= 3)
        {
            for(int i=0; i<3; i++)
                xyz[i] = stof(strArray[i].c_str());
            return true;
        }
        else
            return false;
	}
	// Convert 4 bytes to unsigned int and move the input pointer
	unsigned int ReadUInt32(unsigned char *&currentPointer)
	{
		unsigned int output;

		*((unsigned char*)(&output) + 0) = *currentPointer; currentPointer++;
		*((unsigned char*)(&output) + 1) = *currentPointer; currentPointer++;
		*((unsigned char*)(&output) + 2) = *currentPointer; currentPointer++;
		*((unsigned char*)(&output) + 3) = *currentPointer; currentPointer++;

		return output;
	}

	// Convert 4 bytes to float and move the input pointer
	float ReadSingle(unsigned char *&currentPointer)
	{
#if 1
        float *pf = reinterpret_cast<float *>(currentPointer);
        currentPointer += 4;
        return *pf;
#endif
#if 0
        float output;
        *((unsigned char*)(&output) + 0) = *currentPointer; currentPointer++;
        *((unsigned char*)(&output) + 1) = *currentPointer; currentPointer++;
        *((unsigned char*)(&output) + 2) = *currentPointer; currentPointer++;
        *((unsigned char*)(&output) + 3) = *currentPointer; currentPointer++;
        return output;
#endif
    }
    
    void WriteUInt32(unsigned char *&currentPointer, unsigned int i)
    {
        *currentPointer = i & 0xFF; currentPointer++;
        *currentPointer = (i >> 8) & 0xFF; currentPointer++;
        *currentPointer = (i >> 16) & 0xFF; currentPointer++;
        *currentPointer = (i >> 24) & 0xFF; currentPointer++;
    }
	void WriteSingle(unsigned char *&currentPointer, float f)
	{
		unsigned char const * p = reinterpret_cast<unsigned char const *>(&f);

		*currentPointer = p[0]; currentPointer++;
		*currentPointer = p[1]; currentPointer++;
		*currentPointer = p[2]; currentPointer++;
		*currentPointer = p[3]; currentPointer++;
	}

	BoundingBox GetModelBoundingBox(unsigned char *stlBinary)
	{
		unsigned char *r = stlBinary + 80;	// pointer to start parse address
		unsigned int nTri = ReadUInt32(r);	// number of triangles

		BoundingBox bbox;

		bbox.minPoint.x = std::numeric_limits<float>::infinity();
		bbox.minPoint.y = std::numeric_limits<float>::infinity();
		bbox.minPoint.z = std::numeric_limits<float>::infinity();

		bbox.maxPoint.x = -1 * std::numeric_limits<float>::infinity();
		bbox.maxPoint.y = -1 * std::numeric_limits<float>::infinity();
		bbox.maxPoint.z = -1 * std::numeric_limits<float>::infinity();

		unsigned char *w = nullptr;
		for (unsigned int i = 0; i < nTri; i++)
		{
			float x, y, z;

			x = ReadSingle(r);
			y = ReadSingle(r);
			z = ReadSingle(r);

			w = r;
			for (int i = 0; i < 3; i++)
			{
				x = ReadSingle(r);
				bbox.minPoint.x = min(bbox.minPoint.x, x);
				bbox.maxPoint.x = max(bbox.maxPoint.x, x);

				y = ReadSingle(r);
				bbox.minPoint.y = min(bbox.minPoint.y, y);
				bbox.maxPoint.y = max(bbox.maxPoint.y, y);

				z = ReadSingle(r);
				bbox.minPoint.z = min(bbox.minPoint.z, z);
				bbox.maxPoint.z = max(bbox.maxPoint.z, z);
			}
			r++;	//skip two bytes
			r++;
		}
		return bbox;
	}

	// Refered from PrintModel.cs -> ResetVertexPosToBBox().
	void ResetVertexPosToBBox(unsigned char *stlBinary, Vector3 bboxCenter)
	{
		unsigned char *r = stlBinary + 80;	// pointer to start parse address
		unsigned int nTri = ReadUInt32(r);	// number of triangles

		unsigned char *w = nullptr;
		float x, y, z;
		for (unsigned int i = 0; i < nTri; i++)
		{
			// normal. Just read. Don't need to change.
			x = ReadSingle(r);
			y = ReadSingle(r);
			z = ReadSingle(r);

			w = r;
			for (int j = 0; j < 3; j++)
			{
				x = ReadSingle(r);
				WriteSingle(w, x - bboxCenter.x);

				y = ReadSingle(r);
				WriteSingle(w, y - bboxCenter.y);

				z = ReadSingle(r);
				WriteSingle(w, z - bboxCenter.z);
			}
			r++;	//skip two bytes
			r++;
		}
	}

	unsigned int GetTrianglesCount(const unsigned char *stlBinary) 
	{
		const unsigned char *r = stlBinary + 80;	// pointer to start parse address

		unsigned int output;

		*((unsigned char*)(&output) + 0) = *r;
		*((unsigned char*)(&output) + 1) = *(r + 1);
		*((unsigned char*)(&output) + 2) = *(r + 2);
		*((unsigned char*)(&output) + 3) = *(r + 3);

		return output;				// number of triangles
	}

	void TransformPoint(const Vector3 &v, Vector3 &outv, const float trans[16])
	{
		float column0[4];
		float column1[4];
		float column2[4];

		/*NOTE: C++ Matrix4 order
				| 0  4  8 12 |
				| 1  5  9 13 |
				| 2  6 10 14 |
				| 3  7 11 15 |*/
		memcpy(column0, trans, sizeof(column0));
		memcpy(column1, trans + 4, sizeof(column1));
		memcpy(column2, trans + 8, sizeof(column2));

		outv.x = column0[0] * v.x + column0[1] * v.y + column0[2] * v.z + column0[3] * 1;
		outv.y = column1[0] * v.x + column1[1] * v.y + column1[2] * v.z + column1[3] * 1;
		outv.z = column2[0] * v.x + column2[1] * v.y + column2[2] * v.z + column2[3] * 1;
	}

	int ModelStlBinaryToWorldStlBinary( /*in*/ const unsigned char *stlBinary, const float trans[16], vector<unsigned char> &vecBufferWor)
	{
		if (stlBinary == nullptr) return -2;				// invalied input

		unsigned char *r = (unsigned char *)stlBinary + 80;	// pointer to start parse address
		unsigned int nTri = ReadUInt32(r);					// number of triangles

		vecBufferWor.resize(84 + (nTri * 50), 0);
		unsigned char *w = &vecBufferWor[0] + 80;

		WriteUInt32(w, nTri);

		float x, y, z;
		for (unsigned int i = 0; i < nTri; i++)
		{
			// copy the normal vector, 12 bytes
			memcpy(w, r, 12);
			r += 12;	//advance pointer, 12 bytes
			w += 12;	//advance pointer, 12 bytes

			x = ReadSingle(r);
			y = ReadSingle(r);
			z = ReadSingle(r);
			Vector3 v1(x, y, z);
			Vector3 verWor;
			TransformPoint(v1, verWor, trans);		// to world-coordination
			WriteSingle(w, (float)verWor.x);
			WriteSingle(w, (float)verWor.y);
			WriteSingle(w, (float)verWor.z);

			x = ReadSingle(r);
			y = ReadSingle(r);
			z = ReadSingle(r);
			Vector3 v2(x, y, z);
			TransformPoint(v2, verWor, trans);		// to world-coordination
			WriteSingle(w, (float)verWor.x);
			WriteSingle(w, (float)verWor.y);
			WriteSingle(w, (float)verWor.z);

			x = ReadSingle(r);
			y = ReadSingle(r);
			z = ReadSingle(r);
			Vector3 v3(x, y, z);
			TransformPoint(v3, verWor, trans);		// to world-coordination
			WriteSingle(w, (float)verWor.x);
			WriteSingle(w, (float)verWor.y);
			WriteSingle(w, (float)verWor.z);

			r += 2;	//skip two bytes
			w += 2; //skip two bytes
		}
		return 0;
	}

	int MeshDataToStlFile(const vector<MeshData> &vecStlBinary, string filename)
	{
		vector<unsigned char> totalStlBufferWor(84, 0);	//initial size in 84 bytes with initial value 0.
		int totalTriangleCount = 0;
		for (const auto &meshData : vecStlBinary)
		{
			vector<unsigned char> stlBufferWor;
			ModelStlBinaryToWorldStlBinary(meshData.mesh, meshData.trans, stlBufferWor);
			totalTriangleCount += GetTrianglesCount(&stlBufferWor[0]);
			totalStlBufferWor.insert(totalStlBufferWor.end(), stlBufferWor.begin() + 84, stlBufferWor.end());
		}
		// update total triange count in totalStlBufferWor
		unsigned char *w = &totalStlBufferWor[0] + 80;
		WriteUInt32(w, totalTriangleCount);

		// output the file
		BinFile binFile(filename);
		binFile.Write(&totalStlBufferWor[0], totalStlBufferWor.size());
		return 0;
	}

	bool ReadAllBytes(const string &filename, /*out*/ vector<unsigned char> &vecAllBytes)
	{
		FILE * pFile;
		long lSize;
		size_t result;

		pFile = fopen(filename.c_str(), "rb");
		if (pFile == nullptr) return false;

		// obtain file size:
		fseek(pFile, 0, SEEK_END);
		lSize = ftell(pFile);

		rewind(pFile);

		// allocate memory to contain the whole file:
		vecAllBytes.resize(lSize);

		// copy the file into the buffer:
		result = fread(&vecAllBytes[0], 1, lSize, pFile);
		if (result != lSize)
		{
			fclose(pFile);
			return false;	// reading error;
		}

		/* the whole file is now loaded in the memory buffer. */
		// terminate
		fclose(pFile);
		return true;
	}

	unsigned char *ReadAllBytes(const string &filename)
	{
		FILE *pFile = fopen(filename.c_str(), "rb");	//Note: The file must be open in binary mode; otherwise, the file will be add more extra bytes (0D 0A).
		if (pFile == nullptr)
		{
			printf(" ==> Error: %s was not opened\n", filename.c_str());
			return nullptr;
		}

		unsigned char *buffer;
		unsigned long fileLen;

		//Get file length
		fseek(pFile, 0, SEEK_END);
		fileLen = ftell(pFile);
		fseek(pFile, 0, SEEK_SET);

		//Allocate memory
		buffer = (unsigned char *)malloc(fileLen + 1);
		if (!buffer)
		{
			fprintf(stderr, "Memory error!");
			fclose(pFile);
			return nullptr;
		}

		//Read file contents into buffer
		fread(buffer, 1, fileLen, pFile);
		fclose(pFile);
		return buffer;
	}
	/********************************************************************************************************************************/
	Stopwatch::Stopwatch()
	{
		Reset();
	}

	Stopwatch::~Stopwatch()
	{
	}

	void Stopwatch::Start()
	{
		if (isStarted) return;
		start = Clock::now();
		isStarted = true;
	}

	void Stopwatch::Stop()
	{
		if (!isStarted) return;
		end = Clock::now();
		isStopped = true;

		totalElapsedMilliseconds += (long)std::chrono::duration_cast<TimeUnit>(end - start).count();
		isStarted = false;
	}

	void Stopwatch::Reset()
	{
		isStarted = false;
		end = start;
		isStopped = false;
		totalElapsedMilliseconds = 0;
	}

	long Stopwatch::ElapsedMilliseconds()
	{
		if (!isStopped)	Stop();
		return totalElapsedMilliseconds;
	}
	/********************************************************************************************************************************/
}