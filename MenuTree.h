//
// Created by user on 24.09.2019.
//

#ifndef TESTAPP_MENUTREE_H
#define TESTAPP_MENUTREE_H

#include <functional>
#include "Types.h"
#include "sigslot/signal.hpp"

using TFunItemCall = std::function<void()>;
using TFunCheckEnable = std::function<bool()>;

const size_t defPriority = 100;
const size_t upPriority = 101;
const size_t lowPriority = 99;

class TMenuItem;
using TPtrMenuItem = std::shared_ptr<TMenuItem>;
using TWPtrMenuItem = std::weak_ptr<TMenuItem>;

class TMenuItem : public std::enable_shared_from_this<TMenuItem>{
public:
    TMenuItem() = default;
    TMenuItem(const TString& t, TFunItemCall c = TFunItemCall(), size_t p = defPriority) noexcept;
    TMenuItem(const TMenuItem& oth);

    inline const TString& Text() const                  { return text; }                    //текст пункта меню
    inline TMenuItem& SetText( const TString& value)    { text = value; return *this; }

    inline bool IsValid() const                         { return text.empty() == false;}

    TFunItemCall Call()                                 { return checkable ? call : [this](){ SetIsChecked(!checked); call(); }; } //функция вызова пункта меню
    inline TMenuItem& SetCall(const TFunItemCall& value){ call = value; return *this; }

    inline size_t Priority() const                      { return priority; }

    inline int IndexImg() const                         { return indexImg; }
    inline TMenuItem& SetIndexImg(int index)               { indexImg = index; return *this; }

    inline bool IsToolBar() const                       { return isToolBar; }
    inline TMenuItem& SetIsToolBar(bool value)             { isToolBar = value; return *this; }

    inline bool IsVisible() const                       { return visible; }
    inline TMenuItem& SetIsVisible(bool value)          { visible = value; return *this; }

    inline TString Shortcut() const                     { return shortcut; }
    inline TMenuItem& SetShortcut(const TString& value) { shortcut = value; return *this;}

    inline bool IsChecked() const                       { return checked; }
    inline TMenuItem& SetIsChecked(bool value)          { checked = value; OnChecked(value); return *this;}
    sigslot::signal<bool> OnChecked;

    inline bool IsCheckable() const                     { return checkable; }
    inline TMenuItem& SetIsCheckable(bool value)        { checkable = value; return *this; }

    bool IsEnabled() const;                           //Доступен ли пункт меню для редактирования
    void SetIsEnabled(bool value);                       //Установить доступность пункта меню для редактирования
    sigslot::signal<bool> OnEnabled;                //Событие поменялась доступность пункта меню

    TFunCheckEnable CheckEnable() const;                //получение и установка функции проверки на доступность пункта меню
    void SetCheckEnable(const TFunCheckEnable& value);
    void RunCheckEnable();                              //проверить доступность меню функцией проверки

    TMenuItem& Item(size_t index);
    const TMenuItem& Item(size_t index) const;
    const TPtrMenuItem& ItemPtr(size_t index) const;
    size_t CountItems() const;

    void ClearItems();

    const TPtrMenuItem& Add(const TString& t, TFunItemCall c = TFunItemCall(), size_t p = defPriority);
    const TPtrMenuItem& Add(const TMenuItem& item);
    const TPtrMenuItem& Add(const TPtrMenuItem& item);

    TPtrMenuItem Find(const TString& value);        //ищет элемент по имени в items
    TPtrMenuItem FindPath(const TString& value);    //ищет полный путь к элементу
    TPtrMenuItem FindAdd(const TString& value);     //ищет элемент по имени в items если нет, то создает
private:
    TString text;
    TFunItemCall call;
    int indexImg = -1;
    bool isToolBar = false;
    size_t priority = defPriority;
    TString shortcut;
    std::vector<TPtrMenuItem> items;
    bool enabled = true;
    bool visible = true;
    bool checked = false;
    bool checkable = false;
    TFunCheckEnable checkEnable;
};

class TMenuTree;
using TPtrMenuTree = std::shared_ptr<TMenuTree>;

class TMenuTree : public TMenuItem{
public:
    static TPtrMenuTree Create();
    const TPtrMenuItem& AddItem(const TString text, TFunItemCall call = TFunItemCall(), size_t priority = defPriority);
    sigslot::signal<> OnChange;
private:
    TMenuTree(){};
};



#endif //TESTAPP_MENUTREE_H
