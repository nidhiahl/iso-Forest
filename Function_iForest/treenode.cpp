#include "treenode.h"
#include <math.h>
#include <random>

#define PI_CONSTANT 3.1415926535


inline std::vector<double> derivate (double* X1, double* time, int dim)
{
    /* return the derivative of the function X1 whose have been measured at times time.*/

    std::vector<double> derivate(dim-1, 0.0);
    for(int i=1; i<dim; i++)
    {
        derivate[i-1] = (X1[i] - X1[i-1]) / (time[i] - time[i-1]);
    }
    return derivate;
}


inline std::vector<double> linspace(double  start, double end, int num)
{
    /* return an vector of 'num' equispaced values between 'start' and 'end'. */ 
    std::vector<double> linspaced;

    if (num == 0) { return linspaced; }
    if (num == 1) 
    {
        linspaced.push_back(start);
        return linspaced;
    }

    double delta = (end - start) / (num - 1);

    for(int i=0; i < num-1; ++i)
    {
        linspaced.push_back(start + delta * i);
    }
    linspaced.push_back(end); 

    return linspaced;
}


inline std::vector<double> dictionary_function (int dim, int dic_number)
{
    std::random_device random_seed_generator;
    std::mt19937_64 RandomEngine(random_seed_generator());

    std::vector<double> dic_function (dim, 0.0);
	std::vector<double> t (dim, 0.0);
	t = linspace(-5,5,dim);

    if(dic_number == 0)  //Brownian Motion
    {
        dic_function[0] = std::normal_distribution<double> (0.0, 1.0) (RandomEngine);
        for(int i=1; i<dim; i++)
        {
            dic_function[i] = std::normal_distribution<double> (0.0, std::sqrt(t[i] - t[i-1]))(RandomEngine);
        }
    }
    else if(dic_number == 1) //Gaussian wavelets with various mean and std
    {
        double sigma;
        double K;

        sigma = std::uniform_real_distribution<double> (0.2, 1)(RandomEngine);
        K = std::uniform_real_distribution<double> (-4.0, 4.0)(RandomEngine);
        for(int i=0; i<dim; i++)
        {
            dic_function[i] = -(2 / (std::pow(PI_CONSTANT, 0.25) * std::sqrt(3 * sigma))) * (std::pow(t[i] - K, 2.0) / std::pow(sigma,2.0) - 1) * (std::exp(-(std::pow(t[i]-K, 2.0) / (2 * std::pow(sigma, 2.0)))));
        }
    }
    else if(dic_number == 2) //Cosine with various frequencies and amplitude
    {
        double ampl=std::uniform_real_distribution<double> (-1, 1)(RandomEngine);
        double freq=std::uniform_real_distribution<double> (0, 10)(RandomEngine);

        for(int i=0; i<dim; i++)
        {
            dic_function[i] = ampl * std::cos(2*PI_CONSTANT*freq*t[i]);
        }
    }
    else 
    {
        cout<<"this dictionary is not defined"<<endl;
    }

    return dic_function;
}


inline double inner_product(double* X1, double* X2, double* time, double alpha, int dim)
{
    /* Return the innerproduct between X1 and X2 as a convex combination
	 *between L2 innerproduct and the L2 innerproduct of derivatives. 
	 *
	 * 'alpha=1' corresponds to L2 innerproduct
	 * 'alpha=0.5' corresponds to the Sobolev innerproduct
	 * 'alpha=0' corresponds to the derivative innerproduct. 
	*/
    double result = 0.0;

    if(alpha == 1)
    {
        std::vector<double> prod(dim, 0.0);
        
        prod[0] = X1[0] * X2[0];
        for(int i=1; i<dim; i++)
        {
            prod[i] = X1[i] * X2[i];
            result += (time[i] - time[i-1]) * (prod[i] + prod[i-1]) / 2.0; 
        }
    }
    else if(alpha == 0)
    {
        std::vector<double> prod_derivate(dim-1, 0.0);
        std::vector<double> X1_derivate(dim-1, 0.0);
        std::vector<double> X2_derivate(dim-1, 0.0);

        X1_derivate = derivate(X1, time, dim);
		X2_derivate = derivate(X2, time, dim);		
		
        prod_derivate[0] = X1_derivate[0] * X2_derivate[0];
        for(int i=1; i<dim-1; i++)
        {
            prod_derivate[i] = X1_derivate[i] * X2_derivate[i];
            result += (time[i] - time[i-1]) * (prod_derivate[i] + prod_derivate[i-1]) / 2.0;
        }
    }
    else
    {

        std::vector<double> prod_derivate(dim-1, 0.0);
        std::vector<double> X1_derivate(dim-1, 0.0);
        std::vector<double> X2_derivate(dim-1, 0.0);
        std::vector<double> prod(dim, 0.0);
        std::vector<double> step_time (dim-1, 0.0);
        double inner = 0.0;
        double inner_derivate = 0.0;
        double norm_X1 = 0.0;
		double norm_X2 = 0.0;
		double norm_X1_derivate = 0.0;
		double norm_X2_derivate = 0.0;

        prod[0] = X1[0] * X2[0];
        for(int i=1; i<dim; i++)
        {
            prod[i] = X1[i] * X2[i];
            step_time[i-1] = time[i] - time[i-1];
            inner += step_time[i-1] * (prod[i] + prod[i-1]) / 2.0;
            norm_X1 += step_time[i-1] * (std::pow (X1[i], 2.0) + std::pow (X1[i-1], 2.0)) / 2.0;
			norm_X2 += step_time[i-1] * (std::pow (X2[i], 2.0) + std::pow (X2[i-1], 2.0)) / 2.0;
        }
  
        X1_derivate = derivate(X1, time, dim);
		X2_derivate = derivate(X2, time, dim);

		prod_derivate[0] = X1_derivate[0] * X2_derivate[0];
        for(int i=1; i<dim-1; i++)
        {
            prod_derivate[i] = X1_derivate[i] * X2_derivate[i];
            inner_derivate += step_time[i-1]  * (prod_derivate[i] + prod_derivate[i-1]) / 2.0;
			norm_X1_derivate += step_time[i-1]  * (std::pow (X1_derivate[i], 2.0) + std::pow (X1_derivate[i-1], 2.0)) / 2.0;
			norm_X2_derivate += step_time[i-1]  * (std::pow (X2_derivate[i], 2.0) + std::pow (X2_derivate[i-1], 2.0)) / 2.0;
        }

        result = alpha * inner / (std::sqrt (norm_X1) * std::sqrt (norm_X2)) + (1 - alpha) * inner_derivate / (std::sqrt (norm_X1_derivate) * std::sqrt (norm_X2_derivate));
    }
  
    return result;
}


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


double treenode::splitInfoSelection(const data &dataObject, double alpha, int dic_number, const data &timeDataObject, int random_seed){
   
    std::mt19937_64 RandomEngine (random_seed);
    innerprod.resize(dataPointIndices.size(), 0.0);
    dic_vector = dictionary_function(dataObject.getnumAttributes(), dic_number);
    // for(int i=0;i<dic_vector.size();i++)
    // {
    //     cout<<dic_vector[i]<<" ";
    // }
    // cout<<endl;
    for(int i=0; i<dataPointIndices.size(); i++)
    {
        innerprod[i] = inner_product(dataObject.dataVector[i]->attributes, &dic_vector[0], timeDataObject.dataVector[0]->attributes, alpha, dataObject.getnumAttributes());
    }

    minimumVal = *std::min_element(std::begin(innerprod), std::end(innerprod));
    maximumVal = *std::max_element(std::begin(innerprod), std::end(innerprod));
 
    return std::uniform_real_distribution<double> (minimumVal, maximumVal)(RandomEngine);
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

	
