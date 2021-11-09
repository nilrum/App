//
// Created by user on 15.04.2020.
//

#include "Translator.h"
#include "Property/Serialization.h"
#include "Property/Algorithms.h"
#include "FileSystem/FileSystem.h"

INIT_PROPERTYS(TTranslator)
INIT_PROPERTYS(TTransItem)

TTranslator::TTranslator()
{
    name = "Translator";
    pathTrans = fs::current_path().string();
    if(Singl() == nullptr) Singl() = this;
}

TTranslator::~TTranslator()
{
    if(Singl() == this) Singl() = nullptr;
}

void TTranslator::SaveToFiles(const TString &value)
{
    if(value.empty() == false)
    {
        TVecString buf = Split(value, '|');
        if(buf.size() >= loadingFiles.size())
            loadingFiles = buf;
        else
            for(size_t i = 0; i < buf.size(); i++)
                loadingFiles[i] = buf[i];
    }
    SetSaveItems(true);
}

TString TTranslator::LoadingFiles() const
{
    if(loadingFiles.empty()) return TString();
    TString rez = loadingFiles[0];
    for(size_t i = 1; i < loadingFiles.size(); i++)
        rez = rez + "|" + loadingFiles[i];
    return rez;
}

void TTranslator::SetLoadingFiles(const TString &value)
{
    loadingFiles = Split(value, '|');
    vecItems.clear();
    items.clear();
    if(loadingFiles.empty()) return;
    if(loadingFiles.size() == 1)
    {
        TSerialization().LoadPropFromFileName(pathTrans + "/" + loadingFiles[0], this, "items");
    }
    else
    {
        vecItems.resize(loadingFiles.size());
        for (size_t i = 0; i < loadingFiles.size(); i++)
        {

            TPropInfo info("items", "", false);//делаем новое проперти для загрузки в массив
            info.Get([i, this](const TPropertyClass *) { return TVariable(vecItems[i].size()); });

            info.AddArray([i, this](TPropertyClass *, const TVariable &value) {
                vecItems[i].push_back(VariableToPropertyClassImpl<TPtrTransItem>(value));
            });

            TSerialization().LoadPropFromFile(pathTrans + "/" + loadingFiles[i], this, info);

            items.insert(items.end(), vecItems[i].begin(),
                         vecItems[i].end());//копируем загруженные данные в общий список
        }
    }
    //items.insert(items.end(), consts.begin(), consts.end());
}

void TTranslator::SetSaveItems(bool value)
{
    if(value = false || loadingFiles.empty()) return;
    if(loadingFiles.size() == 1)
    {
        TSerialization().SavePropToFileName(pathTrans + "/" + loadingFiles[0], this, "items");
        return;
    }

    for(size_t i = 0; i < loadingFiles.size(); i++)
    {
        TPropInfo info("items", "", false);//делаем новое проперти для загрузки в массив
        auto getCount = [i, this](const TPropertyClass*)
                 { return TVariable(vecItems[i].size()); };

        auto getItem = [i, this](const TPropertyClass *, int index)
                      { return PropertyClassToVariable(vecItems[i][index]); };
        info.GetArray(getCount, getItem);

        TSerialization().SavePropToFile(pathTrans + "/" + loadingFiles[i], this, info);
    }
}

TPtrTransItem &TTranslator::AddItem(const TPtrTransItem &value)
{
    if(vecItems.size())
    {
        if(curAddIndex >= vecItems.size()) curAddIndex = vecItems.size() - 1;
        vecItems[curAddIndex].push_back(value);
    }
    items.push_back(value);
    OnChanged();
    return items.back();
}

void TTranslator::DelItem(const TPtrTransItem &value)
{
    for(size_t i = 0; i < vecItems.size(); i++)
        RemoveVal(vecItems[i], value);
    RemoveVal(items, value);
    OnChanged();
}

TString TTranslator::Trans(const TString &id) const
{
    for(const auto& it : items)
        if(it->Name() == id) return it->Text(lang);
    return id;
}

TString TTranslator::Consts(const TString &id) const
{
    for(const auto& cnst : consts)
        if(cnst->Name() == id) return cnst->Text(lang);
    return id;
}

TString TTranslator::Trans(const TString &id, TLangEnum toLang) const
{
    for(const auto& it : items)
        if(it->Name() == id) return it->Text(toLang);
    return id;
}
