/**
* **************************************************************
* * ▄█     █▄     ▄████████    ▄████████    ▄█   ▄█▄  ▄██████▄ *
* *███     ███   ███    ███   ███    ███   ███ ▄███▀ ███    ███*
* *███     ███   ███    ███   ███    █▀    ███▐██▀   ███    ███*
* *███     ███   ███    ███   ███         ▄█████▀    ███    ███*
* *███     ███ ▀███████████ ▀███████████ ▀▀█████▄    ███    ███*
* *███     ███   ███    ███          ███   ███▐██▄   ███    ███*
* *███ ▄█▄ ███   ███    ███    ▄█    ███   ███ ▀███▄ ███    ███*
* * ▀███▀███▀    ███    █▀   ▄████████▀    ███   ▀█▀  ▀██████▀ *
* *                                        ▀                   *
* **************************************************************
*/

#include <string>
#include "exprtk.hpp"         // Function parser
#include <emscripten/bind.h>  // WebAssembly tool
#include <stdexcept>
#include <cmath>
#include <algorithm>

using namespace emscripten;

class Function {
    private:
        mutable double x;
        exprtk::symbol_table<double> symbol_table;
        exprtk::expression<double> expression;
        exprtk::expression<double> der1;
        exprtk::expression<double> der2;
        exprtk::parser<double> parser;

    public:
        Function(std::string func_str) : x(0.0) {
            symbol_table.add_variable("x", x);
            symbol_table.add_constants();
            
            expression.register_symbol_table(symbol_table);
            der1.register_symbol_table(symbol_table);
            der2.register_symbol_table(symbol_table);

            if (!parser.compile(func_str, expression)) {
                throw std::runtime_error("funzione non valida :/");
            }

            if(!exprtk::derivative<double>(expression, "x", der1)){
                throw std::runtime_error("derivata prima non valida :/");
            }

            if(!exprtk::derivative<double>(der1, "x", der2)){
                throw std::runtime_error("derivata seconda non valida :/");
            }
        }

        /*
            calculate f(x0)
            @param new_x x0   
        */
        double evaluate(double new_x) const {
            x = new_x;
            double res = expression.value();
            return (!std::isnan(res)) ? res : throw std::domain_error("dominio funzione non valido") ;
        }

        /*
            calculate f'(x0)
            @param new_x x0   
        */
        double evaluate_der1(double new_x) const {
            x = new_x;
            return der1.value();
        }

        /*
            calculate f''(x0)
            @param new_x x0   
        */
        double evaluate_der2(double new_x) const {
            x = new_x;
            return der2.value();
        }
};

class Interval{
    private:
        double a;
        double b;
        const Function &f;

        bool is_unique_zero(double a, double b) const {  
            // I Teorema -> esistenza dello zero
            if (f.evaluate(a) * f.evaluate(b) >= 0) throw std::runtime_error("Teorema di Bolzano non soddisfatto"); 
    
            double epsilon = 1e-9;
            int n = 500;

            bool isUnique = true;

            int step = n;

            //The use of max() rapresents the case in which epsilon < b - a. 
            //Thus is better to just check at the single point a
            double d = std::max((std::abs(b - a) - 2 * epsilon)/step, 0.0);
            double x0 = a + epsilon;

            // II teorema -> monotonicità
            int value = (f.evaluate_der1(x0)>0) ? 1 : -1;
            while(step-- > 0 && isUnique){
                x0+=d;
                if(value*f.evaluate_der1(x0) <= 0) isUnique = false;
            }
            if(isUnique) return true;

            step = n;
            x0 = a + epsilon;
            // III teorema -> concavità
            value = (f.evaluate_der2(x0)>=0) ? 1 : -1;
            while(step-- > 0){
                x0+=d;
                if(value*f.evaluate_der2(x0) < 0) return false;    
            } 
            return true; 
        }

    public:
        Interval(const Function &f_ref, double x1, double x2) : f(f_ref){
            if(x1 > x2) std::swap(x1, x2);

            this->a = x1;
            this->b = x2;

            if(!this->is_unique_zero(a, b)) throw std::runtime_error("Impossibile dimostrare l'unicità dello zero");
        }

        double get_a() const{
            return this->a;
        }

        double get_b() const {
            return this->b;
        }

        const Function& get_function() const {
            return this->f;
        }

        double evaluate_a() const {
            return this->f.evaluate(a);
        }

        double evaluate_b() const {
            return this->f.evaluate(b);
        }

        void set_a(double c){
            if(c>a && c<b) a = c;
            else throw std::runtime_error("valore fuori dall'intervallo");
        }

        void set_b(double c){
            if(c>a && c<b) b = c;
            else throw std::runtime_error("valore fuori dall'intervallo");
        }

        double shrink(double c){
            double f_c = f.evaluate(c);
            (f_c * evaluate_a() > 0) ? set_a(c) : set_b(c);
            return f_c;
        }
};

struct Result{
    double x0;
    int iterations;
};

/**
 * A function to round values to specific decimals
 * @param valore the value to round
 * @param approx the number of decimals to include
 * @example round_value(3.1415, 2) => 3.14
 *  */
double round_value(double value, int approx) {
    double pot = std::pow(10, approx);
    return std::round(value * pot) / pot;
}

/**
 * Bisection method of numeric analysis
 * @param expression a string representing the function
 * @param a an extreme of the interval [a;b]
 * @param b the other extreme of the interval [a;b]
 * @param approx the number of decimals to look for
 * @throw throws runtime_error if the iterations surpass a certain threshold to prevent stalling
 *  */
Result bisezione(std::string expression, double a, double b, int approx){

    Function f(expression);
    Interval inter(f, a, b);
    
    const int MAX_ITER = 100;
    double x;
    int iter = 0;
    do {
        if(iter++ > MAX_ITER) throw std::runtime_error("numero di iterazioni massime superato! :/");
        x = (inter.get_b() + inter.get_a()) / 2;
     
        if(inter.shrink(x) == 0) break;        
    }while(
        round_value(inter.get_a(), approx) 
        != 
        round_value(inter.get_b(), approx)
    );
    return {round_value(x, approx), iter};
}

/**
 * Secants method of numeric analysis
 * @param expression a string representing the function
 * @param a an extreme of the interval [a;b]
 * @param b the other extreme of the interval [a;b]
 * @param approx the number of decimals to look for
 * @throw throws runtime_error if the iterations surpass a certain threshold to prevent stalling
 *  */
Result secanti(std::string expression, double a, double b, int approx){

    Function f(expression);
    Interval inter(f, a, b);

    const int MAX_ITER = 1000;
    int iter = 0;
    double x;
    do{
        if(iter++ > MAX_ITER) throw std::runtime_error("numero di iterazioni massime superato! :/");

        double fa = inter.evaluate_a();
        double fb = inter.evaluate_b();

        x = (fa*inter.get_b() - fb*inter.get_a()) / (fa - fb);

        if(inter.shrink(x) == 0) break;
    }while(
        round_value(inter.get_a(), approx) 
        != 
        round_value(inter.get_b(), approx)
    ); 
    return {round_value(x, approx), iter};
}

// --- THE EMSCRIPTEN BRIDGE ---
EMSCRIPTEN_BINDINGS(bisezione_module) {
    value_object<Result>("ResultSet")
    .field("result", &Result::x0)
    .field("steps", &Result::iterations);

    function("bisezione", &bisezione);
    function("secanti", &secanti);
}