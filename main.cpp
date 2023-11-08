#include <bits/stdc++.h>
using namespace std ;
#define ll long long

const int MAX_GENERATIONS = 50 ;
const int POPULATION_SIZE = 50 ;
const double PC = 0.5 ;
const double PM = 0.01 ;

// problem parameters
int w, n ;
vector<int>weight , value;

vector<string> generatePopulation(){
    vector<string>population ;
    srand(time(0)) ;        // used to minimize generating the same number everytime
    while ((int)population.size() < POPULATION_SIZE){
        // generate valid chromosomes => fitness_i <= w
        string chromosome ;
        int curW = 0 ;
        for(int i=0 ; i<n ; ++i){
            unsigned int prob = rand() % 15 ;
            // if prob < 5 , and we can take that element
            if((prob < 5) && curW+weight[i] <= w){
                chromosome.push_back('1') ;
                curW += weight[i] ;
            }
            else
                chromosome.push_back('0') ;
        }
        population.push_back(chromosome) ;
    }
    return population;
}

// const => can never change the population
// by reference => to save time
vector<pair<double,string>> calcFitness(const vector<string>& population){
    vector<pair<double,string>>fitness(population.size()) ;
    for(int ind = 0 ; ind< population.size() ; ++ind){
        fitness[ind].first = 0 ;
        fitness[ind].second = population[ind] ;
    }

    // for each chromosome in population
    for(int ch = 0 ; ch < (int) population.size() ; ++ch){
        ll totW = 0 ;
        // for each gene in chromosome
        for(int i=0 ; i<(int)population[ch].size() ; ++i){
            if(population[ch][i] == '1'){
                // element is selected
                fitness[ch].first += value[i] ;
                totW += weight[i] ;
            }
        }
        // if 2 solutions with the same profit => the one with greater remW is better
        fitness[ch].first *= 0.8 ;
        fitness[ch].first += (w-totW)*0.2 ;
    }
    return fitness ;
}

vector<double> rankChromosomes(int sz){
    int totSz = (sz*(sz+1))/2 ;
    vector<double>prob(sz) ;
    for(int i=0 ; i<sz ; ++i){
        prob[i] = (1.0*(i+1)) / totSz ;
        if(i)
            prob[i] += prob[i-1] ;
    }
    return prob ;
}

pair<int,int> selectChromosomes(vector<double>& prob){
    double r1 = (double)rand() / (double)RAND_MAX ;     // [0, 1]
    double r2 = (double)rand() / (double)RAND_MAX ;     // [0, 1]

    int chosenInd = lower_bound(prob.begin(),prob.end(),r1) - prob.begin() ;


    int chosenInd2 = lower_bound(prob.begin(),prob.end(),r2) - prob.begin() ;

    return {chosenInd, chosenInd2} ;
}

pair<string, string> crossoverChromosomes(string parent1, string parent2){
    double r1 = (double )rand() / (double )RAND_MAX ;
    string final1, final2 ;
    if(r1 <= PC){
        int crossOverPoint = parent1.size()/2 ;
        final1 ;
        final2 ;
        for(int cr=0 ; cr<crossOverPoint ; ++cr){
            final1.push_back(parent1[cr]) ;
            final2.push_back(parent2[cr]);
        }
        for(int cr=crossOverPoint ; cr<parent1.size() ; ++cr){
            final1.push_back(parent2[cr]) ;
            final2.push_back(parent1[cr]);
        }
    }
    else{
        final1 = parent1 ;
        final2 = parent2 ;
    }
    return {final1, final2};
}

void mutateChromosomes(string& offspring1, string& offspring2){
    for(int ind = 0 ; ind < offspring1.size() ; ++ind){
        // mutation in off1
        double r2 = (double )rand() / (double )RAND_MAX ;
        if(r2 <= PM)
            offspring1[ind] = (offspring1[ind] == '0' ? '1' : '0') ;

        // mutation in off2
        r2 = (double )rand() / (double )RAND_MAX ;
        if(r2 <= PM)
            offspring2[ind] = (offspring2[ind] == '0' ? '1' : '0') ;
    }
}

pair<ll,string> decode(string solution){
    pair<ll,string>ret = {0, solution};
    int curW = 0 ;
    for(int i=0 ; i<n ; ++i){
        if(solution[i] == '1'){
            ret.first += value[i] ;
            curW += weight[i] ;
        }
    }
    if(curW > w)
        ret.first = INT_MIN ;
    return ret ;
}

int main() {
    string line ;
    int tests ;
    ifstream file("knapsack_input.txt");
    // input file:
        // 1. number of test cases
        // for each test:
            // a. knapsack weight
            // b. no. of elements ( n )
            // n lines follow each contain {w_i , v_i}
    if(file.is_open()){
        file >> tests ;
//        tests = 1 ;     // to make sure input is read successfully ( print the first test )
        for(int t=1 ; t<=tests ; ++t){
            // input
            file >> w ;         // knapsack weigh
            file >> n ;         // total number of items
            weight.resize(n) ;
            value.resize(n) ;

            for(int i=0 ; i<n ; ++i)
                file >> weight[i] >> value[i] ;

            // 1. initialize population => binary strings of length n
            vector<string>population = generatePopulation() ;

            string ans("0", n) ;
            for(int gen = 0 ; gen < MAX_GENERATIONS ; gen++){
                // 2. evaluate fitness => value + remW^2
                vector<pair<double,string>>popFitness = calcFitness(population) ;

                // ranking: rank[i] = i+1
                sort(popFitness.begin(), popFitness.end()) ;
                int sz = (int)popFitness.size() ;
                vector<double>prob = rankChromosomes(sz) ;


                // DO THE JOB
                srand(time(0)) ;
                for(int iter=0 ; iter<(sz+1)/2 ; ++iter){
                    // 1. selection
                    pair<int,int>chosen =  selectChromosomes(prob) ;
                    string parent1 = popFitness[chosen.first].second ;
                    string parent2 = popFitness[chosen.second].second ;

                    // 2. crossover
                    pair<string,string>offsprings = crossoverChromosomes(parent1, parent2) ;
                    string offspring1 = offsprings.first ;
                    string offspring2 = offsprings.second ;

                    // 3. mutation
                    mutateChromosomes(offspring1, offspring2) ;

                    // 4. replacement
                    // consider all valid solutions, and take the 'POPULATION_SIZE' best as the new generation
                    pair<ll,string>off1 = decode(offspring1) ;
                    pair<ll,string>off2 = decode(offspring2) ;

                    if(off1.first != INT_MIN)
                        popFitness.emplace_back(off1) ;
                    if(off2.first != INT_MIN)
                        popFitness.emplace_back(off2) ;

                }
                // ELITIST Replacement : remove the worst until size = default population size
                sort(popFitness.begin(), popFitness.end(), greater<>()) ;
                while(popFitness.size() > POPULATION_SIZE)
                    popFitness.pop_back() ;

                ans = popFitness[0].second ;
            }
            // output
            cout << "test: " << t << ", ans is: " << ans << endl ;
        }
        file.close() ;
    }
    else
        cout << "ERR Opening the file" << endl ;

    return 0 ;
}
