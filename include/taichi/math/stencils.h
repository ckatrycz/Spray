/*******************************************************************************
    Copyright (c) The Taichi Authors (2016- ). All Rights Reserved.
    The use of this software is governed by the LICENSE file.
*******************************************************************************/

#pragma once

#include <taichi/math/math.h>
#include <taichi/math/math.h>
#include <vector>

TC_NAMESPACE_BEGIN

/*
static const Vector3i neighbour6_3d[]{Vector3i(0, 0, 1), Vector3i(0, 0, -1),
                                      Vector3i(0, 1, 0), Vector3i(0, -1, 0),
                                      Vector3i(1, 0, 0), Vector3i(-1, 0, 0)};

static const Vector2i neighbour4_2d[]{
    Vector2i(1, 0), Vector2i(-1, 0), Vector2i(0, 1), Vector2i(0, -1),
};

static const Vector2i neighbour4[]{Vector2i(0, 1), Vector2i(0, -1),
                                   Vector2i(-1, 0), Vector2i(1, 0)};
#define ROW(x) Vector2i((x), -1), Vector2i(x, 0), Vector2i(x, 1), Vector2i(x, 2)
static const Vector2i rasterize_support2[]{ROW(-1), ROW(0), ROW(1), ROW(2)};
#undef ROW
*/

TC_NAMESPACE_END
