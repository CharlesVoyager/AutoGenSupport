#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <cstdarg>

using namespace std;

void DbgMsg(const char *zcFormat, ...);
void DbgMsgW(const wchar_t * format, ...);

namespace MyTools
{
	struct MeshData
	{
		unsigned char *mesh;
		float trans[16];
	};

	struct Vector3
	{
	public:
		float x;
		float y;
		float z;

		Vector3(float _x, float _y, float _z)
		{
			x = _x;
			y = _y;
			z = _z;
		}
		Vector3()
		{
			x = y = z = -1;
		}
	};

	struct BoundingBox
	{
	public:
		Vector3 minPoint;
		Vector3 maxPoint;

		Vector3 GetCenter() {
			return Vector3((minPoint.x + maxPoint.x) / 2.0f, (minPoint.y + maxPoint.y) / 2.0f, (minPoint.z + maxPoint.z) / 2.0f);
		}
	};

	wstring str2wstr(const std::string &s);// reference from Clipper test program.
	wstring str2wstr1(const std::string &s);
    string wstr2str(const std::wstring &wstr);
	vector<string> StringSplit(string strWhole, const char *separator);
    bool StringToFloatXYZ(const string &str, float xyz[3]);

	void PrintMatrix4(const float matrix[16]);

	BoundingBox GetModelBoundingBox(unsigned char *stlBinary);

	void ResetVertexPosToBBox(unsigned char *stlBinary, Vector3 bboxCenter);
	unsigned int GetTrianglesCount(const unsigned char *stlBinary);
	
	// Convert 4 bytes to unsigned int and move the input pointer
	unsigned int ReadUInt32(unsigned char *&currentPointer);
	// Convert 4 bytes to float and move the input pointer
    float ReadSingle(unsigned char *&currentPointer);
    void WriteUInt32(unsigned char *&currentPointer, unsigned int i);
    void WriteSingle(unsigned char *&currentPointer, float f);

	int ModelStlBinaryToWorldStlBinary( /*in*/ const unsigned char *stlBinary, const float trans[16], vector<unsigned char> &vecBufferWor);
	int MeshDataToStlFile(const vector<MeshData> &vecStlBinary, string filename);

	bool ReadAllBytes(const string &filename, /*out*/ vector<unsigned char> &vecAllBytes);
	// Note that the caller has to free the buffer.
	unsigned char *ReadAllBytes(const string &filename);

	class TextFile
	{
	private:
		FILE *m_pFile;

	public:
		TextFile() { m_pFile = nullptr; }
		TextFile(const string& filename)
		{
			m_pFile = fopen(filename.c_str(), "w");	// fopen ==> is C standard library, fopen_s ==> is Microsoft C libary.
		}
		~TextFile()
		{
			if (m_pFile != nullptr)
			{
				fclose(m_pFile);
				m_pFile = nullptr;
			}
		}
		bool IsSuccess() { return m_pFile != nullptr; }
		void Write(const char *zcFormat, ...)
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

			fwrite(strText.c_str(), sizeof(char), strText.length(), m_pFile);
		}
	};

	class BinFile
	{
	private:
		FILE *m_pFile;

	public:
		BinFile() { m_pFile = nullptr; }
		BinFile(const string& filename)
		{
			m_pFile = fopen(filename.c_str(), "wb");
		}
		BinFile(const wstring& wFilename)
		{
	#ifdef _WINDOWS
			m_pFile = _wfopen(wFilename.c_str(), L"wb");
	#elif __APPLE__
			string strFilename(wFilename.begin(), wFilename.end());	
			m_pFile = fopen(strFilename.c_str(), "wb");
	#endif
		}
		~BinFile()
		{
			if (m_pFile != nullptr)
			{
				fclose(m_pFile);
				m_pFile = nullptr;
			}
		}
		void Open(const string& filename)
		{
			if (m_pFile != nullptr) Close();

			m_pFile = fopen(filename.c_str(), "wb");
		}
		bool IsSuccess() { return m_pFile != nullptr; }
		void Write(const void *buffer, size_t _ElementSize, size_t _ElementCount)
		{
			if (m_pFile != nullptr)
				fwrite(buffer, _ElementSize, _ElementCount, m_pFile);
		}
		void Write(const void *buffer, size_t _ElementCount)
		{
			Write(buffer, 1, _ElementCount);
		}
		void Write(const vector<unsigned char>& buffer)
		{
			if (m_pFile != nullptr)
				fwrite(&buffer[0], 1, buffer.size(), m_pFile);
		}
		void Close()
		{
			if (m_pFile != nullptr)
			{
				fclose(m_pFile);
				m_pFile = nullptr;
			}
		}
	};

	class Stopwatch
	{
	private:
		typedef std::chrono::milliseconds TimeUnit;
		typedef std::chrono::high_resolution_clock Clock;

		std::chrono::time_point<Clock> start;
		std::chrono::time_point<Clock> end;
		bool isStarted;
		bool isStopped;

		long totalElapsedMilliseconds;

	public:
		Stopwatch();
		~Stopwatch();

		void Start();
		void Stop();
		void Reset();
		long ElapsedMilliseconds();
	};
}
