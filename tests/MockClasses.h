//
// Created by user on 11.01.2021.
//

#ifndef ATHENA_MOCKCLASSES_H
#define ATHENA_MOCKCLASSES_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "../App.h"

using namespace testing;
class TAppMock : public TApp{
public:
    TAppMock(const TString& nameApp): TApp(nameApp){}
    MOCK_METHOD(TResult, ShowMessage, (const TString& message, bool isQuestion, bool isCancel), (override));
    MOCK_METHOD(TResult, InputItems, (const TString& message, const TVecString& items, TString& value, int& index, bool isEdit), (override));
    MOCK_METHOD(TResult, SelectFile, (const TString &filter, const TString &ext, TString &res), (override));
};

#endif //ATHENA_MOCKCLASSES_H
