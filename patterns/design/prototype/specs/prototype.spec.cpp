#include <igloo/igloo_alt.h>

#include "../src/monsters/Ghost.h"
#include "../src/monsters/Demon.h"
#include "../src/monsters/Sorcerer.h"
#include "../src/monsters/Spawner.h"

using namespace igloo;

Describe(CloneSpec) {

    It(produces_a_monster_of_the_same_type) {
        Ghost ghost;
        auto copy = ghost.clone();
        Assert::That(copy->name(), Equals("Ghost"));
    };

    It(carries_the_prototype_stats) {
        Demon demon;
        auto copy = demon.clone();
        Assert::That(copy->health(), Equals(demon.health()));
        Assert::That(copy->speed(),  Equals(demon.speed()));
    };

    It(is_an_independent_object) {
        Ghost ghost;
        auto copy = ghost.clone();
        Assert::That(copy.get() != &ghost, Equals(true));
    };
};

Describe(SpawnerSpec) {

    It(spawns_the_type_of_its_prototype) {
        Spawner spawner(std::make_unique<Ghost>());
        Assert::That(spawner.spawn()->name(), Equals("Ghost"));
    };

    It(spawns_distinct_instances) {
        Spawner spawner(std::make_unique<Demon>());
        auto a = spawner.spawn();
        auto b = spawner.spawn();
        Assert::That(a.get() != b.get(), Equals(true));
    };

    It(spawns_the_new_type_after_the_prototype_is_swapped) {
        Spawner spawner(std::make_unique<Ghost>());
        Assert::That(spawner.spawn()->name(), Equals("Ghost"));
        spawner.setPrototype(std::make_unique<Sorcerer>());
        Assert::That(spawner.spawn()->name(), Equals("Sorcerer"));
    };

    It(spawns_carry_the_prototypes_stats) {
        Sorcerer prototype;
        Spawner spawner(std::make_unique<Sorcerer>());
        auto spawned = spawner.spawn();
        Assert::That(spawned->health(), Equals(prototype.health()));
        Assert::That(spawned->speed(),  Equals(prototype.speed()));
    };
};

Describe(MonsterTypesSpec) {

    It(give_each_type_distinct_stats) {
        Ghost ghost; Demon demon; Sorcerer sorcerer;
        Assert::That(ghost.health()  != demon.health(),   Equals(true));
        Assert::That(demon.speed()   != sorcerer.speed(), Equals(true));
    };
};
