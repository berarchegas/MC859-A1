#include <bits/stdc++.h>
 
using namespace std;
using namespace chrono;

// timer, pra gente controlar por quanto tempo o codigo roda
struct timer : high_resolution_clock {
	const time_point start;
	timer(): start(now()) {}
	int operator()() {
		return duration_cast<milliseconds>(now() - start).count();
	}
};

// gerador de numeros random
mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

// alfa1 = 5%, alfa2 = 10%
const int alfa1 = 10, alfa2 = 20, segundos = 20, infinity = 100000;
int n, W;
vector<int> w;
vector<vector<int>> a;

// Returns a vector with every item that can be added to the knapsack and their value
vector<pair<int, int>> build_candidates(int solution_weight, set<int> &solution) {
    vector<pair<int, int>> candidates;
    for (int i = 0; i < n; i++) {
        if (!solution.count(i) && w[i] <= W - solution_weight) {
            int value = a[i][i];
            for (int x : solution) value += a[i][x];
            candidates.push_back({i, value});
        }
    }
    return candidates;
}

set<int> random_plus_greedy_construction(int p) {
    set<int> solution;
    int solution_weight = 0;
    vector<pair<int, int>> candidates = build_candidates(solution_weight, solution);
    while (!candidates.empty()) {
        int id;
        if (solution.size() < p) {
            // take an item randomly
            id = rng() % ((int)candidates.size());
        }
        else {
            // take the best item
            id = 0;
            for (int i = 1; i < (int)candidates.size(); i++) {
                if (candidates[i].second > candidates[id].second) {
                    id = i;
                }
            }
        }
        solution.insert(candidates[id].first);
        solution_weight += w[candidates[id].first];
        candidates = build_candidates(solution_weight, solution);
    }
    return solution;
}

// todo 
set<int> sampled_greedy_construction(int p) {
    set<int> solution;
    int solution_weight = 0;
    vector<pair<int, int>> candidates = build_candidates(solution_weight, solution);
    while (!candidates.empty()) {
        if (candidates.size() > p) {
            vector<int> random_value(n);
            for (int &x : random_value) x = rng();
            sort(candidates.begin(), candidates.end(), [&](pair<int, int> a, pair<int, int> b) {
                return random_value[a.first] < random_value[b.first];
            });
            while (candidates.size() > p) candidates.pop_back();
        }
        // take the best item
        int id = 0;
        for (int i = 1; i < (int)candidates.size(); i++) {
            if (candidates[i].second > candidates[id].second) {
                id = i;
            }
        }
        solution.insert(candidates[id].first);
        solution_weight += w[candidates[id].first];
        candidates = build_candidates(solution_weight, solution);
    }
    return solution;
}

// vanilla greedy randomized construction
set<int> standard_greedy_randomized_construction(int alfa) {
    set<int> solution;
    int solution_weight = 0;
    vector<pair<int, int>> candidates = build_candidates(solution_weight, solution);
    while (!candidates.empty()) {
        int max_value = -infinity, min_value = infinity;
        for (auto x : candidates) {
            max_value = max(max_value, x.second);
            min_value = min(min_value, x.second);
        }
        vector<pair<int, int>> RCL;
        for (auto x : candidates) {
            if (x.second >= max_value - alfa * (max_value - min_value) / 100) 
                RCL.push_back(x);
        }
        int id = rng() % ((int)RCL.size());
        solution.insert(RCL[id].first);
        solution_weight += w[RCL[id].first];
        candidates = build_candidates(solution_weight, solution);
    }
    return solution;
}

// Returns the value of a solution
int evaluate(set<int> &solution) {
    int value = 0;
    for (int x : solution) {
        for (int y : solution) {
            if (y >= x)
                value += a[x][y];
        }
    }
    return value;
}

// fills the solution of a local search
void fill_solution(set<int> &solution) {
    int solution_weight = 0;
    for (int x : solution) solution_weight += w[x];
    vector<pair<int, int>> candidates = build_candidates(solution_weight, solution);
    sort(candidates.begin(), candidates.end(), [&] (pair<int, int> a, pair<int, int> b) {
        return 1.0 * a.second / w[a.first] > 1.0 * b.second / w[b.first];
    });
    for (auto x : candidates) {
        if (w[x.first] <= W - solution_weight) {
            solution_weight += w[x.first];
            solution.insert(x.first);
        }
    }
}

// The neighbourhood of a solution is any solution that erases at most one item
// After we erase an item, we can add multiple items

int local_search_best_improving(set<int> &solution) {
    int best_value = evaluate(solution);
    for (int x : solution) {
        set<int> solution_copy = solution;
        solution_copy.erase(x);
        fill_solution(solution_copy);
        best_value = max(best_value, evaluate(solution_copy));
    }
    return best_value;
}

int local_search_first_improving(set<int> &solution) {
    int best_value = evaluate(solution);
    for (int x : solution) {
        set<int> solution_copy = solution;
        solution_copy.erase(x);
        fill_solution(solution_copy);
        int value = evaluate(solution_copy);
        if (value > best_value) {
            return value;
        }
    }
    return best_value;
}

int grasp() {
    timer T;

    // We start with the empty set, with value 0
    int best_solution = 0, cnt = 0;
    while (T() < 1000 * segundos) {

        // aqui voce escolhe entre standard, random_plus_greedy e sample_greedy
        // tambem escolhe o valor de alfa ou p
        set<int> solution = random_plus_greedy_construction(7);

        // aqui voce escolhe first improving ou best improving
        best_solution = max(best_solution, local_search_first_improving(solution));
        cnt++;
    }
    return best_solution;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);
    // number of items and bags capacity
    cin >> n >> W;
    w = vector<int> (n);    
    a = vector<vector<int>> (n, vector<int> (n));
    for (int i = 0; i < n; i++) {
        cin >> w[i];
    }
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n - i; j++) {
            cin >> a[i][j];
            a[j][i] = a[i][j];
        } 
    }
    cout << grasp() << '\n';
    return 0;
}