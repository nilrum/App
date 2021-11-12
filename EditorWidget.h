//
// Created by user on 11.01.2021.
//

#ifndef ATHENA_EDITORWIDGET_H
#define ATHENA_EDITORWIDGET_H

#include "App.h"
#include "Property/PropertyEditor.h"

using TCreateEditorObj = std::function<void(const TPtrObjTree&)>;

class TEditorWidget : public TWidget, public TPropertyEditor{
public:
    TEditorWidget();
    virtual void SetColumnTitles(const TVecString& value){};
    virtual void SetIsShowToolBar(bool value){};

    virtual TPtrPropertyClass SelectObject() const { return TPtrPropertyClass(); }

    void SetCreateFun(const TCreateEditorObj& value) { createFun = value; }
    PROPERTIES(TEditorWidget, TWidget)

protected:
    TCreateEditorObj createFun;
};

class TEditorView{
public:
    CREATE_VIEW(TEditorView)
    virtual void SetEditObject(const TPtrPropertyClass& value){};
    virtual TPropertyEditor* Editor() { return nullptr; }
};

#endif //ATHENA_EDITORWIDGET_H
