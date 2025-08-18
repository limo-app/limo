#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include "../src/core/deployerentry.hpp"

class DeployerEntryVectorMatcher : public Catch::Matchers::MatcherBase<std::vector<std::weak_ptr<DeployerEntry>>> {
    std::vector<std::weak_ptr<DeployerEntry>> m_expected;
public:
    DeployerEntryVectorMatcher(std::vector<std::weak_ptr<DeployerEntry>> const& expected)
        : m_expected(expected) {}

    bool match(std::vector<std::weak_ptr<DeployerEntry>> const& actual) const override;

    std::string describe() const override;
};
inline DeployerEntryVectorMatcher EqualsDeployerEntryVector(
    std::vector<std::weak_ptr<DeployerEntry>> const& expected) {
    return DeployerEntryVectorMatcher(expected);
}
