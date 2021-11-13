//
// Created by user on 16.03.2021.
//

#include "ObjTreeQt.h"
#include "../Translator.h"

QVariant TModelTree::data(const QModelIndex &index, int role) const
{
    if(role != Qt::DisplayRole && role != Qt::CheckStateRole) return QVariant();

    auto treeObj = index.isValid() ? IndexTo(index) : this;
    if(role == Qt::CheckStateRole)
        return treeObj->Tag();// ? Qt::SetIsChecked : Qt::Unchecked;
    else
        return TRANSR(treeObj->Name());
}

bool TModelTree::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(role == Qt::CheckStateRole)
    {
        auto treeObj = index.isValid() ? IndexTo(index) : this;
        treeObj->SetTag(value.toInt() ? Qt::Checked : Qt::Unchecked);
        if(index.parent().isValid())//когда меняем флаг обновим все поля
        {
            dataChanged(this->index(0, 0, index.parent()),
                        this->index(rowCount(index.parent()), 0, index.parent()));
        }
        else
            dataChanged(index, index);
    }
    return QAbstractItemModel::setData(index, value, role);
}

TModelTree::TModelTree()
{
    columnTitles = {TRANSR("Data")};
    treeCust.SetShowProperty(TShowProp::None);
    SetCustomClass(&treeCust);
}

void TModelTree::TagChanged(const TPtrObjTree &value)
{
    //сначало установим для всех детей такое же состояние
    if(parentCall == 0)
    {
        childCall++;//режим обновления детей
        auto countChild = value->CountItems();
        for (size_t i = 0; i < countChild; i++)
            value->Item(i)->SetTag(value->Tag());
        childCall--;
    }

    //потом проверим статус всех вышестоящих родителей
    if(childCall == 0 && value->Parent().expired() == false)
    {
        auto par = value->LockParent();
        parentCall++;//режим обнволвения родителей
        par->SetTag(CheckTag(par));
        parentCall--;
    }
    OnTagChanged(value);
}

int TModelTree::CheckTag(const TPtrObjTree &value)
{
    bool isChecked = false;
    bool isUnchecked = false;
    for(int i = 0; i < value->CountItems(false); i++)
        switch (value->Item(i)->Tag())
        {
            case Qt::PartiallyChecked:
                return Qt::PartiallyChecked;
            case Qt::Unchecked:
                if(isChecked)
                    return Qt::PartiallyChecked;
                isUnchecked = true;
                break;
            case Qt::Checked:
                if(isUnchecked)
                    return Qt::PartiallyChecked;
                isChecked = true;
                break;
        }
    return isChecked ? Qt::Checked : Qt::PartiallyChecked;
}

Qt::ItemFlags TModelTree::flags(const QModelIndex &index) const
{
    return TModelEditorSimple::flags(index) | Qt::ItemIsUserCheckable;
}
