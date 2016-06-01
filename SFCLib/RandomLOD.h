#ifndef RANDOMLOD_H_
#define RANDOMLOD_H_

#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

template<int nDims, int nLevelMax>
class RandomLOD
{
private:
	int m_nlvlnum; // from 0 to m_nlvlnum -1; total is m_nlvlnum

	double m_fintervals[nLevelMax+1]; //the intervals used to catach the input point; totoal levels are nLevelMax;

public:
	int ntest;

	RandomLOD(int lvl_num)
	{
		ntest = 0;

		m_nlvlnum = lvl_num;

		srand(time(NULL));

		long long nintervals[nLevelMax + 1];
		//long long ntotal = 0;		
		
		nintervals[0] = 0;
		nintervals[1] = 1;
		for (int i = 1; i < nLevelMax; i++)
		{
			nintervals[i + 1] = nintervals[i] * ((((long long)1) <<nDims)-1);

			///ntotal += nintervals[i + 1];
		}
		
		m_fintervals[0] = 0;
		for (int i = 1; i <= nLevelMax; i++)
		{
			m_fintervals[i] = (double)nintervals[i] / nintervals[nLevelMax] * 100; //ntotal
		}
	}

	int RLOD_Gen()
	{
		double rnd = ((double)rand() / (RAND_MAX)) * 100;
		int n_level = m_nlvlnum - 1;

		//to check which interval this random value will fall in
		for (int i = nLevelMax; i > 0; i--)
		{
			if (rnd > m_fintervals[i - 1] && rnd <= m_fintervals[i])
			{
				n_level = i - (nLevelMax - m_nlvlnum + 1);
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