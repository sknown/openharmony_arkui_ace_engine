/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FOUNDATION_ACE_TEST_UNITTEST_CORE_PATTERN_GRID_IRREGULAR_MATRICES_H
#define FOUNDATION_ACE_TEST_UNITTEST_CORE_PATTERN_GRID_IRREGULAR_MATRICES_H

#include "core/components_ng/pattern/grid/grid_layout_info.h"
#include "core/components_ng/pattern/grid/grid_layout_options.h"

namespace OHOS::Ace::NG {
GridLayoutOptions GetOptionDemo1();
const decltype(GridLayoutInfo::gridMatrix_) MATRIX_DEMO_1 = {
    { 0, { { 0, 0 }, { 1, 0 }, { 2, 1 } } },     // 0 | 0 | 1
    { 1, { { 0, 2 }, { 1, -2 }, { 2, -1 } } },   // 2 | 2 | 1
    { 2, { { 0, 3 }, { 1, 4 }, { 2, -1 } } },    // 3 | 4 | 1
    { 3, { { 0, 5 }, { 1, 6 }, { 2, 7 } } },     // 5 | 6 | 7
    { 4, { { 0, -5 }, { 1, 8 }, { 2, -7 } } },   // 5 | 8 | 7
    { 5, { { 0, -5 }, { 1, 9 }, { 2, 10 } } },   // 5 | 9 | 10
    { 6, { { 0, -5 }, { 1, -9 }, { 2, -10 } } }, // 5 | 9 | 10
};

GridLayoutOptions GetOptionDemo2();
const decltype(GridLayoutInfo::gridMatrix_) MATRIX_DEMO_2 = {
    { 0, { { 0, 0 }, { 1, 0 }, { 2, 1 } } },   // 0 | 0 | 1
    { 1, { { 0, 0 }, { 1, 0 }, { 2, -1 } } },  // 0 | 0 | 1
    { 2, { { 0, 2 }, { 1, 3 }, { 2, -1 } } },  // 2 | 3 | 1
    { 3, { { 0, 4 }, { 1, -4 }, { 2, -4 } } }, // 4 | 4 | 4
    { 4, { { 0, 5 }, { 1, 6 }, { 2, 7 } } },   // 5 | 6 | 7
    { 5, { { 0, -5 }, { 2, -7 } } },           // 5 | x | 7
};

const decltype(GridLayoutInfo::gridMatrix_) MATRIX_DEMO_3 = {
    { 0, { { 0, 0 }, { 1, 1 } } },
    { 1, { { 0, 2 }, { 1, -2 } } },
    { 2, { { 0, 3 }, { 1, 4 } } },
    { 3, { { 0, 5 }, { 1, -5 } } },
    { 4, { { 0, 6 }, { 1, 7 } } },
    { 5, { { 0, 8 }, { 1, 9 } } },
    { 6, { { 0, 10 }, { 1, -10 } } },
    { 7, { { 0, 11 } } },
};

const decltype(GridLayoutInfo::gridMatrix_) MATRIX_DEMO_4 = {
    { 0, { { 1, 0 }, { 2, 0 } } },
    { 1, { { 0, 1 }, { 1, 0 }, { 2, 0 } } },
    { 2, { { 0, 2 }, { 1, 3 }, { 2, 4 } } },
    { 3, { { 0, 5 }, { 1, 6 }, { 2, 7 } } },
    { 4, { { 0, 8 }, { 1, 8 }, { 1, 7 } } },
    { 5, { { 0, 8 }, { 1, 8 }, { 2, 9 } } },
    { 6, { { 0, 8 }, { 1, 8 }, { 2, 10 } } },
    { 7, { { 0, 8 }, { 1, 8 }, { 2, 11 } } },
    { 8, { { 0, 8 }, { 1, 8 }, { 2, 12 } } },
    { 9, { { 0, 8 }, { 1, 8 }, { 2, 13 } } },
    { 10, { { 0, 8 }, { 1, 8 }, { 2, 14 } } },
    { 11, { { 0, 15 }, { 1, 16 }, { 2, 17 } } },
    { 12, { { 0, 18 }, { 1, 19 } } },
    { 13, { { 0, 20 }, { 1, 20 }, { 2, 21 } } },
    { 14, { { 0, 20 }, { 1, 20 }, { 2, 22 } } },
    { 15, { { 0, 23 }, { 1, 24 }, { 2, 25 } } },
    { 16, { { 0, 26 }, { 1, 27 }, { 2, 28 } } },
    { 17, { { 0, 29 }, { 1, 30 } } },
    { 18, { { 0, 31 }, { 1, 31 } } },
    { 19, { { 0, 31 }, { 1, 31 } } },
};

GridLayoutOptions GetOptionDemo5();
const decltype(GridLayoutInfo::gridMatrix_) MATRIX_DEMO_5 = {
    { 0, { { 0, 0 }, { 1, 1 } } },
    { 1, { { 0, 2 }, { 1, -2 } } },
    { 2, { { 0, 3 }, { 1, -3 } } },
    { 3, { { 0, -3 }, { 1, -3 } } },
    { 4, { { 0, -3 }, { 1, -3 } } },
    { 5, { { 0, -3 }, { 1, -3 } } },
    { 6, { { 0, 4 }, { 1, -4 } } },
    { 7, { { 0, 5 }, { 1, 6 } } },
    { 8, { { 0, -5 }, { 1, 7 } } },
    { 9, { { 0, 8 }, { 1, -8 } } },
    { 10, { { 0, -8 }, { 1, -8 } } },
    { 11, { { 0, 9 }, { 1, 10 } } },
};

GridLayoutOptions GetOptionDemo6();
const decltype(GridLayoutInfo::gridMatrix_) MATRIX_DEMO_6 = {
    { 0, { { 0, 0 }, { 1, 1 } } },
    { 1, { { 0, 2 }, { 1, -2 } } },
    { 2, { { 0, 3 }, { 1, 4 } } },
    { 3, { { 0, 5 }, { 1, -5 } } },
    { 4, { { 0, 6 }, { 1, 7 } } },
    { 5, { { 0, 8 }, { 1, 9 } } },
    { 6, { { 0, 10 }, { 1, -10 } } },
    { 7, { { 0, 11 } } },
};

const decltype(GridLayoutInfo::gridMatrix_) MATRIX_DEMO_6_VARIATION = {
    { 0, { { 0, 0 }, { 1, 1 } } },
    { 1, { { 0, 2 }, { 1, -2 }, { 2, 3 } } },
    { 2, { { 0, 4 }, { 1, 5 }, { 2, -5 } } },
    { 3, { { 0, 6 }, { 1, 7 }, { 2, 8 } } },
    { 4, { { 0, 9 }, { 1, 10 }, { 2, -10 } } },
    { 5, { { 0, 11 } } },
};

const decltype(GridLayoutInfo::gridMatrix_) MATRIX_DEMO_7 = {
    { 0, { { 0, 0 }, { 1, 0 }, { 2, 1 } } },
    { 1, { { 0, 2 }, { 1, 3 }, { 2, 4 } } },
    { 2, { { 0, 5 }, { 1, 6 }, { 2, -4 } } },
    { 3, { { 0, 7 }, { 1, -6 }, { 2, 9 } } },
};

GridLayoutOptions GetOptionDemo8();
const decltype(GridLayoutInfo::gridMatrix_) MATRIX_DEMO_8 = {
    { 0, { { 0, 0 }, { 1, 0 }, { 2, 1 } } },    // 0 | 0 | 1
    { 1, { { 0, 2 }, { 1, -2 }, { 2, -2 } } },  // 2 | 2 | 2
    { 2, { { 0, -2 }, { 1, -2 }, { 2, -2 } } }, // 2 | 2 | 2
    { 3, { { 0, 3 }, { 1, 4 }, { 2, 5 } } },    // 3 | 4 | 5
    { 4, { { 0, 6 }, { 1, -6 }, { 2, -5 } } },  // 6 | 6 | 5
    { 5, { { 2, -5 } } }                        // x | x | 5
};

GridLayoutOptions GetOptionDemo9();
const decltype(GridLayoutInfo::gridMatrix_) MATRIX_DEMO_9 = {
    { 0, { { 0, 0 }, { 1, 1 }, { 2, 2 } } },    // 0 | 1 | 2
    { 1, { { 0, 0 }, { 1, 3 }, { 2, -3 } } },   // 0 | 3 | 3
    { 2, { { 0, 4 }, { 1, -3 }, { 2, -3 } } },  // 4 | 3 | 3
    { 3, { { 0, 5 } } },                        // 5 | x | x
    { 4, { { 0, 6 }, { 1, -6 }, { 2, -6 } } },  // 6 | 6 | 6
    { 5, { { 0, -6 }, { 1, -6 }, { 2, -6 } } }, // 6 | 6 | 6
    { 6, { { 0, 7 }, { 1, 8 }, { 2, 9 } } }     // 7 | 8 | 9
};

GridLayoutOptions GetOptionDemo10();
const decltype(GridLayoutInfo::gridMatrix_) MATRIX_DEMO_10 = {
    { 0, { { 0, 0 }, { 1, 1 }, { 2, -1 } } },  // 0 | 1 | 1
    { 1, { { 0, 2 }, { 1, 3 }, { 2, -3 } } },  // 2 | 3 | 3
    { 2, { { 0, 4 }, { 1, -3 }, { 2, -3 } } }, // 4 | 3 | 3
    { 3, { { 0, 5 }, { 1, 6 } } },             // 5 | 6 | x
    { 4, { { 1, -6 } } },                      // x | 6 | x
    { 5, { { 0, 7 }, { 1, -7 } } }             // 7 | 7 | x
};

GridLayoutOptions GetOptionDemo11();
const decltype(GridLayoutInfo::gridMatrix_) MATRIX_DEMO_11 = {
    { 0, { { 0, 0 }, { 1, 0 }, { 2, 1 } } },    // 0 | 0 | 1
    { 1, { { 0, 0 }, { 1, 0 }, { 2, 2 } } },    // 0 | 0 | 2
    { 2, { { 0, 3 }, { 1, 4 }, { 2, 5 } } },    // 3 | 4 | 5
    { 3, { { 0, -3 }, { 1, -4 } } },            // 3 | 4 | x
    { 4, { { 0, 6 }, { 1, -6 }, { 2, -6 } } },  // 6 | 6 | 6
    { 5, { { 0, -6 }, { 1, -6 }, { 2, -6 } } }, // 6 | 6 | 6
    { 6, { { 0, 7 }, { 1, 8 }, { 2, 9 } } },    // 7 | 8 | 9
};

} // namespace OHOS::Ace::NG
#endif
