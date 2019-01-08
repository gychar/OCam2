#ifndef MEDIAN_H
#define MEDIAN_H

#include <cmath>
#include <vector>
#include <numeric>
#include <iterator>
#include <iostream>
#include <algorithm>

using namespace std;

class Maths
{
public:
    Maths(){};
    template <typename T>
    T Median(vector<T> const& v);
    template <typename T>
    T Mean(vector<T> a);
    template <typename T>
    T Variance(vector<T> a);
    template <typename T>
    T StandardDeviation(vector<T> a);
};


template <typename T>
T Maths::Median(vector<T> const& v)
{
    bool isEven = !(v.size() % 2);
    size_t n    = v.size() / 2;

    vector<size_t> vi(v.size());
    iota(vi.begin(), vi.end(), 0);

    partial_sort(begin(vi), vi.begin() + n + 1, end(vi),
        [&](size_t lhs, size_t rhs) { return v[lhs] < v[rhs]; });

    return isEven ? 0.5 * (v[vi.at(n-1)] + v[vi.at(n)]) : v[vi.at(n)];
}

template <typename T>
T Maths::Mean(vector<T> a){
    T mean = 0;
    int n = a.size();
    for(int i = 0; i < n; i++){
        mean += a[i];
    }
    mean = mean / n;
    return mean;
}

template <typename T>
T Maths::Variance(vector<T> a){
    T var = 0;
    int n = a.size();
    T mean = Maths::Mean(a);
    for(int i = 0; i < n; i++){
        var += (a[i] - mean)*(a[i] - mean);
    }
    var = var / n;
    return var;
}

template <typename T>
T Maths::StandardDeviation(vector<T> a){
    T SD = 0;
    int n = a.size();
    SD = sqrt(static_cast<double>(Maths::Variance(a)));
    return SD;
}
#endif // MEDIAN_H
