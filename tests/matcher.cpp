#include "matcher.h"

bool DeployerEntryVectorMatcher::match(std::vector<std::weak_ptr<DeployerEntry>> const& actual) const {
  if (actual.size() != m_expected.size()) return false;

  for (size_t i = 0; i < actual.size(); ++i) {
    auto a = actual[i].lock();
    auto e = m_expected[i].lock();

    if (!a || !e) return false;
    if (*a != *e) return false; // Uses DeployerEntry's operator==
  }
  return true;
}

std::string DeployerEntryVectorMatcher::describe() const {
  std::ostringstream ss;
  ss << "contains equal DeployerEntry objects as reference vector";
  return ss.str();
}

