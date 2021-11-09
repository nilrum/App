//
// Created by user on 15.04.2020.
//

#ifndef NEO_TRANSLATOR_H
#define NEO_TRANSLATOR_H

#include "Property/PropertyClass.h"

ENUM(TLangEnum, lgEng, lgRus);

class TTransItem : public TPropertyClass{
public:
    TTransItem(){};
    TTransItem(const TString& n, const TString r, const TString& e):
        rus(r), eng(e){ name = n; }

    inline TString Text(TLangEnum value)
    {
        if(value == lgEng)
            return IfEmpty(eng);
        else
            return IfEmpty(rus);
    }

    inline TString IfEmpty(const TString& value)
    {
        if(value.empty()) return name;
        return value;
    }

    PROPERTIES(TTransItem, TPropertyClass,
            PROPERTY(TString, rus, Rus, SetRus);
            PROPERTY(TString, eng, Eng, SetEng);
            )
    PROPERTY_FUN(TString, rus, Rus, SetRus);
    PROPERTY_FUN(TString, eng, Eng, SetEng);
private:
    TString rus;
    TString eng;
};

using TPtrTransItem = std::shared_ptr<TTransItem>;

class TTranslator : public TPropertyClass {
public:
    TTranslator();
    ~TTranslator();

    void LoadFromFiles(const TString& value) { SetLoadingFiles(value); };
    void SaveToFiles(const TString& value = TString());

    TString Trans(const TString& id) const;
    TString Trans(const TString& id, TLangEnum toLang) const;
    TString Consts(const TString& id) const;

    PROPERTIES(TTranslator, TPropertyClass,
       PROPERTY(TLangEnum, lang, Lang, SetLang);
       PROPERTY(TString, pathTrans, PathTrans, SetPathTrans);
       PROPERTY(TString, loadingFiles, LoadingFiles, SetLoadingFiles);
       PROPERTY(size_t, curAddIndex, CurAddIndex, SetCurAddIndex).NoSerialization();
       PROPERTY_ARRAY(TTransItem, items, CountItem, Item, AddItem, DelItem).NoSerialization();
       PROPERTY(bool, saveItems, SaveItems, SetSaveItems).NoSerialization();
    )
    PROPERTY_FUN(TLangEnum, lang, Lang, SetLang);
    PROPERTY_FUN(TString, pathTrans, PathTrans, SetPathTrans);
    PROPERTY_FUN(size_t, curAddIndex, CurAddIndex, SetCurAddIndex);
    PROPERTY_ARRAY_READ_FUN(TPtrTransItem, items, CountItem, Item);
    PROPERTY_ARRAY_FUN_CHG(TPtrTransItem, consts, CountConsts, Const, AddConst, DelConst);

    TPtrTransItem& AddItem(const TPtrTransItem& value);
    void DelItem(const TPtrTransItem& value);

    STATIC_ARG(TTranslator*, Singl, nullptr);
private:
    TLangEnum lang = lgEng;
    TString pathTrans;
    size_t curAddIndex = 0;
    using TVecTransItems = std::vector<TPtrTransItem>;
    TVecTransItems items;
    TVecString loadingFiles;
    std::vector<TVecTransItems> vecItems;
    TVecTransItems consts;
    TString LoadingFiles() const;
    void SetLoadingFiles(const TString& value);
    bool SaveItems() const { return false; }
    void SetSaveItems(bool value);

};

using TPtrTranslator = std::shared_ptr<TTranslator>;

#define TRANS(VALUE) (TTranslator::Singl()->Trans(VALUE))
#define TRANSL(VALUE, LANG) (TTranslator::Singl()->Trans(VALUE, LANG))
#define TRANSR(VALUE) (STR(TRANS(VALUE)))
#define APPCONST(VALUE) VALUE
//(TTranslator::Singl()->Consts(VALUE))


#endif //NEO_TRANSLATOR_H
