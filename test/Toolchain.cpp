/*
 *  test/Toolchain.cpp
 *  Copyright 2020-2021 ItJustWorksTM
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */
#include <filesystem>
#include <iostream>
#include <fstream>
#include <catch2/catch_test_macros.hpp>
#include "SMCE/Toolchain.hpp"
#include "defs.hpp"

TEST_CASE("Toolchain invalid", "[Toolchain]") {
    const auto path = SMCE_TEST_DIR "/empty_dir";
    std::filesystem::create_directory(path);
    smce::Toolchain tc{path};
    REQUIRE(tc.check_suitable_environment());
    REQUIRE(tc.resource_dir() == path);

    //Test when configure fails.
    smce::Sketch sk{path,{.fqbn = "null"}};
    auto ec = tc.compile(sk);
    REQUIRE(ec == smce::toolchain_error::configure_failed);

    //Test when sketch fails due to no source.
    smce::Sketch skTwo{"" ,{.fqbn = "arduino:avr:nano"}};
    ec = tc.compile(skTwo);
    REQUIRE(ec == smce::toolchain_error::sketch_invalid);

    //Test when sketch fails due to that configuration .fqbn is empty.
    smce::Sketch skThree{path ,{.fqbn = ""}};
    ec = tc.compile(skThree);
    REQUIRE(ec == smce::toolchain_error::sketch_invalid);

    //Test when resource directory is absent due to empty.
    smce::Toolchain tcTwo{""};
    REQUIRE(tcTwo.check_suitable_environment() == smce::toolchain_error::resdir_absent);

    //Test when resource directory is empty.
    const auto newPath = SMCE_TEST_DIR "/empty_dir/empty";
    std::filesystem::create_directory(newPath);
    smce::Toolchain tcThree{SMCE_TEST_DIR "/empty_dir/empty"};
    REQUIRE(tcThree.check_suitable_environment() == smce::toolchain_error::resdir_empty);

    //Test when resource directory is pointing to a file.
    const auto textPath = SMCE_TEST_DIR "/empty_dir/testfile.txt";
    std::ofstream ofs(textPath);
    ofs << "Test file for toolchain\n";
    ofs.close();
    smce::Toolchain tcFour{SMCE_TEST_DIR "/empty_dir/testfile.txt"};
    REQUIRE(tcFour.check_suitable_environment() == smce::toolchain_error::resdir_file);

}

TEST_CASE("Toolchain valid", "[Toolchain]") {
    smce::Toolchain tc{SMCE_PATH};
    REQUIRE(!tc.check_suitable_environment());
    REQUIRE(tc.resource_dir() == SMCE_PATH);
    REQUIRE_FALSE(tc.cmake_path().empty());
}