#include <gtest/gtest.h>
#include <requirements.hpp>

enum class NiceGuys
{
    Kyle,
    John,
    Harry,
    Jack,
    Joe
};

using ng = NiceGuys;

class RequirementsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        req1.add(ng::Kyle, ng::Jack);   // Kyle requires Jack
        req1.add(ng::Jack, ng::John);   // Jack requires John
        req1.add(ng::Joe, ng::John);    // Joe requires John
        req2.add(ng::Harry, ng::Joe);   // Harry requires Joe
        req2.add(ng::Joe, ng::Harry);   // Joe requires Harry (only allowed if reflexive)
    }

    // void TearDown() override {}

    requirements::Requirements<NiceGuys> req0{ false };
    requirements::Requirements<NiceGuys> req1{ false };
    requirements::Requirements<NiceGuys> req2{ true };
};

using RequirementsDeathTest = RequirementsTest;

TEST_F(RequirementsTest, Initialization)
{
    EXPECT_FALSE(req0.reflexive());
    EXPECT_TRUE(req0.empty());
    EXPECT_EQ(req0.size(), 0);
    EXPECT_FALSE(req1.reflexive());
    EXPECT_FALSE(req1.empty());
    EXPECT_EQ(req1.size(), 3);
    EXPECT_TRUE(req2.reflexive());
    EXPECT_FALSE(req2.empty());
    EXPECT_EQ(req2.size(), 2);
}

TEST_F(RequirementsDeathTest, Assertion_If_Non_Reflexive)
{
    EXPECT_DEATH(req1.add(ng::Jack, ng::Kyle), "");     // only allowed if reflexive but req1 is not reflexive
}

TEST_F(RequirementsTest, Exists)
{
    EXPECT_TRUE(req1.exists(ng::Kyle, ng::Jack) && req1.exists(ng::Jack, ng::John) && req1.exists(ng::Joe, ng::John));
    EXPECT_TRUE(req2.exists(ng::Harry, ng::Joe) && req2.exists(ng::Joe, ng::Harry));
}

TEST_F(RequirementsTest, Has_Requirements)
{
    EXPECT_TRUE(req1.has_requirements(ng::Kyle));
    EXPECT_TRUE(req1.has_requirements(ng::Jack));
    EXPECT_TRUE(req1.has_requirements(ng::Joe));
    EXPECT_FALSE(req1.has_requirements(ng::John));
    EXPECT_FALSE(req1.has_requirements(ng::Harry));
    EXPECT_TRUE(req2.has_requirements(ng::Harry));
    EXPECT_TRUE(req2.has_requirements(ng::Joe));
    EXPECT_FALSE(req2.has_requirements(ng::Kyle));
    EXPECT_FALSE(req2.has_requirements(ng::Jack));
    EXPECT_FALSE(req2.has_requirements(ng::John));
}

TEST_F(RequirementsTest, Has_Dependents)
{
    EXPECT_TRUE(req1.has_dependents(ng::Jack));
    EXPECT_TRUE(req1.has_dependents(ng::John));
    EXPECT_FALSE(req1.has_dependents(ng::Kyle));
    EXPECT_FALSE(req1.has_dependents(ng::Harry));
    EXPECT_FALSE(req1.has_dependents(ng::Joe));
    EXPECT_TRUE(req2.has_dependents(ng::Harry));
    EXPECT_TRUE(req2.has_dependents(ng::Joe));
    EXPECT_FALSE(req2.has_dependents(ng::Kyle));
    EXPECT_FALSE(req2.has_dependents(ng::Jack));
    EXPECT_FALSE(req2.has_dependents(ng::John));
}

TEST_F(RequirementsTest, Dependents)
{
    auto deps = req1.dependents(ng::John);
    ASSERT_EQ(deps.size(), 2);
    EXPECT_TRUE(deps[0] == ng::Jack || deps[0] == ng::Joe);
    if (deps[0] == ng::Jack)
        EXPECT_EQ(deps[1], ng::Joe);
    else
        EXPECT_EQ(deps[1], ng::Jack);
}

TEST_F(RequirementsTest, All_Requirements)
{
    auto dep_paths = req1.all_requirements(true);
    EXPECT_EQ(dep_paths.size(), 2);
    for (auto path : dep_paths)
        EXPECT_EQ(path.back(), ng::John);
}

TEST_F(RequirementsTest, All_Dependencies)
{
    auto req_paths = req1.all_dependencies(true);
    EXPECT_EQ(req_paths.size(), 2);
    for (auto path : req_paths)
        EXPECT_TRUE(path.back() == ng::Joe || path.back() == ng::Kyle);
}

TEST_F(RequirementsTest, Exists_Recursive)
{
    EXPECT_TRUE(req1.exists(ng::Kyle, ng::John, true));
    EXPECT_FALSE(req1.exists(ng::Jack, ng::Joe, true));
    EXPECT_TRUE(req2.exists(ng::Harry, ng::Joe, true));
    EXPECT_TRUE(req2.exists(ng::Joe, ng::Harry, true));
}

TEST_F(RequirementsTest, Remove_All)
{
    req1.remove_all(ng::Jack);
    EXPECT_FALSE(req1.has_requirements(ng::Kyle));
    EXPECT_FALSE(req1.exists(ng::Jack, ng::John));
    EXPECT_EQ(req1.size(), 1);
}

TEST_F(RequirementsTest, Clear)
{
    req1.clear();
    req2.clear();
    EXPECT_EQ(req1.size(), 0);
    EXPECT_TRUE(req1.empty());
    EXPECT_EQ(req2.size(), 0);
    EXPECT_TRUE(req2.empty());
}
