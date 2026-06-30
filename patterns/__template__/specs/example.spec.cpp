#include <igloo/igloo_alt.h>

using namespace igloo;

// Sample spec — replace with specs for your pattern's classes.
// Igloo is header-only; tests read as plain-English Describe/It blocks.
Describe(ExampleSpec) {

    It(passes_a_placeholder_assertion) {
        Assert::That(1 + 1, Equals(2));
    };

    It(supports_string_matchers) {
        std::string greeting = "hello";
        Assert::That(greeting, Equals("hello"));
    };
};
