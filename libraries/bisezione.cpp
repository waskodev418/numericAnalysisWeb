#include <iostream>
#include <string>
#include "exprtk.hpp"       // Your math parser
#include <emscripten/bind.h> // The Emscripten magic

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
        Function(std::string func_str) {
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
            return expression.value();
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

        bool is_not_unique_zero(double a, double b) const {  
            if (f.evaluate(a) * f.evaluate(b) > 0) return true; // I teorema
            else { 
                int n = 100;

                int step = n;
                double d = std::abs(b-a)/step;
                double x0 = a;

                // II teorema -> monotonicità
                int value = (f.evaluate_der1(x0)>0) ? 1 : -1;
                while(step-- > 0){
                    x0+=d;
                    if(value*f.evaluate_der1(x0) <= 0) return true;
                }
                
                step = n;
                x0 = a;
                // III teorema -> concavità
                value = (f.evaluate_der2(x0)>0) ? 1 : -1;
                while(step-- > 0){
                    x0+=d;
                    if(value*f.evaluate_der2(x0) <= 0) return true;
                }  
            }
            return false; 
        }

    public:
        Interval(Function &f_ref, double x1, double x2) : f(f_ref){
            if(x1 > x2) std::swap(x1, x2);

            this->a = x1;
            this->b = x2;

            if(this->is_not_unique_zero(a, b)){
                throw std::runtime_error("zero non univoco o non presente :/");
            }
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
 *  */
double bisezione(std::string expression, double a, double b, int approx){

    Function func(expression);
    Interval intervallo(func, a, b);
     
    double mid;
    do {

        mid = (intervallo.get_b() + intervallo.get_a()) / 2;
        double f_mid = func.evaluate(mid);
        if(f_mid == 0) break;
        
        (f_mid * intervallo.evaluate_a() > 0) ? intervallo.set_a(mid) : intervallo.set_b(mid);
    }while(
        round_value(intervallo.get_a(), approx) 
        != 
        round_value(intervallo.get_b(), approx)
    );
    return round_value(mid, approx);
}

// --- THE EMSCRIPTEN BRIDGE ---
EMSCRIPTEN_BINDINGS(bisezione_module) {
    function("bisezione", &bisezione);
}