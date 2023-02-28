/// PROJECT IDENTIFIER: 40FB54C86566B9DDEAB902CC80E8CE85C1C62AAD
#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <getopt.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <deque>
#include <stack>
#include <queue>
#include <utility>
#include <vector>
using namespace std;

struct coord {
	char s; //map symbol, default '/'
	char d; //direction, default 'D'
};

struct cor {
	uint32_t x;
	uint32_t y;
};

class hunt {
public:
	hunt(uint32_t r, char f) :row(r), format(f) {
		hunt_order = "NESW";
		water = 0;
		land = 0;
		went_ashore = 0;
		trea_found = false;
		verbose = false;
		stats = false;
		cap_con = false;
		fm_con = false;
	}

	~hunt() {}

	/***********VARIABLES***********/
	vector<vector<coord> > map;
	vector<cor> bt_con; // store path when back tracing
	string hunt_order;
	cor sail_lct; //sail location
	cor search_lct; //search location
	cor starting_pt;
	uint32_t row;
	uint32_t went_ashore;
	uint32_t water;  //total water location investigated
	uint32_t land; ////total land location investigated
	char p_format;
	char format; //format of the map
	bool verbose; //true if user want to print verbose
	bool stats; // true if user wants to print stat
	bool cap_con; //true if user specify queue for captain
	bool fm_con; //true if user specify stack for captain
	bool trea_found; //true if treasure is found

	/************READ MAP**************/

	void map_init(istream& is) {
		map.resize(int(row));
		for (uint32_t i = 0; i < row; i++) {
			map[i].resize(int(row));
		}
		switch (format) {
		case 'M':
			for (uint32_t i = 0; i < row; i++) {
				for (uint32_t j = 0; j < row; j++) {
					map[i][j] = { '/','D' };
					cin >> map[i][j].s;
				}
			}
			break;
		case 'L':
			for (uint32_t i = 0; i < row; i++) {
				for (uint32_t j = 0; j < row; j++) {
					map[i][j] = { '.','D' };
				}
			}
			uint32_t r, c;
			char symbol;
			while (is >> r >> c >> symbol) {
				map[r][c].s = symbol;
			}
			break;
		}
	}

	/************GET MODE***************/

	void getMode(int argc, char* argv[]) {
		opterr = false; // Let us handle all error output for command line options
		int choice;
		int option_index = 0;
		uint32_t p_num = 0;
		uint32_t a = 0;
		uint32_t b = 0;
		uint32_t c = 0;
		uint32_t d = 0;
		string mode;
		option long_options[] = {
			{"help", no_argument, nullptr, 'h' },
			{"captain", required_argument, nullptr, 'c' },
			{"first-mate", required_argument, nullptr, 'f' },
			{"hunt-order", required_argument, nullptr, 'o' },
			{"verbose", no_argument, nullptr, 'v'},
			{"stats", no_argument, nullptr, 's' },
			{"show-path", required_argument, nullptr, 'p' },
			{ nullptr, 0, nullptr, '\0' }
		};

		while ((choice = getopt_long(argc, argv, "c:f:o:p:vsh", long_options, &option_index)) != -1) {
			switch (choice) {
			case 'h':
				printHelp(argv);
				exit(0);

			case 'c':
				mode = optarg;
				if (mode != "STACK" && mode != "QUEUE") {
					cerr << "Invalid argument to --captain" << endl;
					cerr << "  Invalid argument is: " << optarg << endl;
					exit(1);
				}
				if (mode == "QUEUE") {
					cap_con = true;
				}
				break;

			case 'f':
				mode = optarg;
				if (mode != "STACK" && mode != "QUEUE") {
					cerr << "Invalid argument to --first-mate" << endl;
					cerr << "  Invalid argument is: " << optarg << endl;
					exit(1);
				}
				if (mode == "STACK") {
					fm_con = true;
				}
				break;

			case 'o':
				hunt_order = optarg;
				if (hunt_order.length() != 4) {
					cerr << "Invalid argument to --hunt-order" << endl;
					cerr << "  Invalid argument is: " << optarg << endl;
					exit(1);
				}
				for (int i = 0; i < 4; i++) {
					if (hunt_order.at(i) != 'N' && hunt_order.at(i) != 'E' &&
						hunt_order.at(i) != 'S' && hunt_order.at(i) != 'W') {
						cerr << "Invalid argument to --hunt-order" << endl;
						cerr << "Argument must be N/E/S/W " << optarg << endl;
						exit(1);
					}
				}
				for (int i = 0; i < 4; i++) {
					switch (hunt_order.at(i)) {
					case 'N':
						a++;
						break;
					case 'E':
						b++;
						break;
					case 'S':
						c++;
						break;
					case 'W':
						d++;
						break;
					}
				}
				if (a != 1 || b != 1 || c != 1 || d != 1) {
					cerr << "Invalid argument to --hunt-order" << endl;
					cerr << "  Invalid argument is: " << optarg << endl;
					exit(1);
				}
				break;

			case 'v':
				verbose = true;
				break;

			case 's':
				stats = true;
				break;

			case 'p':
				mode = optarg;
				p_num++;
				if (p_num > 1) {
					cerr << "Invalid argument to --show-path" << endl;
					cerr << "Invalid argument is: " << optarg << endl;
					exit(1);
				}
				if (mode.length() != 1) {
					cerr << "Invalid argument to --show-path" << endl;
					cerr << "  Invalid argument is: " << optarg << endl;
					exit(1);
				}
				if (mode != "M" && mode != "L") {
					cerr << "Invalid argument to --show-path" << endl;
					cerr << "Invalid argument is: " << optarg << endl;
					exit(1);
				}
				p_format = mode.at(0);
				break;

			default:
				cerr << "Error: invalid option" << endl;
				exit(1);
			} // switch
		} // while
	} // getMode()

	/************HELPER FUNC***************/

	void find_starting_point() {
		for (uint32_t i = 0; i < row; i++) {
			for (uint32_t j = 0; j < row; j++) {
				if (map[i][j].s == '@') {
					map[i][j].d = 'P';
					starting_pt = { i,j };
					return;
				}
			}
		}
	}

	bool check_out_of_bound(uint32_t x, uint32_t y) {
		return (int)x >= 0 && x < row && (int)y >= 0 && y < row ? false : true;
	}

	void check_treasure(cor& lct) {
		if (map[lct.x][lct.y].s == '$') {
			trea_found = true;
		}
	}

	void printPoint(cor& pt) {
		cout << pt.x << "," << pt.y;
	}

	void back_trace(cor point) {
		while (map[point.x][point.y].d != 'P') {
			if (map[point.x][point.y].d == 'N') {
				cor p_1 = { point.x + 1,point.y };
				bt_con.push_back(p_1);
				point = { point.x + 1,point.y };
			}
			else if (map[point.x][point.y].d == 'E') {
				cor p_2 = { point.x,point.y - 1 };
				bt_con.push_back(p_2);
				point = { point.x,point.y - 1 };
			}
			else if (map[point.x][point.y].d == 'S') {
				cor p_3 = { point.x - 1,point.y };
				bt_con.push_back(p_3);
				point = { point.x - 1,point.y };
			}
			else if (map[point.x][point.y].d == 'W') {
				cor p_4 = { point.x ,point.y + 1 };
				bt_con.push_back(p_4);
				point = { point.x,point.y + 1 };
			}
		}
	}

	/************SAIL CON***************/

	void pop_sail_con(deque<cor>& sail_con) {
		!cap_con ? sail_con.pop_back() : sail_con.pop_front();
	}

	void update_sail_lct(deque<cor>& sail_con) {
		if (!sail_con.empty()) {
			if (!cap_con) {
				sail_lct = sail_con.back();
			}
			else {
				sail_lct = sail_con.front();
			}
		}
	}

	/************SEARCH CON***************/

	void pop_search_con(deque<cor>& search_con) {
		!fm_con ? search_con.pop_front() : search_con.pop_back();
	}

	void update_search_lct(deque<cor>& search_con) {
		if (!search_con.empty()) {
			if (!fm_con) {
				search_lct = search_con.front();
			}
			else {
				search_lct = search_con.back();
			}
			land++;
		}
	}

	/**********CAPTAIN INVEST***************/
	void cp_helper(deque<cor>& sail_con, uint32_t x, uint32_t y, char direction) {
		if (map[x][y].s == '.') {
			cor p = { x,y };
			sail_con.push_back(p);
			map[x][y].d = direction;
		}
		else if (map[x][y].s == 'o' || map[x][y].s == '$') {
			cor p = { x,y };
			went_ashore++;
			map[x][y].d = direction;
			search_lct = p;
			land++;
			if (verbose) {
				cout << "Went ashore at: ";
				printPoint(search_lct);
			}
			First_mate_search();
		}
	}

	void check_captain_north(deque<cor>& sail_con, cor& lct) {
		if (!check_out_of_bound(lct.x - 1, lct.y) && map[lct.x - 1][lct.y].d == 'D') {
			cp_helper(sail_con, lct.x - 1, lct.y, 'N');
		}
	}

	void check_captain_east(deque<cor>& sail_con, cor& lct) {
		if (!check_out_of_bound(lct.x, lct.y + 1) && map[lct.x][lct.y + 1].d == 'D') {
			cp_helper(sail_con, lct.x, lct.y + 1, 'E');
		}
	}

	void check_captain_south(deque<cor>& sail_con, cor& lct) {
		if (!check_out_of_bound(lct.x + 1, lct.y) && map[lct.x + 1][lct.y].d == 'D') {
			cp_helper(sail_con, lct.x + 1, lct.y, 'S');
		}
	}

	void check_captain_west(deque<cor>& sail_con, cor& lct) {
		if (!check_out_of_bound(lct.x, lct.y - 1) && map[lct.x][lct.y - 1].d == 'D') {
			cp_helper(sail_con, lct.x, lct.y - 1, 'W');
		}
	}

	/**************FM INVEST***************/
	void fm_helper(deque<cor>& search_con, uint32_t x, uint32_t y, char direction) {
		if (map[x][y].s == 'o') {
			cor p = { x,y };
			search_con.push_back(p);
			map[x][y].d = direction;
		}
		else if (map[x][y].s == '$') {
			cor p = { x,y };
			search_con.push_back(p);
			map[x][y].d = direction;
			search_lct = p;
			trea_found = true;
			land++;
		}
	}

	void check_fm_north(deque<cor>& search_con, cor& lct) {
		if (!check_out_of_bound(lct.x - 1, lct.y) && map[lct.x - 1][lct.y].d == 'D') {
			fm_helper(search_con, lct.x - 1, lct.y, 'N');
		}
	}

	void check_fm_east(deque<cor>& search_con, cor& lct) {
		if (!check_out_of_bound(lct.x, lct.y + 1) && map[lct.x][lct.y + 1].d == 'D') {
			fm_helper(search_con, lct.x, lct.y + 1, 'E');
		}
	}

	void check_fm_south(deque<cor>& search_con, cor& lct) {
		if (!check_out_of_bound(lct.x + 1, lct.y) && map[lct.x + 1][lct.y].d == 'D') {
			fm_helper(search_con, lct.x + 1, lct.y, 'S');
		}
	}

	void check_fm_west(deque<cor>& search_con, cor& lct) {
		if (!check_out_of_bound(lct.x, lct.y - 1) && map[lct.x][lct.y - 1].d == 'D') {
			fm_helper(search_con, lct.x, lct.y - 1, 'W');
		}
	}
	/****************SEARCH***************/

	void Captain_search() {
		deque<cor> sail_con;
		sail_con.push_back(starting_pt);
		if (hunt_order == "NESW") {
			while (!sail_con.empty() && !trea_found) {
				pop_sail_con(sail_con);
				check_captain_north(sail_con, sail_lct);
				if (!trea_found) {
					check_captain_east(sail_con, sail_lct);
				}
				if (!trea_found) {
					check_captain_south(sail_con, sail_lct);
				}
				if (!trea_found) {
					check_captain_west(sail_con, sail_lct);
				}
				if (!trea_found) {
					update_sail_lct(sail_con);
				}
				water++;
			}
		}
		else {
			while (!sail_con.empty() && !trea_found) {
				pop_sail_con(sail_con);
				for (int i = 0; i < 4; i++) {
					switch (hunt_order.at(i)) {
					case 'N':
						if (!trea_found) {
							check_captain_north(sail_con, sail_lct);
						}
						break;
					case 'E':
						if (!trea_found) {
							check_captain_east(sail_con, sail_lct);
						}
						break;
					case 'S':
						if (!trea_found) {
							check_captain_south(sail_con, sail_lct);
						}
						break;
					case 'W':
						if (!trea_found) {
							check_captain_west(sail_con, sail_lct);
						}
						break;
					}
				}
				if (!trea_found) {
					update_sail_lct(sail_con);
				}
				water++;
			}
		}
	}

	void First_mate_search() {
		deque<cor> search_con;
		search_con.push_back(search_lct);
		if (hunt_order == "NESW") {
			while (!search_con.empty() && !trea_found) {
				check_treasure(search_lct);
				pop_search_con(search_con);
				check_fm_north(search_con, search_lct);
				if (!trea_found) {
					check_fm_east(search_con, search_lct);
				}
				if (!trea_found) {
					check_fm_south(search_con, search_lct);
				}
				if (!trea_found) {
					check_fm_west(search_con, search_lct);
				}
				if (!trea_found) {
					update_search_lct(search_con);
				}
			}
		}
		else {
			while (!search_con.empty() && !trea_found) {
				check_treasure(search_lct);
				pop_search_con(search_con);
				for (int i = 0; i < 4; i++) {
					switch (hunt_order.at(i)) {
					case 'N':
						if (!trea_found) {
							check_fm_north(search_con, search_lct);
						}
						break;
					case 'E':
						if (!trea_found) {
							check_fm_east(search_con, search_lct);
						}
						break;
					case 'S':
						if (!trea_found) {
							check_fm_south(search_con, search_lct);
						}
						break;
					case 'W':
						if (!trea_found) {
							check_fm_west(search_con, search_lct);
						}
						break;
					}
				}
				if (!trea_found) {
					update_search_lct(search_con);
				}
			}
		}
		if (verbose) {
			if (trea_found) {
				cout << "\nSearching island... party found treasure at ";
				printPoint(search_lct);
				cout << ".\n";
			}
			else {
				cout << "\nSearching island... party returned with no treasure.\n";
			}
		}
	}

	/****************DRIVER******************/

	void start() {
		find_starting_point();

		if (verbose) {
			cout << "Treasure hunt started at: ";
			printPoint(starting_pt);
			cout << endl;
		}

		sail_lct = starting_pt;
		Captain_search();

		if (trea_found) {
			bt_con.push_back(search_lct);
			back_trace(search_lct);
		}

		if (verbose) {
			if (!trea_found) {
				cout << "Treasure hunt failed" << endl;
			}
		}

		if (stats) {
			printStats();
		}

		if (p_format == 'M' || p_format == 'L') {
			printPath();
		}

		printResult();
	}

	/************COMMAND FUNC*****************/
	/*
	void printVerbose() {
		cout << "Treasure hunt started at: ";
		printPoint(starting_pt);
		cout << endl;

		if (trea_found) {
			int t = static_cast<int>(went_ashore_con.size()) - 1;
			for (int i = 0; i < t; i++) {
				cout << "Went ashore at: ";
				printPoint(went_ashore_con[i]);
				cout << "\nSearching island... party returned with no treasure." << endl;
			}
			cout << "Went ashore at: ";
			printPoint(went_ashore_con[t]);
			cout << "\nSearching island... party found treasure at ";
			printPoint(search_lct);
			cout << ".\n";
		}
		else {
			int t = static_cast<int>(went_ashore_con.size());
			for (int i = 0; i < t; i++) {
				cout << "Went ashore at: ";
				printPoint(went_ashore_con[i]);
				cout << "\nSearching island... party returned with no treasure." << endl;
			}
			cout << "Treasure hunt failed" << endl;
		}
	}
	*/
	void printStats() {
		cout << "--- STATS ---" << endl;
		cout << "Starting location: ";
		printPoint(starting_pt);
		cout << endl;
		cout << "Water locations investigated: " << water << endl;
		cout << "Land locations investigated: " << land << endl;
		cout << "Went ashore: " << went_ashore << endl;
		if (trea_found) {
			cout << "Path length: " << bt_con.size() - 1 << endl;
			cout << "Treasure location: ";
			printPoint(search_lct);
			cout << endl;
		}
		cout << "--- STATS ---" << endl;
	}

	void updateMap() {
		for (uint32_t i = (uint32_t)(bt_con.size()) - 2; i >= 1; i--) {
			if (map[bt_con[i].x][bt_con[i].y].d == 'N' || map[bt_con[i].x][bt_con[i].y].d == 'S') {
				if (map[bt_con[i - 1].x][bt_con[i - 1].y].d == 'N' ||
					map[bt_con[i - 1].x][bt_con[i - 1].y].d == 'S' ||
					map[bt_con[i - 1].x][bt_con[i - 1].y].d == 'D') {
					map[bt_con[i].x][bt_con[i].y].s = '|';
				}
				else {
					map[bt_con[i].x][bt_con[i].y].s = '+';
				}
			}
			if (map[bt_con[i].x][bt_con[i].y].d == 'W' || map[bt_con[i].x][bt_con[i].y].d == 'E') {
				if (map[bt_con[i - 1].x][bt_con[i - 1].y].d == 'W' ||
					map[bt_con[i - 1].x][bt_con[i - 1].y].d == 'E' ||
					map[bt_con[i - 1].x][bt_con[i - 1].y].d == 'D') {
					map[bt_con[i].x][bt_con[i].y].s = '-';
				}
				else {
					map[bt_con[i].x][bt_con[i].y].s = '+';
				}
			}
		}
		map[search_lct.x][search_lct.y].s = 'X';
	}

	void printPath() {
		if (trea_found) {
			int t = static_cast<int>(bt_con.size()) - 1;
			if (p_format == 'L') {
				cout << "Sail:" << endl;
				for (int i = t; i >= 0; i--) {
					if (map[bt_con[i].x][bt_con[i].y].s == '.' || map[bt_con[i].x][bt_con[i].y].s == '@') {
						printPoint(bt_con[i]);
						cout << endl;
					}
				}
				cout << "Search:" << endl;
				for (int i = t; i >= 0; i--) {
					if (map[bt_con[i].x][bt_con[i].y].s == 'o' || map[bt_con[i].x][bt_con[i].y].s == '$') {
						printPoint(bt_con[i]);
						cout << endl;
					}
				}
			}
			else {
				updateMap();
				uint32_t i = 0;
				uint32_t j = 0;
				while (i < row) {
					while (j < row) {
						cout << map[i][j].s;
						j++;
					}
					cout << endl;
					j = 0;
					i++;
				}
			}
		}
	}

	void printHelp(char* argv[]) {
		cout << "Usage: " << argv[0] << endl;
		cout << "\t[--help] | [-h]\n";
		cout << "\t[--captain QUEUE|STACK] | [-c QUEUE|STACK]\n";
		cout << "\t[--first-mate QUEUE|STACK] | [-f QUEUE|STACK]\n";
		cout << "\t[--hunt-order ORDER] | [-o ORDER]\n";
		cout << "\t[--verbose] | [-v]\n";
		cout << "\t[--stats] | [-s]\n";
		cout << "\t[--show-path M|L] | [-p M|L]\n";
	} // printHelp()

	void printResult() {
		if (!trea_found) {
			cout << "No treasure found after investigating "
				<< water + land << " locations." << endl;
		}
		else {
			cout << "Treasure found at ";
			printPoint(search_lct);
			cout << " with path length " << bt_con.size() - 1 << "." << endl;
		}
	}

private:

};
#endif // HUNT_H