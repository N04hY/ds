#include "Manager.h"

void Manager::run(const char* cmd_txt)
{
	fin.open(cmd_txt);
	flog.open("log.txt", ios::app);
	if(!fin)
	{
		flog << "File Open Error" << endl;
		return;
	}
	while (!fin.eof())
	{
		getline(fin, cmd);
		if (cmd == "LOAD")
			LOAD();
		else if (cmd == "BTLOAD")
			BTLOAD();
		else if (cmd == "PRINT_ITEMLIST")
			PRINT_ITEMLIST();
		else if (cmd == "PRINT_FPTREE")
			PRINT_FPTREE();
		else if (cmd.find("PRINT_BPTREE") != string::npos) {
			option.clear();
			stringstream sso(cmd);
			while (getline(sso, cmd, '\t')) {
				option.push_back(cmd);
			}
			PRINT_BPTREE(option[1], stoi(option[2]));
		}
		else if (cmd.find("PRINT_CONFIDENCE") != string::npos) {
			option.clear();
			stringstream sso2(cmd);
			while (getline(sso2, cmd, '\t')) {
				option.push_back(cmd);
			}
			PRINT_CONFIDENCE(option[1], stof(option[2]));
		}
		else if (cmd.find("PRINT_RANGE") != string::npos) {
			option.clear();
			stringstream sso3(cmd);
			while (getline(sso3, cmd, '\t')) {
				option.push_back(cmd);
			}
			PRINT_RANGE(option[1], stoi(option[2]), stoi(option[3]));
		}
		else if (cmd == "EXIT") {
			break;
		}
	}
	
	return;
}

bool Manager::LOAD()
{
	flog << "==========LOAD=========\n";

	ifstream market("market.txt"); 
	if (!market.is_open() || fpgrowth) { // text file doesn't exist or data already exist
		printErrorCode(100);
		return false;
	}
	fpgrowth = new FPGrowth(&flog, threshold);

	vector<list<pair<int, string>>> itemsets;
	list<pair<int, string>> itemset; 
	pair<int, string> item; 
	
	while (!market.eof()) // extract data from market.txt to itemsets, 
	{
		getline(market, item.second);
		stringstream ssis(item.second);
		while (getline(ssis, item.second, '\t')) {
			fpgrowth->createTable(item.second, 1);
			itemset.push_back(item);
		}
		itemsets.push_back(itemset);
		itemset.clear();
	}	
	fpgrowth->descendingList();
	
	list<pair<int,string>> *idx = fpgrowth->getHeaderTable()->getindexTable();
	list<pair<int,string>>::iterator iter_trv; // iterator to traverse itemsets
	list<pair<int,string>>::iterator iter_find; // iterator to find item in idx
	for (int i = 0; i < itemsets.size(); i++) {
		for (iter_trv = itemsets[i].begin(); iter_trv != itemsets[i].end(); iter_trv++) {
			iter_find = find_if(idx->begin(), idx->end(),
			 [&iter_trv](const pair<int, string>& idx){ return idx.second == iter_trv->second; }); //find item
			iter_trv->first = iter_find->first; // copy frequency from index table to itemsets
		}
		itemsets[i].sort(greater<pair<int, string>>()); // item from market.txt sort by frequency
		
		fpgrowth->createFPtree(fpgrowth->getTree(), fpgrowth->getHeaderTable(), itemsets[i], 1);
	}

	printSuccessCode();
	return true;
}


bool Manager::BTLOAD()
{
	flog << "==========BTLOAD=========\n";

	ifstream result("result.txt"); 
	if (!result.is_open() || bptree) { // text file doesn't exist or data already exist
		printErrorCode(300);
		return false;
	}
	bptree = new BpTree(&flog, bpOrder);

	string temp;
	int key;
	set<string> FPset;
	
	while (!result.eof()) // extract data from market.txt to itemsets, 
	{
		getline(result, temp);
		stringstream ssrs(temp);
		getline(ssrs, temp, '\t');
		if (temp[0] < '0' || temp[0] > '9') { // exception for first word in a line is not a key
			continue;
		}
		key = stoi(temp);
		while (getline(ssrs, temp, '\t')) {
			FPset.insert(temp);			
		}
		bptree->Insert(key, FPset);
		FPset.clear();
	}	
	
	printSuccessCode();
	return true;
}

bool Manager::PRINT_ITEMLIST() { 
	flog << "========PRINT_ITEMLIST=========\n";
	if (!fpgrowth || !fpgrowth->getHeaderTable()) { // header table doesn't exist
		printErrorCode(300);
		return false;
	}
	else { // header table exist
		flog << "Item\tFrequency\n";
		fpgrowth->printList();
		flog << "===============================\n\n";
		return true;
	}
}

bool Manager::PRINT_FPTREE() {
	flog << "========PRINT_FPTREE========\n";
	if (!fpgrowth || !fpgrowth->getTree()) { // fp tree doesn't exist
		printErrorCode(400);
		return false;
	}
	flog << "{StandardItem,Frequency} {Path_Item,Frequency}\n";
	fpgrowth->printTree();
	flog << "===============================\n";
	return true;
}

bool Manager::PRINT_BPTREE(string item, int min_frequency) {
	flog << "========PRINT_BPTREE=========\n";
	if (!bptree || !bptree->getRoot()) {
		printErrorCode(500);
		return false;
	}
	flog << "FrequentPattern\tFrequency\n";
	bptree->printFrequency(item, min_frequency);
	flog << "===============================\n\n";
	return true;
}

bool Manager::PRINT_CONFIDENCE(string item, double rate) {
	flog << "========PRINT_CONFIDENCE========\n";
	if (!bptree || !bptree->getRoot()) {
		printErrorCode(600);
		return false;
	}
	flog << "FrequentPattern\tFrequency\tConfidence\n";
	int frequency = fpgrowth->getHeaderTable()->find_frequency(item);
	bptree->printConfidence(item, (double)frequency, rate);
	flog << "================================\n\n";
	return true;
}

bool Manager::PRINT_RANGE(string item, int start, int end) {
	flog << "========PRINT_RANGE========\n";
	if (!bptree || !bptree->getRoot()) {
		printErrorCode(700);
		return false;
	}
	flog << "FrequentPattern\tFrequency\n";
	bptree->printRange(item, start, end);	
	flog << "===========================\n\n";
	return true;
}

void Manager::printErrorCode(int n) {				//ERROR CODE PRINT
	flog << "ERROR " << n << endl;
	flog << "=======================\n\n";
}

void Manager::printSuccessCode() {//SUCCESS CODE PRINT 
	flog << "Success\n";
	flog << "=======================\n\n";
}

