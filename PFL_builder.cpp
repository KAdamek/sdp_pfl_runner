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


//--------------- Function A ----------------
// Function A which does not call other PF functions
class pflFunctionABuilder;
class pflFunctionAConfiguration;
class pflFunctionA;

// Configuration
class pflFunctionAConfiguration {
public:
    int a;
    int b;
    float c;

    pflFunctionAConfiguration() {
        a = 0;
        b = 0;
        c = 0;
    }

    bool check() {
        if (a<=0 || b<=0) return false;
        return true;
    }
};
   
// Function
class pflFunctionA {
    friend class pflFunctionABuilder;

private: // variables
    pflFunctionAConfiguration conf;

    float *workarea{};

    pflFunctionA(pflFunctionAConfiguration outside_config) {
        conf = outside_config;
    }

public:
    void run(float *outputA, int *error_status) {
        printf("Running function A...\n");
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
        if (error_status!=0) throw new std::exception;
    }

    ~pflFunctionA() {
        printf("Destructor for function A called.\n");
        if (workarea!=nullptr) {
            free(workarea);
            workarea = nullptr;
        }
    }
};

// Builder
class pflFunctionABuilder {
public: // variables
    pflFunctionAConfiguration config;

public: // methods
    pflFunctionABuilder() {}

    pflFunctionA *build() {
        printf("Building function A\n");
        if (config.check()) {
            pflFunctionA *temp = new pflFunctionA(config);
            temp->workarea = (float *)malloc(config.a*config.b*sizeof(float));
            return(temp);
        } else {
            return nullptr;
        }
    }

    pflFunctionABuilder &fromFile() {
        printf("Pretending to loading a file and configure based on that\n");
        config.a = 5;
        config.b = 6;
        config.c = 0.1f;
        return *this;
    };

    pflFunctionABuilder &fromConfiguration(pflFunctionAConfiguration &ref_config) {
        config = ref_config;
        return *this;
    }
};


//--------------- Function B ----------------
// Function B which does call other PF functions
class pflFunctionBBuilder;
class pflFunctionBConfiguration;
class pflFunctionB;

// Configuration
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

// Function
class pflFunctionB {
    friend class pflFunctionBBuilder;

private: // variables
    pflFunctionBConfiguration conf;
    pflFunctionA *funcA{}; // <- some other PF function

    float *workarea{};

    pflFunctionB(pflFunctionBConfiguration outside_config) {
        conf = outside_config;
    }

public:

    void run(float *inputB, float *outputB, int *error_status) {
        if (*error_status!=0) return;
        
        printf("Running function B...\n");
        funcA->run(workarea, error_status);

        for (int f = 0; f<conf.e*conf.d; f++) {
            outputB[f] = inputB[f] + workarea[4*f];
        }
    }

    void run(float *inputB, float *outputB) {
        int error_status = 0;
        run(inputB, outputB, &error_status);
        if (error_status!=0) throw new std::exception;
    }

    ~pflFunctionB() {
        printf("Destructor for function B called.\n");
        delete funcA; // Need to call this to avoid memory leaks

        if (workarea!=nullptr) {
            free(workarea);
            workarea = nullptr;
        }
    }
};

// Builder
class pflFunctionBBuilder {
public: // variables
    pflFunctionBConfiguration config;

public: // methods
    pflFunctionBBuilder() {
        config.d = 0;
        config.e = 0;
    }

    pflFunctionB *build() {
        printf("Building function B.\n");
        if (!config.check()) return nullptr;

        int is_all_configured = 1;

        // Configuring function B
        pflFunctionB *temp = new pflFunctionB(config);
        temp->workarea = (float *)malloc(2*config.e*2*config.d*sizeof(float));

        // Building function A
        pflFunctionAConfiguration Aconf;
        Aconf.a = 2*config.e;
        Aconf.b = 2*config.d;
        Aconf.c = 0.1f;
        if (Aconf.check()) {
            pflFunctionABuilder ABuilder;
            temp->funcA = ABuilder.fromConfiguration(Aconf).build();
            if (temp->funcA==nullptr) {
                printf("Error while building function A\n");
                is_all_configured *= 0;
            }
        }

        if (is_all_configured) {
            return (temp);
        } else {
            delete temp;
            return nullptr;
        }
    }

    pflFunctionBBuilder &fromFile() {
        printf("Pretending to loading a file and configure based on that\n");
        config.e = 5;
        config.d = 6;
        return *this;
    };

    pflFunctionBBuilder &fromConfiguration(pflFunctionBConfiguration &ref_config) {
        config = ref_config;
        return *this;
    }
};




int main() {
    std::cout << "Hello!\n";

    printf("Configure and run function A\n");
    pflFunctionAConfiguration Aconfig;
    Aconfig.a = 10;
    Aconfig.b = 20;
    Aconfig.c = 0.1f;
    if (Aconfig.check()) printf("Configuration A is valid\n");
    else printf("Configuration A is invalid\n");

    float *outputA;
    outputA = (float *)malloc(Aconfig.a*Aconfig.b*sizeof(float));
    if (outputA==nullptr) printf("bad memory allocation for outputA\n");
    
    pflFunctionABuilder ABuilder;
    ABuilder.fromConfiguration(Aconfig);
    pflFunctionA *Afunc = ABuilder.build();
    if (Afunc==nullptr) printf("Function A was not constructed properly\n");
    else {
        try {
            Afunc->run(outputA);
        } catch (...) {
            printf("Error during execution of function A!\n");
        }
    }
    delete Afunc;

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
    else for (int f = 0; f<Bconfig.e*Bconfig.d; f++) inputB[f] = (float) f;

    pflFunctionBBuilder BBuilder;
    BBuilder.fromConfiguration(Bconfig);
    pflFunctionB *Bfunc = BBuilder.build();
    if (Bfunc==nullptr) printf("Function B was not constructed properly\n");
    else {
        try {
            Bfunc->run(inputB, outputB);
        } catch (...) {
            printf("Error during execution of function B!\n");
        }
    }
    delete Bfunc;


    free(outputA);
    free(outputB);
    free(inputB);
}
