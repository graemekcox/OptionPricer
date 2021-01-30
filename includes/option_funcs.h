#ifndef __OPTION_FUNCS__
#define __OPTION_FUNSS__

// Standard normal probability density function
double norm_pdf(const double& x) {
    return (1.0 / (pow(2 * M_PI, 0.5))) * exp(-0.5 * x * x);
}
double norm_cdf(const double& x) {
    double k = 1.0 / (1.0 + 0.2316419 * x);
    double k_sum = k * (0.319381530 + k * (-0.356563782 + k * (1.781477937 + k * (-1.821255978 + 1.330274429 * k))));

    if (x >= 0.0) {
        return (1.0 - (1.0 / (pow(2 * M_PI, 0.5))) * exp(-0.5 * x * x) * k_sum);
    }
    else {
        return 1.0 - norm_cdf(-x);
    }
}
// This calculates d_j, for j in {1,2}. This term appears in the closed
// form solution for the European call or put price
double d_j(const int& j, const double& S, const double& K, const double& r, const double& v, const double& T) {
    return (log(S / K) + (r + (pow(-1, j - 1)) * 0.5 * v * v) * T) / (v * (pow(T, 0.5)));
}
double get_bsm_estimate(const double& S, const double& K, const double& r, const double& v, const double& T) {
    /*
        C = Call option price
        S = Current stock (or other underlaying)
        K = strike price
        r = Risk-free interest rate
        t = time to maturity
        N = a normal distribution

        C = S*N(d1) - Ke^(-rt)*N(d2)
        */
    return S * norm_cdf(d_j(1, S, K, r, v, T)) - K * exp(-r * T) * norm_cdf(d_j(2, S, K, r, v, T));
}
void calculate_options_prices() {
    // FIXME This is just an example. Replace with actual option prices
    double S = 100.0; // Underlying price
    double K = 100.0; // Strike price
    double r = 0.05; // Risk free rate(5%)
    double v = 0.2; // volatility of the underlying (20%)
    double T = 1.0; // One year until expiry
    double C = get_bsm_estimate(S, K, r, v, T);

    std::cout << "Estimated call price is $" << C << std::endl;
}

#endif