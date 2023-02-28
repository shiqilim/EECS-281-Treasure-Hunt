// PROJECT IDENTIFIER: 40FB54C86566B9DDEAB902CC80E8CE85C1C62AAD
#include <getopt.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <deque>
#include <stack>
#include <queue>
#include <utility>
#include <vector>
#include "functions.h"
using namespace std;

int main(int argc, char* argv[]) {
	ios_base::sync_with_stdio(false);

	string s;
	uint32_t row;
	char format;
	getline(cin, s);
	while (s.at(0) == '#') {
		getline(cin, s);
	}
	format = s.at(0);
	cin >> row;

	hunt trea_hunt(row, format);
	trea_hunt.getMode(argc, argv);
	trea_hunt.map_init(cin);

	trea_hunt.start();

}