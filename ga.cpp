#include <bits/stdc++.h>

using namespace std;
using namespace chrono;

// Parameters of the Genetic Algorithm
// You only need to change stuff here to test
// the probabilities are divided by 10k
int numSeconds = 1000, populationSize = 300, mutationProbability = 10, crossoverPoints = 2;
bool diversityMaintenance = false;
bool uniformCrossover = true; int bernoulliProbability = 400;

struct Solution
{
    vector<bool> x;
    int value, weight;

    static int N, maxWeight;

    static vector<int> w;
    static vector< vector<int> > a;

    Solution() : x(N, false), value(0), weight(0) {}

    Solution operator ^ (int item) const
    {
        Solution newSolution = *this;

        for(int i = 0 ; i < N ; i++)
            newSolution.value -= a[i][item]*x[i]*x[item];

        newSolution.weight -= w[item]*x[item];

        newSolution.x[item] = !newSolution.x[item];

        for(int i = 0 ; i < N ; i++)
            newSolution.value += a[i][item]*newSolution.x[i]*newSolution.x[item];

        newSolution.weight += w[item]*newSolution.x[item];
        return newSolution;
    }

    bool operator == (Solution other) const { return x == other.x; }
    bool operator < (Solution other) const { return value < other.value; }

    bool isValid() { return weight <= maxWeight; }
};

// random number generator
mt19937 rng((int) chrono::steady_clock::now().time_since_epoch().count());

// Struct static variables
int Solution::N = 0, Solution::maxWeight = 0;
vector<int> Solution::w = vector<int>();
vector< vector<int> > Solution::a = vector< vector<int> >();

// Literally just sets every bit with a 50/50 chance
Solution generateRandomSolution()
{
    Solution curSolution;
    for (int i = 0; i < Solution::N; i++)
    {
        if( rng()%2 )
            continue;

        curSolution = curSolution^i;
    }
    return curSolution;
}

// Given 2x Children, it randomly sorts the children 
// and then selects the best one out of every two consecutive children
void select_best(vector<Solution> &population, vector<Solution> &newPopulation)
{
    vector<int> id(newPopulation.size()), randomValues(newPopulation.size());
    iota(id.begin(), id.end(), 0);
    for (int i = 0; i < newPopulation.size(); i++) {
        randomValues[i] = rng();
    }
    sort(id.begin(), id.end(), [&](int i, int j) {
        return randomValues[i] > randomValues[j];
    });
    for (int i = 0; i < populationSize; i++) {
        int id1 = id[2 * i];
        int id2 = id[2 * i + 1];
        if (newPopulation[id1].isValid() == newPopulation[id2].isValid()) {
            if (newPopulation[id1].value > newPopulation[id2].value) {
                population[i] = newPopulation[id1];
            } else {
                population[i] = newPopulation[id2];
            }
        }
        else {
            if (newPopulation[id1].isValid()) {
                population[i] = newPopulation[id1];
            } else {
                population[i] = newPopulation[id2];
            }
        }
    }
}

// Applies crossover and mutation to the children
void generate_children(vector<Solution> &newPopulation, Solution child1, Solution child2, int mutationProbability, int crossoverPoints, bool diversityMaintenance, bool uniformCrossover, int bernoulliProbability) {
    
    // Crossover
    vector<bool> crossover(Solution::N);

    if (uniformCrossover)
    {
        for (int i = 0; i < Solution::N; i++)
            crossover[i] = (rng() % 10000 < bernoulliProbability);    
    }
    else if (diversityMaintenance)
    {
        vector<bool> childXor(Solution::N);
        for (int i = 0; i < Solution::N; i++)
            childXor[i] = child1.x[i] ^ child2.x[i];
        int firstOne = 0, lastOne = 0;
        for (int i = 0; i < Solution::N; i++) 
        {
            if (childXor[i])
            {
                firstOne = i;
                break;
            }
        }
        for (int i = Solution::N - 1; i >= 0; i--) 
        {
            if (childXor[i])
            {
                lastOne = i;
                break;
            }
        }
        if (firstOne != lastOne) {
            vector<int> points(crossoverPoints);
            for (int i = 0; i < crossoverPoints; i++)
                points[i] = rng() % (lastOne - firstOne) + firstOne + 1;

            sort(points.begin(), points.end()); 
            for (int i = 0, id = 0; i < Solution::N; i++)
            {
                if (id < crossoverPoints && i == points[id])
                {
                    id++;
                }
                crossover[i] = id % 2;
            }
        }
    }
    else
    {
        vector<int> points(crossoverPoints);
        for (int i = 0; i < crossoverPoints; i++)
            points[i] = rng() % Solution::N;
        sort(points.begin(), points.end()); 
        for (int i = 0, id = 0; i < Solution::N; i++)
        {
            if (id < crossoverPoints && i == points[id])
            {
                id++;
            }
            crossover[i] = id % 2;
        }
    }

    for (int i = 0; i < Solution::N; i++)
    {
        if (crossover[i])
        {
            if (child1.x[i] != child2.x[i]) {
                child1 = child1 ^ i;
                child2 = child2 ^ i;
            }
        }
    }

    // Mutation
    for (int i = 0; i < Solution::N; i++)
    {
        if (rng() % 10000 < mutationProbability)
        {
            child1 = child1 ^ i;
        }
        if (rng() % 10000 < mutationProbability)
        {
            child2 = child2 ^ i;
        }
    }

    newPopulation.push_back(child1);
    newPopulation.push_back(child2);
}

Solution genetic_algorithm(int numSeconds, int populationSize, int mutationProbability, int crossoverPoints, bool diversityMaintenance, bool uniformCrossover, int bernoulliProbability)
{
    Solution answer;
    auto start = high_resolution_clock::now();
    vector<Solution> population(populationSize);
    for (int i = 0; i < populationSize; i++)
        population[i] = generateRandomSolution();

    int gen = 0;
    while( duration_cast<seconds>(high_resolution_clock::now() - start).count() < numSeconds )
    {
        gen++;
        if (gen % 10 == 0)
            cout << "Generation " << gen << endl;
        vector<Solution> newPopulation;

        while (newPopulation.size() < 4 * populationSize) {
            int parent1 = rng() % populationSize;
            int parent2 = parent1;
            while (parent2 == parent1) {
                parent2 = rng() % populationSize;
            }
            generate_children(newPopulation, population[parent1], population[parent2], mutationProbability, crossoverPoints, diversityMaintenance, uniformCrossover, bernoulliProbability);
        }

        vector<Solution> intermediate(2 * populationSize);
        select_best(intermediate, newPopulation);  
        select_best(population, intermediate);  

        for (int i = 0; i < populationSize; i++)
        {
            if( answer < population[i] && population[i].isValid() )
            {
                auto elapsedTime = duration_cast<seconds>(high_resolution_clock::now() - start).count();

                cout << "Found new answer after " << elapsedTime << " seconds with value " << population[i].value << endl;
            
                answer = population[i];
            }
        }
    }

    return answer;
}

int main()
{
    int N, W;
    cin >> N >> W;
    
    vector<int> w(N);
    vector< vector<int> > a(N, vector<int>(N, 0));

    for(int i = 0 ; i < N ; i++)
        cin >> w[i];

    for(int i = 0 ; i < N ; i++)
        for(int j = i ; j < N ; j++)
            cin >> a[i][j];

    for(int i = 0 ; i < N ; i++)
        for(int j = 0 ; j < i ; j++)
            a[i][j] = a[j][i];

    Solution::a = a; Solution::w = w;
    Solution::N = N; Solution::maxWeight = W;

    Solution ans = genetic_algorithm(numSeconds, populationSize, mutationProbability, crossoverPoints, diversityMaintenance, uniformCrossover, bernoulliProbability);
    cout << ans.value << '\n';
    for (int x : ans.x) cout << x << ' ';
    cout << '\n';
}                  