#include "stdafx.h"
#include "SFCConversion.h"

//template< int nDims, int  mBits>
//void SFCConversion<nDims, mBits>::MortonEncode(void) // from n*m to m*n
//{
//	for (int i = 0; i < mBits; i++)//m
//	{
//		ptBits[i] = 0;
//		long mask = 1 << (mBits - i);
//
//		for (int j = 0; j < nDims; j++) //get one bit from each nDims
//		{
//			if (ptCoord[j] & mask) // both 1
//				ptBits[i] |= 1 << (nDims - j);// nDims iteration
//		}//
//	}//m group
//
//	//set the ouput point n;
//	ptBits.getBitLength(nDims);
//}

template< int nDims, int  mBits>
void SFCConversion<nDims, mBits>::MortonDecode(void)
{
	for (int i = 0; i < mBits; i++)//m n-bits
	{
		long ntemp = ptBits[i]; // each row in  m n-bits
		long mask = 1 << (mBits - i);

		for (int j = 0; j < nDims; j++)
		{
			if (ntemp & 1) //get the last bit from  each bits row 
				ptCoord[nDims - j - 1] |= mask;

			ntemp >>= 1; //move forward one bit
		}//
	}//n nDims
}


/*===========================================================*/
/* calc_P ---Get each key part from the input key*/
/*===========================================================*/
template< int nDims, int  mBits>
unsigned int SFCConversion<nDims, mBits>::calc_P3(int i, Point<long, mBits> H)
{
	/*unsigned int mask = ((unsigned int)1 << (mBits+1)) - 1;
	unsigned int P;//, temp2

	P = ((H.hcode[0] >> i) & mask);*/

	unsigned int P = H[(mBits * nDims - i) / nDims - 1];

	return P;
}

/*===========================================================*/
/* calc_P2 */
/*===========================================================*/
template< int nDims, int  mBits>
unsigned int SFCConversion<nDims, mBits>::calc_P2(unsigned int S)
{
	int i;
	unsigned int P;
	P = S & g_mask[0];
	for (i = 1; i < nDims; i++)
		if (S & g_mask[i] ^ (P >> 1) & g_mask[i])
			P |= g_mask[i];
	return P;
}

template< int nDims, int  mBits>
unsigned int SFCConversion<nDims, mBits>::calc_J(unsigned int P)
{
	int i;
	unsigned int J;
	J = nDims;
	for (i = 1; i < nDims; i++)
		if ((P >> i & 1) == (P & 1))
			continue;
		else
			break;
	if (i != nDims)
		J -= i;
	return J;
}

/*===========================================================*/
/* calc_T */
/*===========================================================*/
template< int nDims, int  mBits>
unsigned int SFCConversion<nDims, mBits>::calc_T(unsigned int P)
{
	if (P < 3)
		return 0;
	if (P % 2)
		return (P - 1) ^ (P - 1) / 2;
	return (P - 2) ^ (P - 2) / 2;
}
/*===========================================================*/
/* calc_tS_tT */
/*===========================================================*/
template< int nDims, int  mBits>
unsigned int SFCConversion<nDims, mBits>::calc_tS_tT(unsigned int xJ, unsigned int val)
{
	unsigned int retval, temp1, temp2;
	retval = val;
	if (xJ % nDims != 0)
	{
		temp1 = val >> (xJ % nDims);
		temp2 = val << (nDims - xJ % nDims);
		retval = temp1 | temp2;
		retval &= ((unsigned int)1 << nDims) - 1;
	}
	return retval;
}

/*===========================================================*/
/* calc_tS_tT */
/*===========================================================*/
template< int nDims, int  mBits>
unsigned int SFCConversion<nDims, mBits>::calc_tS_tT2(unsigned int xJ, unsigned int val)
{
	unsigned int retval, temp1, temp2;
	retval = val;
	if (xJ % nDims != 0)
	{
		temp1 = val << (xJ % nDims);
		temp2 = val >> (nDims - xJ % nDims);
		retval = temp1 | temp2;
		retval &= ((unsigned int)1 << nDims) - 1;
	}
	return retval;
}


template< int nDims, int  mBits>
void SFCConversion<nDims, mBits>::HilbertEncode(void) // from n*m to m*n
{
	unsigned int mask = (unsigned int)1 << mBits - 1, element,
		A, W = 0, S, tS, T, tT, J, P = 0, xJ;
	//Point h = { 0 };
	int i = mBits * nDims - nDims, j;

	A = 0;
	for (j = 0; j < nDims; j++)
	{
		if (ptCoord[j] & mask)
			A |= g_mask[j];
	}
	S = tS = A;
	P = calc_P2(S);
	/* add in nDims bits to hcode */
	//element = i / mBits;

	/*if (i % mBits > mBits - nDims)
	{
	h.hcode[element] |= P << (i % mBits);
	h.hcode[element + 1] |= P >> (mBits - i % mBits);
	}
	else
	h.hcode[element] |= P << (i - element * mBits);*/

	element = (mBits * nDims - i) / nDims - 1;
	ptBits[element] = P;
	///h.hcode[0] |= P << i;

	J = calc_J(P);
	xJ = J - 1;
	T = calc_T(P);
	tT = T;
	for (i -= nDims, mask >>= 1; i >= 0; i -= nDims, mask >>= 1)
	{
		A = 0;
		for (j = 0; j < nDims; j++)
		{
			if (ptCoord[j] & mask)
				A |= g_mask[j];
		}
		W ^= tT;
		tS = A ^ W;
		S = calc_tS_tT2(xJ, tS);
		P = calc_P2(S);
		/* add in nDims bits to hcode */
		//element = i / mBits;
		/*if (i % mBits > mBits - nDims)
		{
		h.hcode[element] |= P << (i % mBits);
		h.hcode[element + 1] |= P >> (mBits - i % mBits);
		}
		else
		h.hcode[element] |= P << (i - element * mBits);*/

		element = (mBits * nDims - i) / nDims - 1;
		ptBits[element] = P;
		//h.hcode[0] |= P << i;

		if (i > 0)
		{
			T = calc_T(P);
			tT = calc_tS_tT2(xJ, T);
			J = calc_J(P);
			xJ += J - 1;
		}
	}
	//return h;
}

template< int nDims, int  mBits>
void SFCConversion<nDims, mBits>::HilbertDecode(void)
{
	unsigned int mask = (unsigned int)1 << mBits - 1,
		A, W = 0, S, tS, T, tT, J, P = 0, xJ;
	//Point pt = { 0 };
	int i = mBits * nDims - nDims, j;

	P = calc_P3(i, ptBits); //get key part from the hilbert key

	J = calc_J(P);
	xJ = J - 1;
	A = S = tS = P ^ P / 2;
	T = calc_T(P);
	tT = T;
	/*--- distrib bits to coords ---*/
	for (j = nDims - 1; A > 0; A >>= 1, j--)
	{
		if (A & 1)
			ptCoord[j] |= mask;
	}
	/*pt.hcode[(nDims*mBits -i)/mBits -1] = A;*/

	for (i -= nDims, mask >>= 1; i >= 0; i -= nDims, mask >>= 1)
	{
		P = calc_P3(i, ptBits); //get key part from the hilbert key

		S = P ^ P / 2;
		tS = calc_tS_tT(xJ, S);
		W ^= tT;
		A = W ^ tS;
		/*--- distrib bits to coords ---*/
		for (j = nDims - 1; A > 0; A >>= 1, j--)
		{
			if (A & 1)
				ptCoord[j] |= mask;
		}
		/*pt.hcode[(nDims*mBits - i) / mBits - 1] = A;*/

		if (i >= 0)
		{
			T = calc_T(P);
			tT = calc_tS_tT(xJ, T);
			J = calc_J(P);
			xJ += J - 1;
		}
	}

	//return pt;	
}