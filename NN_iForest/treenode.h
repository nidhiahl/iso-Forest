#ifndef TREENODE_H
#define TREENODE_H
# include "data.h"
#include <vector>
#include <boost/serialization/list.hpp>


class treenode
{
    public:

        int nodeId;
        int nextId;
        treenode *nearestNeighbour;
        vector<double> center;
		long double radius;
        bool isLeaf;
        
		
        treenode();
        treenode(int);
        virtual ~treenode();
        double splitInfoSelection(const data &);
        void createLeftChild();
		void createRightChild();

    protected:

    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            ar & nodeId;
            ar & nextId;
            ar & radius;
            ar & center;
            ar & isLeaf;   
        }
};

#endif // TREENODE_H
