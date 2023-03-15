#pragma once

namespace XYZSupport
{
	template <class T1, class T2, class T3, class T4>
	class Quadruple
	{
	private:
		T1 first;
		T2 second;
		T3 third;
		T4 fourth;
	public:
		T1 First() const { return first; }
		T2 Second() const { return second; }
		T3 Third() const { return third; }
		T4 Fourth() const { return fourth; }

		Quadruple()	{}

		Quadruple(T1 f, T2 s, T3 t, T4 u)
		{
			first = f;
			second = s;
			third = t;
			fourth = u;
		}

		Quadruple(const Quadruple<T1, T2, T3, T4>& quad)
		{
			first = quad.first;
			second = quad.second;
			third = quad.third;
			fourth = quad.fourth;
		}

		/*
		Charles: ADDED
		*/
		bool operator<(const Quadruple& other) const
		{
			if (first < other.First())
				return true;
			else
				return false;
		}
	};
	
	template <class T1, class T2, class T3>
	class Triple
	{
	private:
		T1 first;
		T2 second;
		T3 third;
	public:
		T1 First() const { return first; }
		T2 Second() const { return second; }
		T3 Third() const { return third; }
		
		Triple() {}

		Triple(T1 f, T2 s, T3 t)
		{
			first = f;
			second = s;
			third = t;
		}

		bool operator==(const Triple& other) const	//called by find(selectedTab3.begin(), selectedTab3.end(), target); in SupportMeshsBed.cpp.
		{
			if (first == other.First() &&
				second == other.Second() &&
				third == other.Third())
				return true;
			else
				return false;
		}

		bool operator<(const Triple& other) const	// for "set" container.
		{
			if (first == other.First() &&
				second == other.Second() &&
				third == other.Third())
				return false;

			if (first == other.First() &&
				second == other.Second())
				return third < other.Third();

			if (first == other.First())
				return second < other.Second();

			return first < other.First();
		}
	};
	
	template <class T1, class T2>
    class Tuple
	{
	private:
		T1 first;
		T2 second;
	public: 
		T1 First() const { return first; }
		T2 Second() const { return second; }

		Tuple() {}

		Tuple(T1 f, T2 s)
		{
			first = f;
			second = s;
		}

		/***********************************/
		
		// Need to verify the following operator<!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// 2019.11.25

		/***********************************/
		// NOTE: This override operator < is required for the sort function using in the getPickTouchListforTailforNewsup() function.
		bool operator<(const Tuple& other) const
		{
			if (first == other.First() &&
				second == other.Second())
				return false;

			if (first == other.First())
				return second < other.Second();

			return first < other.First();
		}

		//#region IEquatable<Tuple<T1,T2>> Members
		bool Equals(Tuple<T1, T2> other) const
		{
			return first.Equals(other.first) &&	second.Equals(other.second);
		}

		int GetHashCode()
		{
			return first.GetHashCode() ^ second.GetHashCode();
		}
		//#endregion
	};
}
