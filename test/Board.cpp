
#include <chrono>
#include <fstream>
#include <future>
#include <iostream>
#include <catch2/catch_test_macros.hpp>
#include "SMCE/Board.hpp"
#include "SMCE/Sketch.hpp"
#include "SMCE/Toolchain.hpp"
#include "defs.hpp"

using namespace std::literals;

TEST_CASE("Board contracts", "[Board]") {
    smce::Toolchain tc{SMCE_PATH};
    REQUIRE(!tc.check_suitable_environment());
    smce::Sketch sk{SKETCHES_PATH "noop", {.fqbn = "arduino:avr:nano"}};
    const auto ec = tc.compile(sk);
    if (ec)
        std::cerr << tc.build_log().second;
    REQUIRE_FALSE(ec);
    REQUIRE(sk.is_compiled());
    smce::Board br{};
    REQUIRE(br.status() == smce::Board::Status::clean);
    REQUIRE_FALSE(br.view().valid());
    REQUIRE(br.configure({}));
    REQUIRE(br.status() == smce::Board::Status::configured);
    REQUIRE_FALSE(br.view().valid());
    REQUIRE(br.attach_sketch(sk));
    REQUIRE_FALSE(br.view().valid());
    REQUIRE(br.start());
    REQUIRE(br.status() == smce::Board::Status::running);
    REQUIRE(br.view().valid());
    REQUIRE(br.suspend());
    REQUIRE(br.status() == smce::Board::Status::suspended);
    //Can not suspend if it is already suspended
    REQUIRE_FALSE(br.suspend());
    //Can not attach sketch if suspended.
    REQUIRE_FALSE(br.attach_sketch(sk));
    //If the board is suspended - not possible to reset.
    REQUIRE_FALSE(br.reset());
    REQUIRE(br.view().valid());
    REQUIRE(br.resume());
    REQUIRE(br.status() == smce::Board::Status::running);
    //Can not attach sketch if already running.
    REQUIRE_FALSE(br.attach_sketch(sk));
    REQUIRE(br.view().valid());
    REQUIRE(br.stop());
    REQUIRE(br.status() == smce::Board::Status::stopped);
    REQUIRE_FALSE(br.view().valid());
    REQUIRE(br.reset());
    REQUIRE(br.status() == smce::Board::Status::clean);
    REQUIRE_FALSE(br.view().valid());
}

TEST_CASE("Board exit_notify", "[Board]") {
    smce::Toolchain tc{SMCE_PATH};
    REQUIRE(!tc.check_suitable_environment());
    smce::Sketch sk{SKETCHES_PATH "uncaught", {.fqbn = "arduino:avr:nano"}};
    const auto ec = tc.compile(sk);
    if (ec)
        std::cerr << tc.build_log().second;
    REQUIRE_FALSE(ec);
    std::promise<int> ex;
    smce::Board br{[&](int ec) { ex.set_value(ec); }};
    REQUIRE(br.configure({}));
    REQUIRE(br.attach_sketch(sk));
    REQUIRE(br.start());
    auto exfut = ex.get_future();
    int ticks = 0;
    while (ticks++ < 5 && exfut.wait_for(0ms) != std::future_status::ready) {
        exfut.wait_for(1s);
        br.tick();
    }
    REQUIRE(exfut.wait_for(0ms) == std::future_status::ready);
    REQUIRE(exfut.get() != 0);
}

TEST_CASE("Mixed INO/C++ sources", "[Board]") {
    smce::Toolchain tc{SMCE_PATH};
    REQUIRE(!tc.check_suitable_environment());
    smce::Sketch sk{SKETCHES_PATH "with_cxx", {.fqbn = "arduino:avr:nano"}};
    const auto ec = tc.compile(sk);
    if (ec)
        std::cerr << tc.build_log().second;
    REQUIRE_FALSE(ec);
}


TEST_CASE("Board start", "[Board]")
{
    smce::Toolchain tc{SMCE_PATH};
    REQUIRE(!tc.check_suitable_environment());

    //Initialize sketch
    smce::Sketch sk{SKETCHES_PATH "with_cxx", {.fqbn = "arduino:avr:nano"}};
    //Initialize board
    smce::Board br{};

    //If board has not been configured it can not be started.
    REQUIRE_FALSE(br.start());
    REQUIRE(br.configure({}));
    REQUIRE(br.status() == smce::Board::Status::configured);

    //Configured but no sketch attached = not be able to start.
    REQUIRE_FALSE(br.start());

    //Should not be able to start with an uncompiled attatched sketch.
    REQUIRE(br.attach_sketch(sk));
    REQUIRE_FALSE(br.start());
    tc.compile(sk);
    REQUIRE(br.attach_sketch(sk));
    REQUIRE(br.start());

    // If board is already running, it can not be started.
    REQUIRE_FALSE(br.start());
}

