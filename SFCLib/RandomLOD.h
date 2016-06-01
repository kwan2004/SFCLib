#ifndef RANDOMLOD_H_
#define RANDOMLOD_H_

#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

template<int nDims>
class RandomLOD
{
private:
	int m_nlvlnum; // from 0 to m_nlvlnum -1; total is m_nlvlnum
	int m_nLevelMax;

	double* m_pintervals;// size; [nLevelMax + 1];the intervals used to catach the input point; totoal levels are nLevelMax;
	
public:
	int ntest;

	RandomLOD(int lvl_num, int nLevelMax)
	{
		ntest = 0;

		m_nlvlnum = lvl_num;
		m_nLevelMax = nLevelMax;
		m_pintervals = new double[nLevelMax + 1];

		srand(time(NULL));

		unsigned long long* nintervals = new unsigned long long[nLevelMax + 1];//[nLevelMax + 1];

		memset(m_pintervals, 0, sizeof(double)*(nLevelMax + 1));
		memset(nintervals, 0, sizeof(unsigned long long)*(nLevelMax + 1));
		//long long ntotal = 0;		
		
		nintervals[0] = 0;
		nintervals[1] = 1;

		int i;
		for (i = 1; i < nLevelMax; i++)
		{
			nintervals[i + 1] = nintervals[i] * ( ( ((long long)1) << nDims) - 1);

			///ntotal += nintervals[i + 1];
		}
		
		m_pintervals[0] = 0;
		for (i = 1; i <= nLevelMax; i++)
		{
			m_pintervals[i] = (double)nintervals[i] / nintervals[nLevelMax] * 100; //ntotal
		}
		
		delete[] nintervals;
	}

	~RandomLOD()
	{
		if (m_pintervals != NULL)
		{
			delete[] m_pintervals;
		}
	}

	int RLOD_Gen()
	{
		double rnd = ((double)rand() / (RAND_MAX)) * 100;
		int n_level = m_nlvlnum - 1;

		//to check which interval this random value will fall in
		for (int i = m_nLevelMax; i > 0; i--)
		{
			if (rnd > m_pintervals[i - 1] && rnd <= m_pintervals[i])
			{
				n_level = i - (m_nLevelMax - m_nlvlnum + 1);
				break;
			}
		}


		// to check if it fall out of the scopes. the target levels are 0 ~ m_nlvlnum -1
		///  0 ~ m_nlvlnum -1 --->>> 33-m_nlvlnum ~  32; if out of scope ,put it in the bottom level
		if (n_level < 0 || n_level > m_nlvlnum - 1)
		{
			ntest++;
			n_level = m_nlvlnum - 1;
		}

		return n_level;
	}

};


#endif