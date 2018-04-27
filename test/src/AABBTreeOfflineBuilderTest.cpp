/*
 Copyright (C) 2010-2017 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include "Vec.h"
#include "BBox.h"
#include "Ray.h"
#include "AABBTree.h"

#include <random>
#include <algorithm>

static constexpr size_t NumObjects = 64000;

// 10-bit per component morton code AABB tree builder

/**
 * Takes the lower 10 bits of `input` and inserts a pair of "0" bits between each input bit,
 * producing 28 bits of output. e.g. 0bXYZ -> 0bX00Y00Z
 */
static uint32_t expandFrom10Bit(uint32_t input) {
    uint32_t out = 0;
    for (size_t i = 0; i < 10; ++i) {
        out |= ((input >> i) & 1U) << (i * 3U);
    }
    return out;
}

/**
 * Inverse of Expand. Takes the lower 28 bits of `input` and produces 10 bits of output,
 * discarding the top 2 bits of every 3 bits)
 * e.g. 0bX00Y00Z -> 0bXYZ
 */
static uint32_t contractTo10Bit(uint32_t input) {
    uint32_t out = 0;
    for (size_t i = 0; i < 10; ++i) {
        out |= ((input >> (i * 3U)) & 1U) << i;
    }
    return out;
}

/**
 * Takes three 10-bit integer coordinates (0 <= coord < 1024) and
 * packs them into a 30-bit Morton code.
 */
static uint32_t mortonEncode(uint32_t x, uint32_t y, uint32_t z) {
    return (expandFrom10Bit(x) << 2) | (expandFrom10Bit(y) << 1) | expandFrom10Bit(z);
}

/**
 * Inverse of MortonEncode.
 */
static std::tuple<uint32_t, uint32_t, uint32_t> mortonDecode(uint32_t code) {
    return {contractTo10Bit(code >> 2), contractTo10Bit(code >> 1), contractTo10Bit(code)};
}

TEST(AABBTreeOfflineBuilderTest, mortonCodes) {
    // test low values
    for (uint32_t x = 0; x < 8; ++x) {
        for (uint32_t y = 0; y < 8; ++y) {
            for (uint32_t z = 0; z < 8; ++z) {
                EXPECT_EQ(std::make_tuple(x,y,z), mortonDecode(mortonEncode(x, y, z)));
            }
        }
    }

    // test high values
    for (uint32_t x = 1020; x < 1024; ++x) {
        for (uint32_t y = 1020; y < 1024; ++y) {
            for (uint32_t z = 1020; z < 1024; ++z) {
                EXPECT_EQ(std::make_tuple(x,y,z), mortonDecode(mortonEncode(x, y, z)));
            }
        }
    }

    // test an expected encoding
    EXPECT_EQ((0b1001 << 2) | (0b0000 << 1) | 0b1000,
              mortonEncode(0b11, 0b00, 0b10));

    EXPECT_EQ((0b1001 << 2) | (0b0001 << 1) | 0b0001,
              mortonEncode(0b11, 0b01, 0b01));
}

/**
 * Returns a vector of points with pseudo-random integer coordinates in [-4096, 4096)
 *
 * @return the points
 */
static std::vector<Vec<double,3>> randomVecs() {
    std::mt19937 rng(1);

    std::vector<Vec<double,3>> result;

    for (size_t i = 0; i < 64000; ++i) {
        const int x = static_cast<int>(rng() % 8192) - 4096;
        const int y = static_cast<int>(rng() % 8192) - 4096;
        const int z = static_cast<int>(rng() % 8192) - 4096;
        const Vec<double,3> randVec(x, y, z);

        result.push_back(randVec);
    }
    return result;
}

static uint32_t mortonCodeForVec(const Vec3d& vec, const BBox3d& worldBounds) {
    uint32_t integerPoint[3];
    // maps the point's position inside worldBounds to a 10-bit unsigned integer
    for (size_t i = 0; i < 3; ++i) {
        const double frac = Math::clamp((vec[i] - worldBounds.min[i]) / (worldBounds.size()[i]));
        integerPoint[i] = static_cast<uint32_t>(1023 * frac);
    }
    return mortonEncode(integerPoint[0], integerPoint[1], integerPoint[2]);
}

TEST(AABBTreeOfflineBuilderTest, buildVecs) {
    EXPECT_EQ(NumObjects, randomVecs().size());
}

TEST(AABBTreeOfflineBuilderTest, buildVecsAndGetCodes) {
    const BBox3d worldBounds(8192.0);
    const auto points = randomVecs();
    std::vector<uint32_t> codes;
    codes.reserve(points.size());

    for (const auto& vec : points) {
        codes.push_back(mortonCodeForVec(vec, worldBounds));
    }

    std::sort(codes.begin(), codes.end());

    EXPECT_EQ(NumObjects, codes.size());
}
