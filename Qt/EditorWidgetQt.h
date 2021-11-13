//
// Created by user on 11.01.2021.
//

#ifndef ATHENA_EDITORWIDGETQT_H
#define ATHENA_EDITORWIDGETQT_H

#include "../EditorWidget.h"
#include "MainWindowQt.h"

#include <QTreeView>
#include <QItemDelegate>
#include <QComboBox>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QBoxLayout>

class TModelEditor;

class TEditorWidgetQt : public TFloatWidget<TEditorWidget>{
public:
    TEditorWidgetQt(QWidget* parent);
    void SetWidgetObject(const TPtrPropertyClass& value) override;
    void SetColumnTitles(const TVecString& value) override;
    void SetIsShowToolBar(bool value) override;
    TPropertyEditor& SetIsShowType(bool value) override;

    TPtrPropertyClass SelectObject() const override;
    QBoxLayout* Layout() { return ((QBoxLayout*)layout()); }
    QToolBar* ToolBar() { return toolBar; }
    QTreeView* TreeView() { return treeView; }

    QAction* AddAction() { return addAction; }
    QAction* DelAction() { return delAction; }
private:
    QTreeView* treeView;
    QToolBar* toolBar;
    QAction* addAction;
    QAction* delAction;
    std::shared_ptr<TModelEditor> model;
    QModelIndex editingRescan;
    void Init();
    void AddObject();
    void DelObject();
};

class TCreateView : public QDialog{
public:
    TCreateView(QWidget* parent);
    using TApplyFun = std::function<void(const TString&, int)>;
    void SetApplyFun(const TApplyFun& value);
    void SetObjectTree(TObjTree* value);

private:
    TApplyFun applyFun;
    QComboBox* arrays;
    QComboBox* types;
    QDialogButtonBox* buttons;
    TObjTree* objTree = nullptr;
    void LoadTypes(int index);
    void ButtonClicked(QAbstractButton* button);
};

class TModelEditorSimple : public QAbstractItemModel, public TObjTree{
public:
    void SetColumnTitles(const TVecString& value);
    void SetObj(const TPtrPropertyClass& value) override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

protected:

    TVecString columnTitles = {"title", "value"};
};

class TModelEditor : public TModelEditorSimple{
public:
    void AddChild(const QModelIndex& index, const TPtrPropertyClass& value, int indexPropArray);
    void AddChild(const QModelIndex& index, const TPtrPropertyClass& value, TString namePropArray);
    void DelChild(const QModelIndex& index);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    bool IsShowType() const { return isShowType; }
    void SetIsShowType(bool value) { isShowType = value; }

    TOnNotify OnBeginRescan;
    TOnNotify OnEndRescan;
protected:
    bool BeginDelete(TObjTree* objTree) override;
    void EndDelete(TObjTree* objTree) override;
    void BeginAdd(TObjTree* objTree) override;
    void EndAdd(TObjTree* objTree) override;
private:
    bool isShowType = false;
};

TObjTree* IndexTo(const QModelIndex &value);

class TEditItemDelegate : public QItemDelegate {
public:
    TEditItemDelegate(QObject *parent = nullptr, bool delay = true);

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    void destroyEditor(QWidget *editor, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;
    void CommitInt(int);

    const QModelIndex& EditingIndex() const { return editingIndex; }
protected:
    mutable bool isChanged = false;
    mutable bool isFirst = true;
    bool isDelay;
    mutable QWidget* curWidget = nullptr;
    TWaitCallDouble waitDouble;
    TWaitCallInt waitInt;
    TWaitCallQString waitString;

    mutable QModelIndex editingIndex;//индекс который в данный момент редактируется

    void CommitValue();

    virtual TCustProp* CustProp(const QModelIndex &index) const { return nullptr; };
};

class TEditorDelegate : public TEditItemDelegate {
public:
    TEditorDelegate(QObject *parent = nullptr):TEditItemDelegate(parent){}

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    TCustProp* CustProp(const QModelIndex &index) const override;
};


//необходимо чтобы передавать TEnum в QVariant как значение
Q_DECLARE_METATYPE(TEnum)

//Редактор QComboBox выводит полный список элементов TEnum
class TEnumComboBox : public QComboBox{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ value WRITE setValue USER true)
public:
    TEnumComboBox(QWidget* parent = nullptr, bool isDef = false);
    void setValue(QVariant value);
    QVariant value() const;
protected:
    TEnum enumValue;
    size_t begin = 0;//с какого индекса выводить список TEnum
};

//Редактор QComboBox выводит список элементов TEnum без нулевого элемента
class TEnumComboBoxOne : public TEnumComboBox{
public:
    TEnumComboBoxOne(QWidget* parent = nullptr, bool isDef = false):TEnumComboBox(parent, isDef){ begin = 1; }
};

class TBoolCheckBox : public QCheckBox{
    Q_OBJECT
    Q_PROPERTY(bool checked READ checked WRITE setChecked USER true)
public:
    TBoolCheckBox(QWidget* parent = nullptr);

    bool checked() const;
    void setChecked(bool value);

};

//Редактор QComboBox выводит полный список элементов TEnum
class TStringListComboBox : public QComboBox{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ value WRITE setValue USER true)
public:
    TStringListComboBox(QWidget* parent, const TVecString& list, bool isResInt = false);
    void setValue(QVariant value);
    QVariant value() const;

private:
    bool isInt;
};

class TButtonForEditor : public QPushButton{
public:
    TButtonForEditor(QWidget* parent = nullptr);
};


class TEditorViewQt : public QDialog, public TEditorView{
public:
    TEditorViewQt();
    void SetEditObject(const TPtrPropertyClass & value) override;
    TPropertyEditor* Editor() override;
private:
    TEditorWidgetQt* widget;
};


#endif //ATHENA_EDITORWIDGETQT_H
