#include "maths.h"

Maths::Maths()
{
}

float Maths::Median(vector<float> const& v)
{
    bool isEven = !(v.size() % 2);
    size_t n    = v.size() / 2;

    vector<size_t> vi(v.size());
    iota(vi.begin(), vi.end(), 0);

    partial_sort(begin(vi), vi.begin() + n + 1, end(vi),
        [&](size_t lhs, size_t rhs) { return v[lhs] < v[rhs]; });

    return isEven ? 0.5 * (v[vi.at(n-1)] + v[vi.at(n)]) : v[vi.at(n)];
}

float Maths::Mean(vector<float> a){
    float mean = 0;
    int n = a.size();
    for(int i = 0; i < n; i++){
        mean += a[i];
    }
    mean = mean / n;
    return mean;
}

float Maths::Variance(vector<float> a){
    float var = 0;
    int n = a.size();
    float mean = Maths::Mean(a);
    for(int i = 0; i < n; i++){
        var += (a[i] - mean)*(a[i] - mean);
    }
    var = var / n;
    return var;
}

float Maths::StandardDeviation(vector<float> a){
    float SD = 0;
    int n = a.size();
    SD = sqrt(static_cast<double>(Maths::Variance(a)));
    return SD;
}
