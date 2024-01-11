/*
 * Copyright (C) 2023 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <BroadcastRadio.h>
#include <VirtualRadio.h>
#include <broadcastradio-utils-aidl/Utils.h>

#include <gtest/gtest.h>

namespace aidl::android::hardware::broadcastradio {

namespace {
using ::std::vector;

constexpr uint32_t kAmFreq1 = 560u;
constexpr uint32_t kAmFreq2 = 680u;
constexpr uint32_t kAmHdFreq = 1170u;
constexpr uint64_t kAmHdSid = 0xB0000001u;
constexpr uint32_t kFmFreq1 = 94900u;
constexpr uint64_t kFmHdSid1 = 0xA0000001u;
constexpr uint64_t kFmHdSid2 = 0xA0000002u;
constexpr uint32_t kFmHdFreq1 = 98500u;
constexpr uint32_t kFmHdSubChannel0 = 0u;
constexpr uint32_t kFmHdSubChannel1 = 1u;
constexpr uint32_t kFmFreq2 = 99100u;
constexpr uint32_t kFmHdFreq2 = 101100u;

const ProgramSelector kAmSel1 = utils::makeSelectorAmfm(kAmFreq1);
const ProgramSelector kAmSel2 = utils::makeSelectorAmfm(kAmFreq2);
const ProgramSelector kAmHdSel = utils::makeSelectorHd(kAmHdSid, kFmHdSubChannel0, kAmHdFreq);
const ProgramSelector kFmSel1 = utils::makeSelectorAmfm(kFmFreq1);
const ProgramSelector kFmSel2 = utils::makeSelectorAmfm(kFmFreq2);
const ProgramSelector kFmHdFreq1Sel1 =
        utils::makeSelectorHd(kFmHdSid1, kFmHdSubChannel0, kFmHdFreq1);
const ProgramSelector kFmHdFreq1Sel2 =
        utils::makeSelectorHd(kFmHdSid1, kFmHdSubChannel1, kFmHdFreq1);
const ProgramSelector kFmHdFreq2Sel1 =
        utils::makeSelectorHd(kFmHdSid2, kFmHdSubChannel0, kFmHdFreq2);
const ProgramSelector kFmHdFreq2Sel2 =
        utils::makeSelectorHd(kFmHdSid2, kFmHdSubChannel1, kFmHdFreq2);

const VirtualRadio& getAmFmMockTestRadio() {
    static VirtualRadio amFmRadioMockTestRadio(
            "AM/FM radio mock for test",
            {
                    {kAmSel1, "ProgramAm1", "ArtistAm1", "TitleAm1"},
                    {kAmSel2, "ProgramAm2", "ArtistAm2", "TitleAm2"},
                    {kFmSel1, "ProgramFm1", "ArtistFm1", "TitleFm1"},
                    {kFmSel2, "ProgramFm2", "ArtistFm2", "TitleFm2"},
                    {kAmHdSel, "ProgramAmHd1", "ArtistAmHd1", "TitleAmHd1"},
                    {kFmHdFreq1Sel1, "ProgramFmHd1", "ArtistFmHd1", "TitleFmHd1"},
                    {kFmHdFreq1Sel2, "ProgramFmHd2", "ArtistFmHd2", "TitleFmHd2"},
                    {kFmHdFreq2Sel1, "ProgramFmHd3", "ArtistFmHd3", "TitleFmHd3"},
                    {kFmHdFreq2Sel2, "ProgramFmHd4", "ArtistFmHd4", "TitleFmHd4"},
            });
    return amFmRadioMockTestRadio;
}

}  // namespace

class DefaultBroadcastRadioHalTest : public testing::Test {
  public:
    void SetUp() override {
        const VirtualRadio& amFmRadioMockTest = getAmFmMockTestRadio();
        mBroadcastRadioHal = ::ndk::SharedRefBase::make<BroadcastRadio>(amFmRadioMockTest);
    }
    std::shared_ptr<BroadcastRadio> mBroadcastRadioHal;
};

TEST_F(DefaultBroadcastRadioHalTest, GetAmFmRegionConfig) {
    AmFmRegionConfig config;

    auto halResult = mBroadcastRadioHal->getAmFmRegionConfig(/* full= */ false, &config);

    ASSERT_TRUE(halResult.isOk());
    EXPECT_EQ(config.fmDeemphasis, AmFmRegionConfig::DEEMPHASIS_D50);
    EXPECT_EQ(config.fmRds, AmFmRegionConfig::RDS);
}

TEST_F(DefaultBroadcastRadioHalTest, GetAmFmRegionConfigWithFullBand) {
    AmFmRegionConfig config;

    auto halResult = mBroadcastRadioHal->getAmFmRegionConfig(/* full= */ true, &config);

    ASSERT_TRUE(halResult.isOk());
    EXPECT_EQ(config.fmDeemphasis,
              AmFmRegionConfig::DEEMPHASIS_D50 | AmFmRegionConfig::DEEMPHASIS_D75);
    EXPECT_EQ(config.fmRds, AmFmRegionConfig::RDS | AmFmRegionConfig::RBDS);
}

TEST_F(DefaultBroadcastRadioHalTest, GetDabRegionConfig) {
    vector<DabTableEntry> config;

    auto halResult = mBroadcastRadioHal->getDabRegionConfig(&config);

    ASSERT_TRUE(halResult.isOk());
    ASSERT_FALSE(config.empty());
}

TEST_F(DefaultBroadcastRadioHalTest, GetImage) {
    vector<uint8_t> img;

    auto halResult = mBroadcastRadioHal->getImage(BroadcastRadio::INVALID_IMAGE, &img);

    ASSERT_TRUE(halResult.isOk());
    ASSERT_TRUE(img.empty());
}

TEST_F(DefaultBroadcastRadioHalTest, GetProperties) {
    vector<VirtualProgram> mockPrograms = getAmFmMockTestRadio().getProgramList();
    Properties prop;

    auto halResult = mBroadcastRadioHal->getProperties(&prop);

    ASSERT_TRUE(halResult.isOk());
    ASSERT_FALSE(prop.supportedIdentifierTypes.empty());
    std::unordered_set<IdentifierType> supportedTypeSet;
    for (const auto& supportedType : prop.supportedIdentifierTypes) {
        supportedTypeSet.insert(supportedType);
    }
    for (const auto& program : mockPrograms) {
        EXPECT_NE(supportedTypeSet.find(program.selector.primaryId.type), supportedTypeSet.end());
    }
}

}  // namespace aidl::android::hardware::broadcastradio