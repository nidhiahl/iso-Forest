#ifndef TREENODE_H
#define TREENODE_H
# include "data.h"
#include <vector>
#include <boost/serialization/list.hpp>


class treenode
{
    public:

        std::vector<treenode *> children;
        vector<int> dataPointIndices;
        int splitAttribute;
        double splitValue;
        double minimumVal;
        double maximumVal;
        int nodeSize;   
        int nodeHeight;
		long double pathLengthEst;
        bool isLeaf;
        
		
        treenode();
        virtual ~treenode();
        double splitInfoSelection(const data &);
        void createChild();

    protected:

    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version) {
            ar & splitAttribute;
            ar & splitValue;
			ar & minimumVal;
			ar & maximumVal;
            ar & nodeSize;
            ar & pathLengthEst;
            ar & nodeHeight;
            ar & isLeaf; 
			
            
        }
};

#endif // TREENODE_H
