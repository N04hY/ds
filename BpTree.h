#ifndef _BpTree_H_
#define _BpTree_H_

#include "BpTreeDataNode.h"
#include "BpTreeIndexNode.h"
#include <fstream>
#include <iostream>

class BpTree {
private:
	BpTreeNode* root;
	int			order;		// m children
	ofstream* flog;
public:
	BpTree(ofstream *flog, int order = 3) {
		root = NULL;
		this->order = order;
		this->flog = flog;
	}
	~BpTree();
	/* essential */
	bool		Insert(int key, set<string> set);

	bool		excessDataNode(BpTreeNode* pDataNode);
	bool		excessIndexNode(BpTreeNode* pIndexNode);
	void		splitDataNode(BpTreeNode* pDataNode);
	void		splitIndexNode(BpTreeNode* pIndexNode);
	
	BpTreeNode* getRoot() { return root; }
	BpTreeDataNode* searchDataNode(int n);

	void	printFrequentPatterns(set<string> pFrequentPattern);
	bool	printFrequency(string item, int min_frequency);
	bool	printConfidence(string item, double item_frequency, double min_confidence);
	bool 	printRange(string item, int min, int max);

};

#endif
