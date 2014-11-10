#include <Utils/Containers.hpp>
#include "mu/Time.hpp"
#include <iostream>

void TestDynamicArrays() {
    const size_t elements_in_test_num = 104857600;

    typedef mu::Timer<mu::TimeDef::HighResolutionClock, mu::TimeDef::Microseconds> TimerT;

    std::cout << "std::vector<char>\n";
    {
        std::vector<char> vector_;

        TimerT timer;
        timer.Start();
        for(size_t i = 0; i < elements_in_test_num; ++i) {
            vector_.push_back(1);
        }
        timer.Stop();
        std::cout << "Push \t\t" << timer.TimerTime().count() << std::endl;

        TimerT timer2;
        timer2.Start();
        for(size_t i = 0; i < elements_in_test_num; ++i) {
            vector_[i]++;
        }
        timer2.Stop();
        std::cout << "Random access \t" << timer2.TimerTime().count() << std::endl;

        TimerT timer3;
        timer3.Start();
        for(size_t i = 0; i < elements_in_test_num; ++i) {
            vector_.pop_back();
        }
        timer3.Stop();
        std::cout << "Pop \t\t" << timer3.TimerTime().count() << std::endl;
    }

    std::cout << "MR::TDynamicArray<char>\n";
    {
        MR::TDynamicArray<char> darray_;

        TimerT timer;
        timer.Start();
        for(size_t i = 0; i < elements_in_test_num; ++i) {
            darray_.PushBack(1);
        }
        timer.Stop();
        std::cout << "Push \t\t" << timer.TimerTime().count() << std::endl;

        TimerT timer2;
        timer2.Start();
        for(size_t i = 0; i < elements_in_test_num; ++i) {
            darray_.At(i)++;
        }
        timer2.Stop();
        std::cout << "Random access \t" << timer2.TimerTime().count() << std::endl;

        TimerT timer3;
        timer3.Start();
        for(size_t i = 0; i < elements_in_test_num; ++i) {
            darray_.PopBack();
        }
        timer3.Stop();
        std::cout << "Pop \t\t" << timer3.TimerTime().count() << std::endl;
    }
}
