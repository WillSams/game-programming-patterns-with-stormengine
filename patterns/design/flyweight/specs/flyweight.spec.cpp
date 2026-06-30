#include <igloo/igloo_alt.h>
#include <set>

#include "../src/worlds/Terrain.h"
#include "../src/worlds/World.h"

using namespace igloo;

Describe(TerrainSpec) {
    It(exposes_its_intrinsic_movement_cost) {
        Terrain grass(1, false, true, false, 0);
        Assert::That(grass.getMovementCost(), Equals(1));
    };

    It(maps_a_texture_index_to_a_name) {
        Terrain grass(1, false, true, false, 0);
        Terrain river(2, true, false, false, 1);
        Terrain hill(3, false, false, true, 2);
        Assert::That(grass.getTextureName(), Equals("grass"));
        Assert::That(river.getTextureName(), Equals("water"));
        Assert::That(hill.getTextureName(), Equals("hill"));
    };

    It(reports_whether_it_is_water) {
        Terrain river(2, true, false, false, 1);
        Assert::That(river.isWater(), Equals(true));
    };
};

Describe(WorldSpec) {
    It(fills_every_tile) {
        World world;
        world.generateTerrain(1);
        // getTile returns a valid terrain for every cell (no nulls / no crash).
        Assert::That(world.getTile(0, 0).getMovementCost() > 0, Equals(true));
        Assert::That(world.getTile(WIDTH - 1, HEIGHT - 1).getMovementCost() > 0, Equals(true));
    };

    It(shares_at_most_three_terrain_instances_across_the_grid) {
        World world;
        world.generateTerrain(7);

        std::set<const Terrain *> distinct;
        for (int x = 0; x < WIDTH; x++)
            for (int y = 0; y < HEIGHT; y++)
                distinct.insert(&world.getTile(x, y));

        // 100 tiles, but they point at no more than three shared objects.
        Assert::That(distinct.size() <= 3u, Equals(true));
    };

    It(is_reproducible_for_a_given_seed) {
        World a, b;
        a.generateTerrain(42);
        b.generateTerrain(42);
        bool same = true;
        for (int x = 0; x < WIDTH && same; x++)
            for (int y = 0; y < HEIGHT && same; y++)
                if (a.getTile(x, y).getTexture() != b.getTile(x, y).getTexture())
                    same = false;
        Assert::That(same, Equals(true));
    };
};
