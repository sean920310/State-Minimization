#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <unordered_set>

using namespace std;

typedef struct State
{
	string name;
	vector<string> state;
	vector<bool> output;
}State;

namespace std
{
	template<>
	struct hash<State>
	{
		size_t operator()(const State& key) const
		{
			return hash<std::string>()(key.name);
		}
	};
	template<>
	struct equal_to<State>
	{
		bool operator()(const State& lhs,const State& rhs) const
		{
			return lhs.name == rhs.name;
		}
	};
}

typedef struct StateOutput
{
	vector<string> state;
	vector<bool> output;
	
	StateOutput(vector<string> state ,vector<bool> output) :state(state), output(output) {}
	StateOutput(const State& rhs) :state(rhs.state), output(rhs.output) {}
	bool operator ==(const StateOutput& rhs)
	{
		return (state == rhs.state) && (output == rhs.output);
	}
}StateOutput;



typedef struct pairState
{
	State stateA, stateB;
	bool incompatible;

	pairState(State stateA, State stateB, bool incompatible):stateA(stateA), stateB(stateB), incompatible(incompatible){}
	pairState(const pairState& rhs)
	{
		incompatible = rhs.incompatible;
		stateA = rhs.stateA;
		stateB = rhs.stateB;
	}
}pairState;

void reduceState(map<string, State>& states);






int main(int argc, char* argv[])
{
	//if (argc < 3)
	//{
	//	cout << "arg error";
	//	return 0;
	//}
	//string inputFileName = argv[1];
	//string outputFileName = argv[2];
	string inputFileName = "input.kiss";
	string outputFileName = "output.kiss";

	fstream input(inputFileName, ios::in);
	vector<string> implicants, essentialPI, dontCares, result;
	map<string, State> states;
	vector<string> inputVar, outputVar;
	string startState;
	int inputNum = 0, outputNum = 0, productNum = 0, stateNum = 0;

	// read file
	while (!input.eof())
	{
		string buf, op;
		getline(input, buf);
		stringstream ss(buf);
		ss >> op;

		if (op == ".i")
		{
			ss >> inputNum;
		}
		else if (op == ".o")
		{
			ss >> outputNum;
		}
		else if (op == ".p")
		{
			ss >> productNum;
		}
		else if (op == ".s")
		{
			ss >> stateNum;
		}
		else if (op == ".r")
		{
			ss >> startState;
		}
		else if (op == ".start_kiss")
		{
			continue;
		}
		else if (op == ".end_kiss")
		{
			break;
		}
		else
		{
			int output;
			string crntVar,nextVar;
			ss >> crntVar;
			ss >> nextVar;
			ss >> output;
			if (!states.count(crntVar))
			{
				states[crntVar] = State();
				states[crntVar].state = vector<string>(inputNum);
				states[crntVar].output = vector<bool>(inputNum);
			}
			states[crntVar].name = crntVar;
			char* pEnd;
			int i = strtol(op.c_str(), &pEnd, 2);
			states[crntVar].state[i] = nextVar;
			states[crntVar].output[i] = output;
		}
	}


	//minimize state
	reduceState(states);


	//output file

}



//*************************************************************************
//
// * reduce state
//=========================================================================
void reduceState(map<string, State>& states)
{
	set<pair<bool, bool>> outputType;
	vector<pair<string, State>> tempState;
	for (const auto& v : states)
	{
		//copy to temp
		tempState.push_back(v);

		outputType.insert(pair<bool, bool>(v.second.lowOutput, v.second.highOutput));
	}

	while (true)
	{
		//find same output
		set<pair<string,string>> sameOutput;
		for (int i = 0; i < tempState.size() - 1; i++)
		{
			StateOutput first = tempState[i].second;
			for (int j = i + 1; j < tempState.size(); j++)
			{
				StateOutput second = tempState[j].second;
				if (first == second)
					sameOutput.insert(pair<string, string>(tempState[i].first, tempState[j].first));
			}
		}

		if (!sameOutput.size())
			break;

		//remove same output term
		for (const pair<string, string>& term : sameOutput)
		{
			for (int i = 0; i < tempState.size(); i++)
			{
				if (tempState[i].second.highState == term.second)
					tempState[i].second.highState = term.first;
				if (tempState[i].second.lowState == term.second)
					tempState[i].second.lowState = term.first;
				if (tempState[i].first == term.second)
				{
					tempState.erase(tempState.begin() + i);
					i--;
				}
			}
		}
	}

	//reduse term by classify output
	vector<vector<State>> classifyOutput(outputType.size(),vector<State>());
	vector<State> temp;
	for (const auto& state : tempState)
	{
		int i = 0;
		for (const auto& type: outputType)
		{
			if (pair<bool, bool>(state.second.lowOutput, state.second.highOutput) == type)
			{
				classifyOutput[i].push_back(state.second);
				break;
			}
			i++;
		}
	}
	//將不同輸出的set分成不同的set
	while (true)
	{
		map<string, pair<int,int>> outputIndex;
		for (int i = 0; i < classifyOutput.size(); i++)
		{
			for (int j = 0; j < classifyOutput[i].size(); j++)
			{
				pair<int, int> classify;
				int lowIndex, highIndex;
				for (int m = 0; m < classifyOutput.size(); m++)
				{
					for (int n = 0; n < classifyOutput[m].size(); n++)
					{
						if (classifyOutput[i][j].lowState == classifyOutput[m][n].name)
							lowIndex = m;
						if (classifyOutput[i][j].highState == classifyOutput[m][n].name)
							highIndex = m;
					}
				}
				classify.first = lowIndex;
				classify.second = highIndex;

				outputIndex[classifyOutput[i][j].name] = classify;
			}
		}

		vector<vector<State>> newClassifyOutput;
		for (int i = 0; i < classifyOutput.size(); i++)
		{
			if (classifyOutput[i].size() > 1)	//重新分類之後放入新的
			{
				vector<pair<pair<int, int>,unordered_set<State>>> outputSetCount;
				//不同的輸出結果分類
				for (int j = 0; j < classifyOutput[i].size(); j++)
				{
					bool find = false;
					for (auto& set : outputSetCount)
					{
						if(set.first == outputIndex[classifyOutput[i][j].name])
						{
							set.second.insert(classifyOutput[i][j]);
							find = true;
						}
					}
					if (!find)
					{
						outputSetCount.push_back(pair<pair<int, int>, unordered_set<State>>(outputIndex[classifyOutput[i][j].name], unordered_set<State>()));
						outputSetCount[outputSetCount.size()-1].second.insert(classifyOutput[i][j]);
					}
				}
				//把不同的結果放到新的set
				for (const auto& outputSet : outputSetCount)
				{
					vector<State> newSet;
					for (const auto& state : outputSet.second)
					{
						newSet.push_back(state);
					}
					newClassifyOutput.push_back(newSet);
				}
			}
			else	//如果只有一個就直接放入新的
				newClassifyOutput.push_back(classifyOutput[i]);
		}
		//如果沒有再分出新的set就跳出迴圈
		if (classifyOutput.size() != newClassifyOutput.size())
		{
			classifyOutput = newClassifyOutput;
		}
		else
		{
			classifyOutput = newClassifyOutput;
			break;
		}
	}

	//取代所有重複的term
	for (int i = 0; i < classifyOutput.size(); i++)
	{
		if (classifyOutput.size() != 1)
		{
			for (int j = 1; j < classifyOutput[i].size(); j++)
			{
				for (int m = 0; m < tempState.size(); m++)
				{
					if (tempState[m].second.highState == classifyOutput[i][j].name)
						tempState[m].second.highState = classifyOutput[i][0].name;
					if (tempState[m].second.lowState == classifyOutput[i][j].name)
						tempState[m].second.lowState = classifyOutput[i][0].name;
					if (tempState[m].first == classifyOutput[i][j].name)
					{
						tempState.erase(tempState.begin() + m);
						m--;
					}
				}
			}
		}
	}

	//TODO: output;

}
