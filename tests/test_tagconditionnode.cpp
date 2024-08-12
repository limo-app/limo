#include "../src/core/autotag.h"
#include "../src/core/tagconditionnode.h"
#include "test_utils.h"
#include <catch2/catch_test_macros.hpp>


TEST_CASE("Expressions are validated", "[.tags]")
{
  REQUIRE_FALSE(TagConditionNode::expressionIsValid("", 1));
  REQUIRE_FALSE(TagConditionNode::expressionIsValid("and", 1));
  REQUIRE_FALSE(TagConditionNode::expressionIsValid("or", 1));
  REQUIRE_FALSE(TagConditionNode::expressionIsValid("not", 1));
  REQUIRE_FALSE(TagConditionNode::expressionIsValid("1", 1));
  REQUIRE_FALSE(TagConditionNode::expressionIsValid("0  and", 1));
  REQUIRE_FALSE(TagConditionNode::expressionIsValid("0 and and 0", 1));
  REQUIRE_FALSE(TagConditionNode::expressionIsValid("0 (and 0)", 1));
  REQUIRE_FALSE(TagConditionNode::expressionIsValid("0 not 0", 1));
  REQUIRE_FALSE(TagConditionNode::expressionIsValid("0()", 1));
  REQUIRE_FALSE(TagConditionNode::expressionIsValid("0 (or) 0", 1));
  REQUIRE_FALSE(TagConditionNode::expressionIsValid("(0 or 0))", 1));
  REQUIRE_FALSE(TagConditionNode::expressionIsValid("(0 (not 0)", 1));
  REQUIRE_FALSE(TagConditionNode::expressionIsValid("0 or not 0F", 1));
  REQUIRE_FALSE(TagConditionNode::expressionIsValid("0 an 0", 1));
  REQUIRE_FALSE(TagConditionNode::expressionIsValid("0 and not 1", 1));
  REQUIRE_FALSE(TagConditionNode::expressionIsValid("0 and 2 or 3 and not 4 and 5", 5));
  REQUIRE_FALSE(TagConditionNode::expressionIsValid("obviously invalid", 1));
  REQUIRE_FALSE(TagConditionNode::expressionIsValid("0 not and 1", 2));
  REQUIRE_FALSE(TagConditionNode::expressionIsValid("0 an d 1", 2));

  REQUIRE(TagConditionNode::expressionIsValid("0", 1));
  REQUIRE(TagConditionNode::expressionIsValid("not1", 2));
  REQUIRE(TagConditionNode::expressionIsValid("0 and      0", 1));
  REQUIRE(TagConditionNode::expressionIsValid("((0)or(0))", 1));
  REQUIRE(TagConditionNode::expressionIsValid("0 and not 1", 2));
  REQUIRE(TagConditionNode::expressionIsValid("notnotnot0 and not not1 or not00001", 2));
  REQUIRE(TagConditionNode::expressionIsValid("(0 or not 1) andnot 2 and (0 or 0)", 3));
  REQUIRE(TagConditionNode::expressionIsValid("not(not0) and (1) or    2", 3));
}

TEST_CASE("Single node detects files", "[.tags]")
{
  std::vector<TagCondition> conditions{
    { false, TagCondition::Type::file_name, false, "*.txt" },
    { false, TagCondition::Type::file_name, false, "*12*abc" },
    { false, TagCondition::Type::path, false, "dir/abc/*c_1*" },
    { true, TagCondition::Type::file_name, false, "fawefw*fQFQ*3q*" },
    { false, TagCondition::Type::file_name, true, R"(some_\d+_file_.b.)" },
    { false, TagCondition::Type::path, true, R"(d\wr/some_\d+_file_.b.)" },
    { false, TagCondition::Type::file_name, false, "*a*a*a*" }
  };
  const auto files =
    AutoTag::readModFiles(DATA_DIR / "source" / "auto_tags", std::vector<int>{ 0, 1 });

  TagConditionNode node("0", conditions);
  REQUIRE(node.evaluate(files.at(0)));
  REQUIRE_FALSE(node.evaluate(files.at(1)));

  TagConditionNode node_2("1", conditions);
  REQUIRE(node_2.evaluate(files.at(0)));
  REQUIRE_FALSE(node_2.evaluate(files.at(1)));

  TagConditionNode node_3("2", conditions);
  REQUIRE(node_3.evaluate(files.at(0)));
  REQUIRE_FALSE(node_3.evaluate(files.at(1)));

  TagConditionNode node_4("3", conditions);
  REQUIRE(node_4.evaluate(files.at(0)));
  REQUIRE(node_4.evaluate(files.at(1)));

  TagConditionNode node_5("4", conditions);
  REQUIRE(node_5.evaluate(files.at(0)));
  REQUIRE_FALSE(node_5.evaluate(files.at(1)));

  TagConditionNode node_6("5", conditions);
  REQUIRE(node_6.evaluate(files.at(0)));
  REQUIRE_FALSE(node_6.evaluate(files.at(1)));

  TagConditionNode node_7("6", conditions);
  REQUIRE(node_7.evaluate(files.at(0)));
  REQUIRE_FALSE(node_7.evaluate(files.at(1)));
}

TEST_CASE("Expressions of depth 1 are parsed", "[.tags]")
{
  std::vector<TagCondition> conditions{ { false, TagCondition::Type::file_name, false, "*.txt" },
                                        { false, TagCondition::Type::file_name, false, "*12*abc" },
                                        { false, TagCondition::Type::path, false, "dir/abc/*c_1*" },
                                        { false, TagCondition::Type::file_name, false, "r*3" } };
  const auto files =
    AutoTag::readModFiles(DATA_DIR / "source" / "auto_tags", std::vector<int>{ 0, 1 });

  TagConditionNode node("0 and 1 and 2 and 3", conditions);
  REQUIRE(node.evaluate(files.at(0)));
  REQUIRE_FALSE(node.evaluate(files.at(1)));

  TagConditionNode node2("0 or 1 or 2 or 3", conditions);
  REQUIRE(node2.evaluate(files.at(0)));
  REQUIRE(node2.evaluate(files.at(1)));
}

TEST_CASE("Complex expressions are parsed", "[.tags]")
{
  std::vector<TagCondition> conditions{
    { false, TagCondition::Type::file_name, false, "*.txt" },
    { false, TagCondition::Type::file_name, false, "*12*abc" },
    { false, TagCondition::Type::path, false, "dir/abc/*c_1*" },
    { true, TagCondition::Type::file_name, false, "fawefw*fQFQ*3q*" },
    { false, TagCondition::Type::file_name, true, R"(some_\d+_file_.b.)" },
    { false, TagCondition::Type::path, true, R"(d\wr/.*\.png)" },
    { false, TagCondition::Type::file_name, false, "*rw3*" },
    { false, TagCondition::Type::file_name, false, "unique_*f*" },
    { false, TagCondition::Type::path, false, "j/n" },
    { false, TagCondition::Type::path, false, "qwert" },
    { false, TagCondition::Type::path, true, R"(j/a_fi.*)" }
  };
  const auto files =
    AutoTag::readModFiles(DATA_DIR / "source" / "auto_tags", std::vector<int>{ 0, 1, 2 });

  TagConditionNode node("not(not(not(0 and 1 and 2 and not not 3) or 4 and 3)) ", conditions);
  REQUIRE(node.evaluate(files.at(0)));
  REQUIRE(node.evaluate(files.at(1)));

  TagConditionNode node2("(not(not(0 and 1 and 2 and not not 3) or 4 and 3)) ", conditions);
  REQUIRE_FALSE(node2.evaluate(files.at(0)));
  REQUIRE_FALSE(node2.evaluate(files.at(1)));

  TagConditionNode node3("0 and 1 and 2 and 3 and 4", conditions);
  TagConditionNode node4("5 and 6", conditions);
  TagConditionNode node5("not 9 and 10 or 7 and 8", conditions);
  REQUIRE(node3.evaluate(files.at(0)));
  REQUIRE(node4.evaluate(files.at(1)));
  REQUIRE(node5.evaluate(files.at(2)));

  TagConditionNode node6(
    "0 and 1 and 2 and 3 and 4 and not(5 and 6) and not (not 9 and 10 or 7 and 8)", conditions);
  TagConditionNode node7(
    "not(0 and 1 and 2 and 3 and 4) and 5 and 6 and not(not 9 and 10 or 7 and 8)", conditions);
  TagConditionNode node8(
    "not(0 and 1 and 2 and 3 and 4) and not(5 and 6) and (not 9 and 10 or 7 and 8)", conditions);
  REQUIRE(node6.evaluate(files.at(0)));
  REQUIRE_FALSE(node7.evaluate(files.at(0)));
  REQUIRE_FALSE(node8.evaluate(files.at(0)));
  REQUIRE_FALSE(node6.evaluate(files.at(1)));
  REQUIRE(node7.evaluate(files.at(1)));
  REQUIRE_FALSE(node8.evaluate(files.at(1)));
  REQUIRE_FALSE(node6.evaluate(files.at(2)));
  REQUIRE_FALSE(node7.evaluate(files.at(2)));
  REQUIRE(node8.evaluate(files.at(2)));

  TagConditionNode node9(
    "not not(0 and not not 1 and 2 and 3 and 4 or 5 and 6 or (not 9 and 10 or 7 and 8))",
    conditions);
  REQUIRE(node9.evaluate(files.at(0)));
  REQUIRE(node9.evaluate(files.at(1)));
  REQUIRE(node9.evaluate(files.at(2)));

  TagConditionNode node10("0 or 6 and 5", conditions);
  REQUIRE(node10.evaluate(files.at(0)));
  REQUIRE(node10.evaluate(files.at(1)));

  TagConditionNode node11("(0 or 6) and 5", conditions);
  REQUIRE_FALSE(node11.evaluate(files.at(0)));
  REQUIRE(node11.evaluate(files.at(1)));
}
