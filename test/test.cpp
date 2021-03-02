//
// Created by borosm on 10/14/2020.
//

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "../DistributedVector.h"
#include <vector>
#include <iostream>
#include <typeinfo>

TEST_CASE("Base case", "[base_case]"){
    std::vector<int> v;
    dtl::DistributedVector<int> d;

    assert(v.size() == d.size());

    for(int i = 0; i < 20000000; ++i){
        v.push_back(i); d.push_back(i);
        auto v_i = v.at(i);
        auto d_i = d.at(i);
        assert(v_i == d_i);
        if (i % 100 == 0)
            std::cout << v_i << ":" << d_i << '\n';
    }
    std::cout << '\n';

    assert(v.size() == d.size());
}
