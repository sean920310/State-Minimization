#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>

using namespace std;

typedef struct State
{
	string name;
	string lowState, highState;
	bool lowOutput, highOutput;
}State;

typedef struct StateOutput
{
	string lowState, highState;
	bool lowOutput, highOutput;
	
	StateOutput(string lowState, string highState, bool lowOutput, bool highOutput) :lowState(lowState), highState(highState), lowOutput(lowOutput), highOutput(highOutput){}
	StateOutput(const State& rhs) :lowState(rhs.lowState), highState(rhs.highState), lowOutput(rhs.lowOutput), highOutput(rhs.highOutput) {}
	bool operator ==(const StateOutput& rhs)
	{
		return (lowState == rhs.lowState) && (highState == rhs.highState) && (lowOutput == rhs.lowOutput) && (highOutput == rhs.highOutput);
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
			if (!states.count(crntVar)) states[crntVar] = State();
			states[crntVar].name = crntVar;
			if (stoi(op))		//input high
			{
				states[crntVar].highState = nextVar;
				states[crntVar].highOutput = output;
			}
			else				//input low
			{
				states[crntVar].lowState = nextVar;
				states[crntVar].lowOutput = output;
			}
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
	vector<pairState> implicantTable;
	set<string> incompatibleState;
	map<pair<bool, bool>,int> outputState;
	vector<pair<string, State>> tempState;
	for (const auto& v : states)
	{
		//copy to temp
		tempState.push_back(v);

		//find incompatible pair
		if (outputState.count(pair<bool, bool>(v.second.lowOutput, v.second.highOutput)))
			outputState[pair<bool, bool>(v.second.lowOutput, v.second.highOutput)]++;
		else
			outputState[pair<bool, bool>(v.second.lowOutput, v.second.highOutput)] = 1;
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


	//setup implication table
	for (int i = 0; i < tempState.size()-1; i++)
	{
		for (int j = i + 1; j < tempState.size(); j++)
		{
			implicantTable.push_back(pairState(tempState[i].second, tempState[j].second,false));
		}
	}

	//find incompatible pair
	for (const auto& outputVal : outputState)
	{
		if (outputVal.second == 1)
		{
			for (const auto& stateVal : tempState)
			{
				if (pair<bool, bool>(stateVal.second.lowOutput, stateVal.second.highOutput) == outputVal.first)
				{
					incompatibleState.insert(stateVal.first);
					break;
				}
			}
		}
	}

	//set incompatible pair to true
	for (auto& val : implicantTable)
	{
		for (const auto& incompState : incompatibleState)
		{
			if (val.stateA.name == incompState || val.stateB.name == incompState)
			{
				val.incompatible = true;
			}
		}
	}

	//TODO:
	//find all incompatible
}
