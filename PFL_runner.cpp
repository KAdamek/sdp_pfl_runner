#include <stdio.h>
#include <stdarg.h>
#include <memory>
#include <iostream>
#include <algorithm>

// Simple function should be trivially callable
int pflSimpleFunctionA(int A, int B, int *status) {
    printf("SimpleFunctionA\n");
    return(A + B);
}

// Simple function acting on the array without temporary memory should be 
// trivially callable.
void pflSimpleFunctionB(float *output, float *input, int A, int B, int *status) {
    printf("SimpleFunctionB\n");
}


// This class will contain configuration parameters for the pflFunction together with a check of validity
// It could be also part of the pflFunctionRunner but then the check must be factored out. Then it would not be an independent entity.
class pflFunctionAConfiguration {
public:
    int a;
    int b;
    double c;

    pflFunctionAConfiguration() {
        a = 0;
        b = 0;
        c = 0;
    }

    bool check() {
        if (a<=0 || b<=0) return false;
        // this could be more advanced with logging of specific problems
        // if(A<=0) {
        //     log(skaloglevel::warning, "A must be non-zero and positive");
        return true;
    }
};

// This class could be derived from virtual class to enforce some structure
class pflFunctionA {
private: // variables
    pflFunctionAConfiguration conf;

    // There might be some internal configuration
    float *workarea;
    int initiated;

public: // methods
    pflFunctionA() {
        initiated = 0;
        workarea = nullptr;
    }

    void planner(pflFunctionAConfiguration conf, int *error_status) {
        if (*error_status != 0) return;
        if (initiated==1) {
            *error_status = 1;
            return;
        }
        printf("Planner for function A\n");

        this->conf = conf;
        workarea = (float *)malloc(conf.a*conf.b*sizeof(float));
        if (workarea == nullptr) {
            *error_status = 1;
        }

        if(*error_status==0) initiated = 1;
    }

    void planner(pflFunctionAConfiguration conf) {
        int error_status = 0;
        planner(conf, &error_status);
        if(error_status!=0) throw new std::exception;
    }

    void run(float *outputA, int *error_status) {
        if (*error_status != 0) return;
        if (initiated==0) {
            *error_status = 1;
            return;
        }
        printf("Running function A\n");

        for (int f = 0; f<conf.a*conf.b; f++) {
            float ftemp = 0;
            workarea[f] = conf.c*f;
            for (int i = 0; i<f; i++) {
                ftemp = ftemp + workarea[i];
            }
            outputA[f] = ftemp;
        }
    }

    void run(float *outputA) {
        int error_status = 0;
        run(outputA, &error_status);
        if(error_status!=0) throw new std::exception;
    }

    ~pflFunctionA() {
        printf("Destructor for function A called.\n");
        free(workarea);
    }
};

// Another complex function which would used ComplexFunction0
class pflFunctionBConfiguration {
public:
    int d;
    int e;

    pflFunctionBConfiguration() {
        d = 0;
        e = 0;
    }

    bool check() {
        if (d<=0 || e<=0) return false;
        return true;
    }
};

// This class could be derived from virtual class to enforce some structure
class pflFunctionB {
private: // variables
    pflFunctionBConfiguration conf;

    // There might be some internal configuration
    float *workarea;
    pflFunctionAConfiguration Aconf;
    pflFunctionA Afunc;
    int initiated;

public: // methods
    pflFunctionB() {
        initiated = 0;
        workarea = nullptr;
    }

    void planner(pflFunctionBConfiguration conf, int* error_status) {
        if (*error_status != 0) return;
        if (initiated==1) {
            *error_status = 1;
            return;
        }
        printf("Planner for function B\n");

        this->conf = conf;
        workarea = (float*)malloc(2*conf.d*2*conf.e*sizeof(float));
        if (workarea == nullptr) {
            *error_status = 1;
        }

        Aconf.a = 2*this->conf.e;
        Aconf.b = 2*this->conf.d;
        Aconf.c = 0.1f;
        if (!Aconf.check()) {
            *error_status = 2;
        }
        Afunc.planner(Aconf, error_status);

        if(*error_status==0) initiated = 1;
    }

    void planner(pflFunctionBConfiguration conf) {
        int error_status = 0;
        planner(conf, &error_status);
        if (error_status != 0) throw new std::exception;
    }

    void run(float *inputB, float *outputB, int *error_status) {
        if (*error_status != 0) return;
        if (initiated==0) {
            *error_status = 1;
            return;
        }
        printf("Running function B...\n");

        Afunc.run(workarea, error_status);

        for (int f = 0; f<conf.e*conf.d; f++) {
            outputB[f] = inputB[f] + workarea[4*f];
        }
    }

    void run(float *inputB, float *outputB) {
        int error_status = 0;
        run(inputB, outputB, &error_status);
        if (error_status != 0) throw new std::exception;
    }

    ~pflFunctionB() {
        printf("Destructor for function B called.\n");
        free(workarea);
    }
};


extern "C" {
    int pflFunctionACall(float *outputA, int A, int B, double C) {
        pflFunctionAConfiguration conf;
        conf.a = A;
        conf.b = B;
        conf.c = C;
        if (conf.check()==false) return (1);
        pflFunctionA runner;
        // runner.planner might throw an exeption if we want to
        try {
            runner.planner(conf);
        }
        catch (...) {
            return (1);
        }
        runner.run(outputA);
    }
}

int main() {
    std::cout << "Hello!\n";

    printf("Configure and run function A\n");
    pflFunctionAConfiguration Aconfig;
    Aconfig.a = 10;
    Aconfig.b = 20;
    Aconfig.c = 0.1;
    if (Aconfig.check()) printf("Configuration A is valid\n");
    else printf("Configuration A is invalid\n");

    float *outputA;
    outputA = (float *)malloc(Aconfig.a*Aconfig.b*sizeof(float));
    if (outputA==nullptr) printf("bad memory allocation for outputA\n");

    pflFunctionA Afunc;
    try {
        Afunc.planner(Aconfig);
    } catch (...) {
        printf("Function A was not configured properly\n");
    }
    try {
        Afunc.run(outputA);
    } catch (...) {
        printf("Error during execution of function A\n");
    }

    printf("\n");

    // Function B
    printf("Configure and run function B\n");
    pflFunctionBConfiguration Bconfig;
    Bconfig.d = 10;
    Bconfig.e = 5;
    if (Bconfig.check()) printf("Configuration B is valid\n");
    else printf("Configuration B is invalid\n");

    float *inputB, *outputB;
    inputB = (float *)malloc(Bconfig.e*Bconfig.d*sizeof(float));
    outputB = (float *)malloc(Bconfig.e*Bconfig.d*sizeof(float));
    if (outputB==nullptr) printf("bad memory allocation for outputB\n");
    if (inputB==nullptr) printf("bad memory allocation for inputB\n");
    for (int f = 0; f<Bconfig.e*Bconfig.d; f++) inputB[f] = f;

    pflFunctionB Bfunc;
    try {
        Bfunc.planner(Bconfig);
    } catch (...) {
        printf("Function B was not configured properly\n");
    }
    try {
        Bfunc.run(inputB, outputB);
    } catch (...) {
        printf("Error during execution of function B\n");
    }



    free(outputA);
    free(outputB);
    free(inputB);
}
