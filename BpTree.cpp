#include "BpTree.h"

bool BpTree::Insert(int key, set<string> set) {
	
	FrequentPatternNode* fpNode;
	if (root == NULL) { // first routine
		root = new BpTreeDataNode;
		fpNode = new FrequentPatternNode;
		fpNode->InsertList(set);
		root->insertDataMap(key, fpNode);
		return true;
	}

	BpTreeNode* search = searchDataNode(key);
	map<int,FrequentPatternNode*> *mapData = search->getDataMap();
	map<int,FrequentPatternNode*>::iterator iter = mapData->find(key);

	if (iter != mapData->end()) { // find same key
		fpNode = iter->second; // take node with same key
		fpNode->InsertList(set); // put in the same list
		return true;
	}

	// make new node
	fpNode = new FrequentPatternNode;
	fpNode->InsertList(set);
	search->insertDataMap(key, fpNode);

	if (excessDataNode(search))
		splitDataNode(search);

	return true;
}

BpTreeNode* BpTree::searchDataNode(int key) {
	if (typeid(*root) == typeid(BpTreeIndexNode)) {
		BpTreeNode* pCur = root;
		map<int,BpTreeNode*> *mapIndex = pCur->getIndexMap();
		map<int,BpTreeNode*>::iterator iter;
		int mostLeft;

		while (mapIndex) {
			mostLeft = 1;
			for (iter = mapIndex->begin(); iter != mapIndex->end(); iter++) {
				if (key < iter->first) {
					break;
				}
				mostLeft = 0;
			}
			if (mostLeft == 1) {
				pCur = pCur->getMostLeftChild();
				mapIndex = pCur->getIndexMap();
			}
			else {
				iter--;
				pCur = iter->second;
				mapIndex = pCur->getIndexMap();
			}
		}
		map<int,FrequentPatternNode*> *mapData = pCur->getDataMap();
		return pCur;
	}	
	return root;
}


void BpTree::splitDataNode(BpTreeNode* pDataNode) { // setting parent, mostleftchild, next, prev
	int target = (order - 1)/2; // numbering position from 0, not 1
	map<int,FrequentPatternNode*> *mapData = pDataNode->getDataMap();
	map<int,FrequentPatternNode*>::iterator iter = mapData->begin();
	BpTreeDataNode* newDataNode = new BpTreeDataNode;

	while (target-- > 0) {
		newDataNode->insertDataMap(iter->first, iter->second);
		pDataNode->deleteMap(iter->first);
		iter = mapData->begin();
	}

	if (pDataNode->getPrev()) {
		pDataNode->getPrev()->setNext(newDataNode);
		newDataNode->setPrev(pDataNode->getPrev());
	}
	newDataNode->setNext(pDataNode);
	pDataNode->setPrev(newDataNode);

	BpTreeNode* Index = pDataNode->getParent();
	if (!Index) { // need new index node
		Index = new BpTreeIndexNode;
		Index->insertIndexMap(iter->first, pDataNode);
		pDataNode->setParent(Index);
		Index->setMostLeftChild(newDataNode);
		newDataNode->setParent(Index);

		if (pDataNode == root) {	
			root = new BpTreeIndexNode;
			root = Index;		
		}
	}
	else { // use exist index node
		Index->insertIndexMap(iter->first, pDataNode);

		map<int,BpTreeNode*> *mapIndex = Index->getIndexMap();
		map<int,BpTreeNode*>::iterator iter_i = mapIndex->find(iter->first); // find IndexNode that is inserted in upper line
		iter_i--; // point to previous node
		iter_i->second = newDataNode; 
		newDataNode->setParent(Index);


		if (excessIndexNode(Index)) 
			splitIndexNode(Index);
	}
}

void BpTree::splitIndexNode(BpTreeNode* pIndexNode) {
	int target = (order - 1)/2;
	map<int,BpTreeNode*> *mapIndex = pIndexNode->getIndexMap();
	map<int,BpTreeNode*>::iterator iter = mapIndex->begin();
	BpTreeIndexNode* newIndexNode = new BpTreeIndexNode;
	BpTreeNode* IndexMostRightChild;

	newIndexNode->setMostLeftChild(pIndexNode->getMostLeftChild());
	newIndexNode->getMostLeftChild()->setParent(newIndexNode);
	while (target-- > 0) {
		newIndexNode->insertIndexMap(iter->first, iter->second);
		iter->second->setParent(newIndexNode);
		IndexMostRightChild = iter->second;
		pIndexNode->deleteMap(iter->first);
		iter = mapIndex->begin();
	}

	BpTreeNode* Index = pIndexNode->getParent();
	if (!Index) { // need new index node
		Index = new BpTreeIndexNode;

		Index->setMostLeftChild(newIndexNode);
		newIndexNode->setParent(Index);
		
		pIndexNode->setMostLeftChild(iter->second);
		IndexMostRightChild->setNext(iter->second);
		iter->second->setPrev(IndexMostRightChild);

		Index->insertIndexMap(iter->first, pIndexNode);
		pIndexNode->deleteMap(iter->first);
		pIndexNode->setParent(Index);

		if (pIndexNode == root) {
			root = Index;
		}
	}
	else { // use exist index node
		map<int,BpTreeNode*> *mapIndex2 = Index->getIndexMap();
		Index->insertIndexMap(iter->first, pIndexNode);
		map<int,BpTreeNode*>::iterator iter_i = mapIndex2->find(iter->first); // find IndexNode that is inserted in upper line
		pIndexNode->deleteMap(iter->first);
		pIndexNode->setMostLeftChild(iter->second);

		iter_i--;
		iter_i->second = newIndexNode;
		newIndexNode->setParent(Index);
		
		if (excessIndexNode(Index))
			splitIndexNode(Index);
	}

}

bool BpTree::excessDataNode(BpTreeNode* pDataNode) {
	if (pDataNode->getDataMap()->size() > order - 1) return true;//order is equal to the number of elements 
	else return false;
}
bool BpTree::excessIndexNode(BpTreeNode* pIndexNode) {
	if (pIndexNode->getIndexMap()->size() > order - 1)return true;//order is equal to the number of elements 
	else return false;
}

bool BpTree::printConfidence(string item, double item_frequency, double min_confidence)
{
	int min_frequency = item_frequency * min_confidence;
	BpTreeNode* curr = root;	
	while (curr->getMostLeftChild()) {
		curr = curr->getMostLeftChild(); 
	}	// now curr is most left data node

	map<int,FrequentPatternNode*> *dataMap;
	map<int,FrequentPatternNode*>::iterator iter_m; // iterator to traverse mapData
	FrequentPatternNode *fNode;
	multimap<int,set<string>> *fList;
	multimap<int,set<string>>::iterator iter_l; // traverse flist
	set<string> fSet;
	set<string>::iterator iter_s; // traverse fset

	while (curr) {
		dataMap = curr->getDataMap();		
		for (iter_m = dataMap->begin(); iter_m != dataMap->end(); iter_m++) {
			if (iter_m->first < min_frequency) { // frequency is less than min
				continue;
			}
			fNode = iter_m->second;
			fList = fNode->getList();
			for (iter_l = fList->begin(); iter_l != fList->end(); iter_l++) {
				fSet = iter_l->second;
				if (fSet.find(item) == fSet.end()) { // fail to find item
					continue;
				}
				iter_s = fSet.begin();
				*flog << "{" << *iter_s;
				iter_s++;
				for (iter_s; iter_s != fSet.end(); iter_s++) {
					*flog << ", " << *iter_s;
				}
				flog->precision(2);
				*flog << "} " << iter_m->first << " " << iter_m->first/item_frequency << endl;
			}
		}
		curr = curr->getNext();
	}

	return true;
}

bool BpTree::printFrequency(string item, int min_frequency)//print winratio in ascending order
{
	BpTreeNode* curr = root;	
	while (curr->getMostLeftChild()) {
		curr = curr->getMostLeftChild(); 
	}	// now curr is most left data node

	map<int,FrequentPatternNode*> *dataMap;
	map<int,FrequentPatternNode*>::iterator iter_m; // iterator to traverse mapData
	FrequentPatternNode *fNode;
	multimap<int,set<string>> *fList;
	multimap<int,set<string>>::iterator iter_l; // traverse flist
	set<string> fSet;
	set<string>::iterator iter_s; // traverse fset
	while (curr) {
		dataMap = curr->getDataMap();		
		for (iter_m = dataMap->begin(); iter_m != dataMap->end(); iter_m++) {
			if (iter_m->first < min_frequency) { // frequency is less than min
				continue;
			}
			fNode = iter_m->second;
			fList = fNode->getList();
			for (iter_l = fList->begin(); iter_l != fList->end(); iter_l++) {
				fSet = iter_l->second;
				if (fSet.find(item) == fSet.end()) { // fail to find item
					continue;
				}
				iter_s = fSet.begin();
				*flog << "{" << *iter_s;
				iter_s++;
				for (iter_s; iter_s != fSet.end(); iter_s++) {
					*flog << ", " << *iter_s;
				}
				*flog << "} " << iter_m->first << endl;
			}
		}
		curr = curr->getNext();
	}

	return true;
}
bool BpTree::printRange(string item, int min, int max) {
	BpTreeNode* curr = root;	
	while (curr->getMostLeftChild()) {
		curr = curr->getMostLeftChild(); 
	}	// now curr is most left data node

	map<int,FrequentPatternNode*> *dataMap;
	map<int,FrequentPatternNode*>::iterator iter_m; // iterator to traverse mapData
	FrequentPatternNode *fNode;
	multimap<int,set<string>> *fList;
	multimap<int,set<string>>::iterator iter_l; // traverse flist
	set<string> fSet;
	set<string>::iterator iter_s; // traverse fset
	while (curr) {
		dataMap = curr->getDataMap();		
		for (iter_m = dataMap->begin(); iter_m != dataMap->end(); iter_m++) {
			if (iter_m->first != min && min <= max) { // frequency is less than min
				continue;
			}
			fNode = iter_m->second;
			fList = fNode->getList();
			for (iter_l = fList->begin(); iter_l != fList->end(); iter_l++) {
				fSet = iter_l->second;
				if (fSet.find(item) == fSet.end()) { // fail to find item
					continue;
				}
				iter_s = fSet.begin();
				*flog << "{" << *iter_s;
				iter_s++;
				for (iter_s; iter_s != fSet.end(); iter_s++) {
					*flog << ", " << *iter_s;
				}
				*flog << "} " << iter_m->first << endl;
			}
			min++;
		}
		curr = curr->getNext();
	}

	return true;
}

void BpTree::printFrequentPatterns(set<string> pFrequentPattern) {
	*flog << "{";
	set<string> curPattern = pFrequentPattern;
	for (set<string>::iterator it = curPattern.begin(); it != curPattern.end();) {
		string temp = *it++;
		*flog << temp;
		if (it == curPattern.end()) {
			*flog << "} ";
			break;
		}
		*flog << ", ";
	}
}