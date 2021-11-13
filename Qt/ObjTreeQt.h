//
// Created by user on 16.03.2021.
//

#ifndef ATLAS_OBJTREEQT_H
#define ATLAS_OBJTREEQT_H

#include "EditorWidgetQt.h"

class TModelTree : public TModelEditorSimple{
public:
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    void TagChanged(const TPtrObjTree& value) override;
    sigslot::signal<const TPtrObjTree&> OnTagChanged;
protected:
    TModelTree();
    friend TObjTree;
    TCustClass treeCust;
    int childCall = 0;
    int parentCall = 0;
    int CheckTag(const TPtrObjTree& value);
};
#endif //ATLAS_OBJTREEQT_H
