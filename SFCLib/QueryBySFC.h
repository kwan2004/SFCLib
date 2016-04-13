#pragma once
#include<iostream>
#include <vector>
#include<stack>
using namespace std;

class QueryBySFC
{
public:
	int i;
	QueryBySFC();

	void get_result_in_vector(vector<vector<int>> vec, int N, stack<int> tmp, vector<vector<int>>& tmp_result)
	{
		for (int i = 0; i< vec[N].size(); ++i)
		{
			tmp.push(vec.at(N).at(i));
			if (N<vec.size() - 1)
			{
				get_result_in_vector(vec, N + 1, tmp, tmp_result);
			}
			else
			{
				vector<int>one_result;
				vector<int> tmp_vector;
				int count = 0;
				while (!tmp.empty())
				{
					tmp_vector.push_back(tmp.top());
					tmp.pop();
				}
				for (int i = 0; i<tmp.size(); ++i)
				{

					//one_result.push_back(tmp.at(i));
					one_result.push_back(tmp_vector[count - i]);
					tmp.push(tmp_vector[count - i]);
				}
				tmp_result.push_back(one_result);
			}
			if(!tmp.empty())tmp.pop();
			//tmp.pop();
		}
	}

	vector<vector<int>> get_all_combination(vector<vector<int>>& vec)
	{
		stack<int> tmp_vec;
		vector<vector<int>> tmp_result;
		get_result_in_vector(vec, 0, tmp_vec, tmp_result);
		return tmp_result;
	}
	
	void test()
	{
		vector<int> v1 = { 1, 3 };
		vector<int> v2 = { 2, 4 };
		vector<vector<int>> result = { v1, v2 };
		get_all_combination(result);
		std::cout << "result"<<"\n";
		for (int i = 0; i < result.size(); ++i) {
			for (int j = 0; j < result[i].size(); ++j) {
				std::cout << result[i][j] << "\t";
			}
			std::cout << "\n";
		}
	}
};

