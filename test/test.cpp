//
// Created by Martin Boros on 3/1/21.
//

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "../DistributedVector.h"
#include <vector>
#include <iostream>
#include <typeinfo>
#include <wait.h>
#include <thread>

TEST_CASE("Base case", "[base_case]"){
    std::vector<int> v;
    dtl::DistributedVector<int> d;

    REQUIRE(v.size() == d.size());

    for(int i = 0; i < 2000000; ++i){
        v.push_back(i);
        auto j = d.push_back(i);
        auto v_i = v.at(i);
        auto d_i = d.at(i);
        REQUIRE(v_i == d_i);
        REQUIRE(d.at(j) == d_i);
        REQUIRE(d.size() == v.size());
    }

    for(int i = 0; i < 2000000; ++i){
        auto d_i = d.at(i);
        auto x = d.data()[i];
        REQUIRE(x == d_i);
    }

    REQUIRE(v.size() == d.size());
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
        REQUIRE(v.at(i).x == 0.5f);
    }

    v.at(4).x = 1.0f;
    for(int i = 0; i < 4; ++i){
        REQUIRE(v.at(i).x == 0.5f);
    }
    REQUIRE(v.at(4).x == 1.0f);


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

TEST_CASE("fork", "[fork]"){
    dtl::DistributedVector<int> v;

    int pid = fork();

    for(int i = 0; i < 10; ++i){
        v.push_back(i);
    }
    printf("%p\n", &v);

    if (pid != 0){
        int child_status;
        waitpid(pid, &child_status, 0);
        for(int i = 0; i < 23; ++i){
            std::cout << v.data()[i] << ' ';
        }
        std::cout << '\n';
        REQUIRE(v.size() == 23);
    } else {
        v.push_back(11);
        v.push_back(11);
        v.push_back(11);
    }
}

void thread_push(dtl::DistributedVector<int> v){
    for(int i = 0; i < 10; ++i){
        v.push_back(i);
    }
}

TEST_CASE("threads", "[threads]"){
    dtl::DistributedVector<int> v;

    for(int i = 0; i < 10; ++i){
        std::thread t(thread_push, v);
        t.detach();
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));
    for(int i = 0; i < v.size(); ++i){
        std::cout << v.at(i) << ' ';
    }
    std::cout << '\n';
    std::cout << v.size() << ' ' << v.capacity() << '\n';

}