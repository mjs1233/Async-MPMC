#include <iostream>
#include "include/Action.hpp"

struct Job2 {

    void action() && {
        std::cout << "Job2" << std::endl;

    }
};

struct Job1 {

    Action action() && {
        std::cout << "Job1" << std::endl;
        return Job2{};
    }
};



int main() {


    Action ac{Job1{}};
    ac();

    //TODO)
    // 1. load image from files
    // 2. convert image to pixel data
    // 3. apply random task
    // 4. do whatever that is
    // 5. save to file.
    return 0;
}