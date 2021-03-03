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
        v.push_back(i);
        auto j = d.push_back(i);
        auto v_i = v.at(i);
        auto d_i = d.at(i);
        assert(v_i == d_i);
        assert(d.at(j) == d_i);
        assert(d.size() == v.size());
    }

    for(int i = 0; i < 20000000; ++i){
        auto d_i = d.at(i);
        auto x = d.data()[i];
        assert(x == d_i);
    }

    assert(v.size() == d.size());
}

TEST_CASE("pre-alloc constructor", "[pre_alloc]"){
    size_t sz = 200000;
    std::vector<int> v(sz);
    dtl::DistributedVector<int> d(sz);

    REQUIRE(v.size() == d.size());

    for(int i = 0; i < sz; ++i){
        v[i] = i;
        d.data()[i] = i;
    }
    for(int i = sz; i < 3*sz; ++i){
        v.push_back(i);
        d.push_back(i);
    }

    for(int i = 0; i < 3*sz; ++i){
        auto d_i = d.at(i);
        auto x = d.data()[i];
        REQUIRE(x == d_i);
    }

    REQUIRE(v.size() == d.size());
}

struct RandomPod {
    float x=0.5f;
    float y=0.0f;
    float z=0.0f;
    ~RandomPod(){
        std::cout << "~RandomPod() called\n";
    }
};

TEST_CASE("POD object", "[pod_obj]"){
    dtl::DistributedVector<RandomPod> v;

    {
        RandomPod p;
        v.push_back(p);
        v.push_back(p);
        v.push_back(p);
        v.push_back(p);
        v.push_back(p);
    }

    for(int i = 0; i < 5; ++i){
        assert(v.at(i).x == 0.5f);
    }

    dtl::DistributedVector<RandomPod*> v2;
}

TEST_CASE("Destructor", "[destructor]"){

    {
        dtl::DistributedVector<RandomPod> v;
        RandomPod p;
        v.push_back(p);
        v.push_back(p);
        v.push_back(p);
        v.push_back(p);
        v.push_back(p);
    }

}