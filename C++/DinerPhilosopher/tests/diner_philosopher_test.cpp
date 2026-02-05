#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <iostream>
#include "diner_philosopher.hpp"

TEST_CASE("Full test") {
    std::cout << "=== Diner Philosopher Tests ===\n";
    std::cout << "Running 5 philosophers for 3 seconds...\n";

    DinerPhilosopher table(5);
    table.run_for(std::chrono::seconds(3));

    std::cout << "\n=== Test Complete ===";
    std::cout << "If you see eat counts for all philosophers, no deadlock occured\n";
}
