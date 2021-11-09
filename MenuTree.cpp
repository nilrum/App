//
// Created by user on 24.09.2019.
//

#include "MenuTree.h"
#include "Property/Algorithms.h"

TMenuItem::TMenuItem(const TString& t, TFunItemCall c, size_t p) noexcept
    :text(t), call(c), priority(p)
{

}

TMenuItem &TMenuItem::Item(size_t index)
{
    if(index < items.size()) return *items[index];
    return Single<TMenuItem>();
}

const TMenuItem &TMenuItem::Item(size_t index) const
{
    if(index < items.size()) return *items[index];
    return Single<TMenuItem>();
}

size_t TMenuItem::CountItems() const
{
    return items.size();
}

const TPtrMenuItem& TMenuItem::Add(const TString &t, TFunItemCall c, size_t p)
{
    return Add(std::make_shared<TMenuItem>(t, c, p));
}

const TPtrMenuItem& TMenuItem::Add(const TMenuItem &item)
{
    return Add(std::make_shared<TMenuItem>(item));
}

const TPtrMenuItem& TMenuItem::Add(const std::shared_ptr<TMenuItem> &item)
{
    for(size_t i = 0; i < items.size(); i++)
        if(item->Priority() > items[i]->Priority())//если добавляемый приоритет больше то вставляем перед текущим
            return *items.insert(items.begin() + i, item);

    items.emplace_back(item);
    return items.back();
}

TPtrMenuItem TMenuItem::Find(const TString &value)
{
    for(const auto& item: items)
        if(item->Text() == value) return item;
    return TPtrMenuItem();
}

TPtrMenuItem TMenuItem::FindAdd(const TString &value)
{
    TPtrMenuItem item = Find(value);
    if(item != nullptr) return item;
    else return Add(value);
}

void TMenuItem::ClearItems()
{
    items.clear();
}

bool TMenuItem::IsEnabled() const
{
    return enabled;
}

void TMenuItem::SetIsEnabled(bool value)
{
    if(value != enabled)
    {
        enabled = value;
        OnEnabled(enabled);
    }
}

TMenuItem::TMenuItem(const TMenuItem &oth):text(oth.text), call(oth.call), indexImg(oth.indexImg),
    isToolBar(oth.isToolBar), priority(oth.priority), shortcut(oth.shortcut),
    items(oth.items), enabled(oth.enabled)
{

}

TPtrMenuItem TMenuItem::FindPath(const TString &value)
{
    TVecString paths = Split(value, '|');
    if(paths.empty()) return TPtrMenuItem();
    TPtrMenuItem res = shared_from_this();
    for(int i = 0; i < paths.size(); i++)
    {
        res = res->Find(paths[i]);
        if(res == nullptr) break;
    }
    return res;
}

TFunCheckEnable TMenuItem::CheckEnable() const
{
    return checkEnable;
}

void TMenuItem::SetCheckEnable(const TFunCheckEnable &value)
{
    checkEnable = value;
}

void TMenuItem::RunCheckEnable()
{
    if(checkEnable)
        SetIsEnabled(checkEnable());
}



//-------------------------------TMenuTree------------------------------------------------------------------------------
TPtrMenuTree TMenuTree::Create()
{
    return TPtrMenuTree(new TMenuTree());
}

const TPtrMenuItem& TMenuTree::AddItem(const TString text, TFunItemCall call, size_t priority)
{
    auto splits = Split(text, '|');
    TPtrMenuItem parent = shared_from_this();

    for(size_t i = 0; i < splits.size() - 1; i++)
        parent = parent->FindAdd(splits[i]);

    return parent->Add(splits.back(), call, priority);
}

