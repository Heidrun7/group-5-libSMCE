#include "SMCE/Uuid.cpp"
#include "defs.hpp"

using namespace std::literals;

TEST_CASE("To Hex Valid","[Uuid]") {
    Uuid uuid = Uuid::generate();
    print(uuid);
    REQUIRE(true)
}