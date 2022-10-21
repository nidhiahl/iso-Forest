#include "treenode.h"
#include <math.h>
#include <random>


//Standard Deviation Calculator
class StdDevCalc{
    private:

        double count;
        double meanPrev, meanCurr;
        double sPrev, sCurr;
        double varianceCurr;

    public:

        StdDevCalc() {
            count = 0;
        }

        void append(double d) {
            count++;

            if (count == 1) {
                // Set the very first values.
                meanCurr     = d;
                sCurr        = 0;
                varianceCurr = sCurr;
            }
            else {
                // Save the previous values.
                meanPrev     = meanCurr;
                sPrev        = sCurr;

                // Update the current values.
                meanCurr     = meanPrev + (d - meanPrev) / count;
                sCurr        = sPrev    + (d - meanPrev) * (d - meanCurr);
                varianceCurr = sCurr / (count - 1);
            }
        }

    double get_std_dev() {
        return sqrt(varianceCurr);
    }
};

//get K random samples out of N without replacement.
inline std::vector<int> sample_without_replacement(int k, int N)
{
    std::random_device random_seed_generator;
    std::mt19937_64 RandomEngine(random_seed_generator());

    std::unordered_set<int> samples;

    for(int i=N-k+1; i<N+1; i++)
    {
        int v=std::uniform_int_distribution<>(1, i)(RandomEngine);
        if(!samples.insert(v).second) samples.insert(i);
    }

    std::vector<int> result(samples.begin(), samples.end());
    std::shuffle(result.begin(), result.end(), RandomEngine);

    return result;    
}


//inner product of two vectors
inline double inner_product(double* curr, std::vector<double>& normal_vector)
{
    double result=0.0;
    for(int i=0;i<normal_vector.size();i++) result+=((double) curr[i])*normal_vector[i];
    return result;
}

treenode::treenode()
{
    isLeaf = bool(0);
    nodeId = -1;
    parentId = -1;
    lChildId = -1;
    rChildId = -1;
    nodeSize = 0;
    pathLengthEst = 0;
    dataPointIndices.resize(0);
    parentAdd = nullptr;
    lChildAdd = nullptr;
    rChildAdd = nullptr;

}

treenode::treenode(int nId): nodeId(nId)
{
    isLeaf = bool(0);
    parentId = nodeId == 0 ? 0 : (nodeId-1)/2;
    lChildId = -1;
    rChildId = -1;
	nodeHeight = (int)log2(nodeId+1)+1;
    nodeSize = 0;
    pathLengthEst = 0;
    dataPointIndices.resize(0);
    parentAdd = nullptr;
    lChildAdd = nullptr;
    rChildAdd = nullptr;
}

treenode::~treenode()
{
    //dtor
}


void treenode::splitInfoSelection(const data &dataObject, int &numOfAttributes, int &numOfTrialsForHyperplane, int random_seed){
   
    std::mt19937_64 RandomEngine (random_seed);

    std::vector<double> trial_normal_vector;
    std::vector<int> normvect_zero_index;
    std::vector<double> trial_projection_values;
    std::vector<double> trial_sigma_l;
    std::vector<double> trial_sigma_r;
    double trial_sigma_y;
    double trial_splitvalue;
    double trial_sd_gain;

    double sd_gain=0.0;

    for(int trial=0; trial<numOfTrialsForHyperplane; trial++)
    {
        //get the attributes to be considered.
        normvect_zero_index = sample_without_replacement (dataObject.getnumAttributes()-numOfAttributes, dataObject.getnumAttributes());

        //get Normal vector of hyperplane.
        trial_normal_vector.resize(dataObject.getnumAttributes(), 0.0);
        for(int i=0; i<dataObject.getnumAttributes(); i++)
        {
            trial_normal_vector[i]=std::uniform_real_distribution<double> (-1.0, 1.0)(RandomEngine);
        }

        //set normal vector component as 0.0 for attributes not to be considered.
        for(int i=0; i<dataObject.getnumAttributes()-numOfAttributes; i++)
        {
            trial_normal_vector[normvect_zero_index[i]-1] = 0.0;
        }

        //get projection values
        trial_projection_values.resize(dataPointIndices.size());
        for(int i=0; i<dataPointIndices.size(); i++)
        {
            double pdotn = inner_product(dataObject.dataVector[(dataPointIndices[i])]->attributes, trial_normal_vector);
            trial_projection_values[i]=pdotn;
        }

        //Split-Selection Criteria
        trial_sigma_l.resize(dataPointIndices.size()+1);
        trial_sigma_r.resize(dataPointIndices.size()+1);
        sort(trial_projection_values.begin(), trial_projection_values.end());

        trial_sigma_l[0]=0.0;
        StdDevCalc pass1;
        for(int i=1; i<=dataPointIndices.size(); i++)
        {
            pass1.append(trial_projection_values[i-1]);
            trial_sigma_l[i]=pass1.get_std_dev();
        }
        trial_sigma_y = pass1.get_std_dev();

        trial_sigma_r[dataPointIndices.size()]=0.0;
        StdDevCalc pass2;
        for(int i=dataPointIndices.size()-1; i>=0; i--)
        {
            pass2.append(trial_projection_values[i]);
            trial_sigma_r[i]=pass2.get_std_dev();
        }
        
        trial_sd_gain=( trial_sigma_y - (trial_sigma_l[dataPointIndices.size()]+trial_sigma_r[dataPointIndices.size()])/2.0 )/trial_sigma_y;
        trial_splitvalue=1000000.0;
        for(int i=0; i<dataPointIndices.size(); i++)
        {
            double tmp=( trial_sigma_y - (trial_sigma_l[i]+trial_sigma_r[i])/2.0 )/trial_sigma_y;
            if(tmp>trial_sd_gain)
            {
                trial_sd_gain=tmp;
                trial_splitvalue=trial_projection_values[i];
            }
        }

        if(trial_sd_gain>sd_gain)
        {
            sd_gain=trial_sd_gain;
            splitValue=trial_splitvalue;
            normal_vector=trial_normal_vector;
            upperLimit=trial_projection_values[dataPointIndices.size()-1]-trial_projection_values[0];
            lowerLimit=trial_projection_values[0]-trial_projection_values[dataPointIndices.size()-1];
        }

        // cout<< trial_sd_gain<<" ";
    }
    // cout<<endl;
    // cout<<sd_gain<<endl;
    // cout<<endl;
}



void treenode::createLeftChild(){
	lChildAdd = new treenode(2*nodeId+1);
	lChildAdd->parentAdd = this;
	lChildId = lChildAdd->nodeId;
}

void treenode::createRightChild(){
	rChildAdd = new treenode(2*nodeId+2);
	rChildAdd->parentAdd = this;
	rChildId = rChildAdd->nodeId;
}

	
