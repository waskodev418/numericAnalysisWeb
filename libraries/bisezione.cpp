#include <iostream>
#include <string>
#include "exprtk.hpp"       // Your math parser
#include <emscripten/bind.h> // The Emscripten magic

using namespace emscripten;

class Function {
    private:
        double x;
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
                throw std::runtime_error("Invalid function");
            }

            if(!exprtk::derivative(expression, "x", der1)){
                throw std::runtime_error("Invalid first derivate function");
            }

            if(!exprtk::derivative(der1, "x", der2)){
                throw std::runtime_error("Invalid second derivate function");
            }
        }

        /*
            calculate f(x0)
            @param new_x x0   
        */
        double evaluate(double new_x) {
            x = new_x;
            return expression.value();
        }

        /*
            calculate f'(x0)
            @param new_x x0   
        */
        double evaluate_der1(double new_x){
            x = new_x;
            return der1.value();
        }

        /*
            calculate f''(x0)
            @param new_x x0   
        */
        double evaluate_der2(double new_x){
            x = new_x;
            return der2.value();
        }
};

class Interval{
    private:
        double a;
        double b;
        Function &f;

        bool is_not_unique_zero(double a, double b) {  
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
                throw std::runtime_error("zero can be not unique");
            }
        }

        double get_a(){
            return this->a;
        }

        double get_b(){
            return this->b;
        }

        double evaluate_function(double c){
            return this->f.evaluate(c);
        }

        void set_a(double c){
            if(c>a && c<b) a = c;
            else throw std::runtime_error("valore fuori dall'intervallo");
        }

        void set_b(double c){
            if(c>a && c<b) b = c;
            else throw std::runtime_error("valore fuori dall'intervallo");
        }
}

//entry point
double bisezione(std::string expression, double a, double b, int decimals_approx){

    Function f; 
    Interval intervallo;
    try
    {
        f = new Function(expression);
        intervallo = new Interval(f, a, b);
    }
    catch(const std::exception& e)
    {
        throw;
    }
     
    

    return 4.2;
}

// --- THE EMSCRIPTEN BRIDGE ---
EMSCRIPTEN_BINDINGS(bisezione_module) {
    function("bisezione", &bisezione);
}