/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "gtest/gtest.h"

#include "core/components_ng/test/mock/image_provider/mock_image_loader.h"

#define protected public
#define private public

#include "core/components_ng/image_provider/image_loading_context.h"
#include "core/components_ng/image_provider/image_provider.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Ace::NG {
class ImageProviderTestNg : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void ImageProviderTestNg::SetUpTestCase() {}

void ImageProviderTestNg::TearDownTestCase() {}

void ImageProviderTestNg::SetUp() {}

void ImageProviderTestNg::TearDown()
{
    auto count = loader->DecRefCount();
    EXPECT_EQ(count, 0);
}

/**
 * @tc.name: ImageProviderTestNg001
 * @tc.desc: Test ImageProvider Synchronous CreateImageObj failure
 * @tc.type: FUNC
 */
HWTEST_F(ImageProviderTestNg, ImageProviderTestNg001, TestSize.Level1)
{
    EXPECT_CALL(*loader, LoadDecodedImageData);
    auto src = ImageSourceInfo("file://data/data/com.example.test/res/exampleAlt.jpg");
    auto ctx = AceType::MakeRefPtr<ImageLoadingContext>(src, LoadNotifier(nullptr, nullptr, nullptr), true);
    EXPECT_EQ(ctx->stateManager_->GetCurrentState(), ImageLoadingState::UNLOADED);
    ctx->LoadImageData();

    EXPECT_EQ(ctx->syncLoad_, true);
    EXPECT_EQ(ctx->imageObj_, nullptr);
    EXPECT_EQ(ctx->stateManager_->GetCurrentState(), ImageLoadingState::LOAD_FAIL);
}

/**
 * @tc.name: ImageProviderTestNg002
 * @tc.desc: Test ImageProvider Async CreateImageObj failure
 * @tc.type: FUNC
 */
HWTEST_F(ImageProviderTestNg, ImageProviderTestNg002, TestSize.Level1)
{
    EXPECT_CALL(*loader, LoadDecodedImageData);
    auto src = ImageSourceInfo("file://data/data/com.example.test/res/exampleAlt.jpg");
    auto ctx = AceType::MakeRefPtr<ImageLoadingContext>(src, LoadNotifier(nullptr, nullptr, nullptr));
    EXPECT_EQ(ctx->stateManager_->GetCurrentState(), ImageLoadingState::UNLOADED);
    ctx->LoadImageData();

    // wait for load task to finish
    for (auto& thread : threads) {
        thread.join();
    }
    EXPECT_EQ(ctx->syncLoad_, false);
    EXPECT_EQ(ctx->imageObj_, nullptr);
    EXPECT_EQ(ctx->stateManager_->GetCurrentState(), ImageLoadingState::LOAD_FAIL);
}
} // namespace OHOS::Ace::NG
