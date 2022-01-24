//
// Created by user on 11.01.2021.
//

#include "EditorWidgetQt.h"
#include <QVBoxLayout>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QColorDialog>
#include <QInputDialog>
#include <QHeaderView>
#include <QToolBar>
#include <QFormLayout>
#include <QMenu>
#include <Algorithms.h>

INIT_PROPERTYS(TEditorWidgetQt)
ADD_MAP_ALIAS(TEditorWidget, TEditorWidgetQt)
INIT_VIEW(TEditorViewQt)

TObjTree* IndexTo(const QModelIndex &value)
{
    return static_cast<TObjTree*>(value.internalPointer());
}

TEditorWidgetQt::TEditorWidgetQt(QWidget *parent) : TDockWidget<TEditorWidget>(parent)
{
    Init();
    SetFloatingState(true);//т.к. не сохраняет размеры после скрытия и отображения, то сначала установим в док а потом уже открепим
    //dock->setFloating(true);
    dock->setVisible(false);
}

void TEditorWidgetQt::Init()
{
    auto lay = new QVBoxLayout();
    lay->setMargin(0);

    setLayout(lay);

    toolBar = new QToolBar(this);
    toolBar->setVisible(false);
    lay->addWidget(toolBar);
    addAction = toolBar->addAction(QIcon(":/icons/add_sm.png"), "Add", this, &TEditorWidgetQt::AddObject);
    delAction = toolBar->addAction(QIcon(":/icons/del_sm.png"), "Del", this, &TEditorWidgetQt::DelObject);

    model = TModelEditor::CreateShared<TModelEditor>();
    model->SetCheckedProp("isUsed");
    SetTree(model);

    treeView = new QTreeView(this);
    treeView->setEditTriggers(QAbstractItemView::AllEditTriggers);
    treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    treeView->header()->setSectionResizeMode(QHeaderView::Stretch);

    connect(treeView->header(), &QHeaderView::sectionClicked, [this](int)
    {
        treeView->header()->setSectionResizeMode(QHeaderView::Interactive);
    });
    ConnectPopup(treeView, this);

    TEditorDelegate* delegate = new TEditorDelegate(treeView);
    treeView->setItemDelegate(delegate);
    //сохраняем редактируемый индекс
    model->OnBeginRescan.connect([this, delegate](){ editingRescan = delegate->EditingIndex(); });
    //если редактируемый индекс был заново запустим его редактирование
    model->OnEndRescan.connect([this]()
        {
            if(editingRescan.isValid())
                treeView->edit(treeView->model()->index(editingRescan.row(), 1));
            //первый элемент дерева всегда открыт
            if(isAutoOpenFirst) treeView->expand(treeView->model()->index(0, 0));
        });
    model->OnDeleteObj.connect([this](){ SetTitle(Obj().expired() ? "" : LockObj()->Name() ); });
    treeView->setModel(model.get());
    lay->addWidget(treeView);
}

void TEditorWidgetQt::SetWidgetObject(const TPtrPropertyClass &value)
{
    SetObject(value);
    treeView->header()->setSectionsClickable(true);
    SetTitle(value ? value->Name() : "");
}

void TEditorWidgetQt::AddObject()
{
    QModelIndex sel = treeView->selectionModel()->currentIndex();
    if(createFun)
    {
        auto selObj = IndexTo(sel);
        if(selObj == nullptr)
            selObj = model.get();
        createFun(selObj->shared_from_this());
    }
    else
    {
        TCreateView view(this);
        view.SetApplyFun([m = model, sel](const TString &type, int index) {
            m->AddChild(sel, TPropertyClass::CreateFromType(type), index);
        });
        view.SetObjectTree(IndexTo(sel));
    }
}

void TEditorWidgetQt::DelObject()
{
    QModelIndex sel = treeView->selectionModel()->currentIndex();
    if(sel.isValid() == false) return;

    TObjTree* obj = IndexTo(sel);
    if(obj->IsProp() || obj->Parent().expired()) return;
    model->DelChild(sel);
}

void TEditorWidgetQt::SetColumnTitles(const TVecString &value)
{
    model->SetColumnTitles(value);
}

void TEditorWidgetQt::SetIsShowToolBar(bool value)
{
    toolBar->setVisible(value);
}

TPropertyEditor &TEditorWidgetQt::SetIsShowType(bool value)
{
    model->SetIsShowType(value);
    return TPropertyEditor::SetIsShowType(value);
}

TPtrPropertyClass TEditorWidgetQt::SelectObject() const
{
    QModelIndex sel = treeView->selectionModel()->currentIndex();
    if(sel.isValid() == false) return TPtrPropertyClass();

    TObjTree* obj = IndexTo(sel);
    if(obj->IsProp()) return TPtrPropertyClass();

    return obj->LockObj();
}

void TEditorWidgetQt::SetIsButtons(bool value)
{
    TEditorWidget::SetIsButtons(value);
    if(isButtons && box == nullptr)
    {
        box = new QDialogButtonBox(this);
        connect(box->addButton(TRANSR("Apply"), QDialogButtonBox::ActionRole), &QPushButton::clicked, PROXY_C(OnApply, this));
        connect(box->addButton(TRANSR("OK"), QDialogButtonBox::ActionRole), &QPushButton::clicked, PROXY_C(OnOk, this));
        connect(box->addButton(TRANSR("Cancel"), QDialogButtonBox::ActionRole), &QPushButton::clicked, PROXY_C(OnCancel, this));
        layout()->setContentsMargins(0, 0, 2, 5);
        layout()->addWidget(box);
    }
    if(box)
        box->setVisible(value);
}

void TEditorWidgetQt::OnOk()
{
    TResult res;
    OnButton(TTypeButton::Ok, res);
    APP->ShowError(res);
    if(res.IsNoError())
        dock->OnDockClose();
}

void TEditorWidgetQt::OnApply()
{
    TResult res;
    OnButton(TTypeButton::Apply, res);
    APP->ShowError(res);
}

void TEditorWidgetQt::OnCancel()
{
    TResult res;
    OnButton(TTypeButton::Cancel, res);
    dock->OnDockClose();
}

//----------------------------------TCreateView------------------------------------------------------------------------
TCreateView::TCreateView(QWidget *parent):QDialog(parent)
{
    arrays = new QComboBox(this);
    connect(arrays, QOverload<int>().of(&QComboBox::currentIndexChanged), [this](int index){ LoadTypes(index); });

    types = new QComboBox(this);

    buttons = new QDialogButtonBox(this);
    buttons->addButton(TRANSR("Apply"), QDialogButtonBox::ApplyRole);
    buttons->addButton(TRANSR("OK"), QDialogButtonBox::AcceptRole);
    buttons->addButton(TRANSR("Cancel"), QDialogButtonBox::RejectRole);
    connect(buttons, &QDialogButtonBox::clicked, [this](QAbstractButton* button){ ButtonClicked(button); });

    auto lay = new QFormLayout();
    setLayout(lay);
    lay->addRow(TRANSR("Array"), arrays);
    lay->addRow(TRANSR("Types"), types);
    lay->addWidget(buttons);
}

void TCreateView::SetApplyFun(const TCreateView::TApplyFun &value)
{
    applyFun = value;
}

void TCreateView::SetObjectTree(TObjTree *value)
{
    objTree = value;
    if(objTree == nullptr) return;
    TObjTree::TVectArrayInfo vecInfo = objTree->ArrayInfo();
    if(vecInfo.empty()) return;
    for(const TObjTree::TArrayInfo& info : vecInfo)
        arrays->addItem(std::get<0>(info).c_str(), (int)std::get<1>(info));
    exec();
}

void TCreateView::LoadTypes(int index)
{
    types->clear();
    if(index == -1) return;
    int indexProp = arrays->currentData().toInt();

    const TPropertyManager& man = objTree->LockObj()->Manager();
    if(indexProp >= int(man.CountProperty())) return;
    const TPropertyManager& manChild = TPropertyManager::Manager(man.Property(indexProp).Type());
    if(manChild.IsValid() == false) return;

    TPropertyManager::TVecPropManList childMans = manChild.AllListChildManagers();
    for(size_t i = 0; i < childMans.size(); i++)
        types->addItem(STR(childMans[i]->Type()));
}

void TCreateView::ButtonClicked(QAbstractButton *button)
{
    switch (buttons->buttonRole(button))
    {
        case QDialogButtonBox::ApplyRole:
            if(applyFun) applyFun(types->currentText().toStdString(), arrays->currentData().toInt());
        break;

        case QDialogButtonBox::AcceptRole:
            if(applyFun) applyFun(types->currentText().toStdString(), arrays->currentData().toInt());
            accept();
        break;

        case QDialogButtonBox::RejectRole:
            reject();
        break;

        default:
        break;
    }
}

//----------------------------------TModelEditor------------------------------------------------------------------------

//возвращает родительский индекс для child
//у текущего this не может быть QModelIndex и он всегда не валидный
QModelIndex TModelEditorSimple::parent(const QModelIndex &child) const
{
    //если передали не валидный индекс то не можем вернуть нормальный индекс
    if(child.isValid() == false) return QModelIndex();

    //родителя нет значит был передан индекс(child) на текущую модель

    if(IndexTo(child)->Parent().expired())
        return QModelIndex();

    auto parent = IndexTo(child)->LockParent();

    return createIndex( parent->Num(0), 0, parent.get());
}

//возвращает количество child для parent
int TModelEditorSimple::rowCount(const QModelIndex &parent) const
{
    //если передали валидный индекс значит можем получить указатель на объект
    if(parent.isValid())
        return IndexTo(parent)->CountItems();

    //для не валидного индекса надо вернуть количество текущих объектов
    return CountItems();
}

int TModelEditorSimple::columnCount(const QModelIndex &parent) const
{
    return columnTitles.size();
}

//возвращает индекс на объект child у родителя
QModelIndex TModelEditorSimple::index(int row, int column, const QModelIndex &parent) const
{
    //проверка на то что индексы row и column больше -1 и меньше количества строк и столбцов
    if(hasIndex(row, column, parent) == false) return QModelIndex();

    //для не валидного индекса возвращаем себя то иначе получим из индекса указатель на объект TObjTree
    auto parentObj = parent.isValid() ? IndexTo(parent) : this;

    return createIndex(row, column, parentObj->Item(row).get());
}

bool TModelEditorSimple::hasChildren(const QModelIndex &parent) const
{
    auto parentObj = parent.isValid() ? IndexTo(parent) : this;
    if(parentObj->IsLoaded() == false)
        parentObj->CountItems();
    return parentObj->HasChildren();
}

Qt::ItemFlags TModelEditorSimple::flags(const QModelIndex &index) const
{
    Qt::ItemFlags f = QAbstractItemModel::flags(index);
    if(index.isValid())
    {
        TObjTree* obj = IndexTo(index);
        if(index.column() == 0 && obj->IsCheckable() || index.column() > 0 && obj->IsBool())
            return Qt::ItemIsUserCheckable | f;

        if(index.column() > 0 && obj->IsEditable())
            return Qt::ItemIsEditable | f;
    }
    return f;
}

QVariant TModelEditorSimple::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole && section < int(columnTitles.size()))
        return TRANSR(columnTitles[section]);
    return QVariant();
}

void TModelEditorSimple::SetColumnTitles(const TVecString &value)
{
    columnTitles = value;
}

void TModelEditorSimple::SetObj(const TPtrPropertyClass &value)
{
    beginResetModel();
    TObjTree::SetObj(value);
    endResetModel();
}
//----------------------------------------------------------------------------------------------------------------------
QVariant TModelEditor::data(const QModelIndex &index, int role) const
{
    if(index.isValid() == false) return QVariant();
    if(role == Qt::SizeHintRole) return QSize(50, 20);
    TObjTree* obj = IndexTo(index);
    if(role == Qt::CheckStateRole)
    {
        if(index.column() == 0 && obj->IsCheckable()) return obj->IsChecked() ? Qt::Checked : Qt::Unchecked;
        if(index.column() > 0 && obj->IsBool()) return obj->Value().ToBool() ? Qt::Checked : Qt::Unchecked;
        return QVariant();
    }
    if(role == Qt::DecorationRole && index.column() == 1)
    {
        if(obj->IsColor()) return QColor(QRgba64::fromRgba64(obj->Value().ToInt()));
        return QVariant();
    }
    if(role != Qt::DisplayRole && role != Qt::EditRole)  return QVariant();

    if(index.column() == 0)
        return TRANSR(obj->Name());
    else
    {
        TVariable v = obj->Value(isShowType);
        if(obj->IsColor()) return QColor(QRgba64::fromRgba64(v.ToInt()));
        switch(v.Type())
        {
            case TVariableType::Bool: return TRANSR(v.ToString());
            case TVariableType::Int: return int(v.ToInt());
            case TVariableType::UInt: return uint32_t(v.ToUInt());
            case TVariableType::Double: if(role == Qt::EditRole) return v.ToDouble(); return QLocale::c().toString(v.ToDouble(), 'f', 3);
            case TVariableType::Str: return (obj->IsProp()) ? STR(v.ToString()) : TRANSR(v.ToString());
            case TVariableType::Enum: if(role == Qt::EditRole) return QVariant::fromValue(v.GetEnum());
            default: return TRANSR(v.ToString());
        }
    }
}

bool TModelEditor::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(index.isValid())
    {
        TObjTree *obj = IndexTo(index);
        if (role == Qt::CheckStateRole)
        {
            auto check = TWPtrObjTree(obj->shared_from_this());//для того чтобы обезопасить себя от перезагрузки полностью дерева
            if (index.column() == 0)
                obj->SetIsChecked(value == Qt::Checked);
            else
                obj->SetValue(value == Qt::Checked);
            if(check.expired() == false && index.parent().isValid())//когда меняем флаг обновим все поля
            {
                dataChanged(this->index(0, 0, index.parent()),
                            this->index(rowCount(index.parent()), 0, index.parent()));
            }
            else
                dataChanged(index, index);
        }
        else if (role == Qt::EditRole && index.column() > 0)
        {
            if (obj->IsColor())
                obj->SetValue(quint64(qvariant_cast<QColor>(value).rgba64()));
            else if(value.userType() == qMetaTypeId<TEnum>())
                obj->SetValue(qvariant_cast<TEnum>(value));
            else
                obj->SetValue(value.toString().toStdString().c_str());
        }
    }
    return QAbstractItemModel::setData(index, value, role);
}

void TModelEditor::AddChild(const QModelIndex &index, const TPtrPropertyClass &value, int indexPropArray)
{
    TObjTree* obj = IndexTo(index);
    if(obj) obj->AddChild(value, indexPropArray);
}

void TModelEditor::AddChild(const QModelIndex &index, const TPtrPropertyClass &value, TString namePropArray)
{
    TObjTree* obj = IndexTo(index);
    if(obj) obj->AddChild(value, namePropArray);
}

void TModelEditor::DelChild(const QModelIndex &index)
{
    TObjTree* par = IndexTo(parent(index));
    TObjTree* obj = IndexTo(index);
    if(par == nullptr || obj == nullptr) return;
    par->DelChild(obj->LockObj(), obj->IndProp());
}

bool TModelEditor::BeginDelete(TObjTree *objTree)
{
    if(objTree == nullptr) return false;

    auto count = objTree->CountItems();
    if(count == 0) return false;

    OnBeginRescan();

    beginRemoveRows(createIndex(objTree->Num(0), 0, objTree), 0, count - 1);
    return true;
}

void TModelEditor::EndDelete(TObjTree *objTree)
{
    endRemoveRows();
    OnDeleteObj();
}

void TModelEditor::BeginAdd(TObjTree *objTree)
{

}

void TModelEditor::EndAdd(TObjTree *objTree)
{
    if(objTree == nullptr) return;

    int count = objTree->CountItems();
    if(count == 0) return;

    beginInsertRows(objTree == this ? QModelIndex() : createIndex(objTree->Num(0), 0, objTree), 0, count - 1);
    endInsertRows();

    OnEndRescan();
}
//--------------------------------------TEditItemDelegate---------------------------------------------------------------
TEditItemDelegate::TEditItemDelegate(QObject *parent, bool delay):QItemDelegate(parent), isDelay(delay)
{
    auto changed = [this]() { isChanged = true; };

    waitInt.SetRightFun(changed);
    if(isDelay) waitInt.SetResultFun([this](int) { CommitValue(); });

    waitDouble.SetRightFun(changed);
    if(isDelay) waitDouble.SetResultFun([this](double) { CommitValue(); });

    waitString.SetRightFun(changed);
    if(isDelay) waitString.SetResultFun([this](const QString&) { CommitValue(); });
}

QWidget* TEditItemDelegate::createEditor(QWidget *parent,
                                         const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    isChanged = false;
    isFirst = true;
    editingIndex = index;
    TCustProp* prop = CustProp(index);

    int userType = index.data(Qt::EditRole).userType();
    if(userType == qMetaTypeId<TEnum>())
    {
        TEnumComboBox* el = new TEnumComboBox(parent);
        connect(el, QOverload<int>().of(&TEnumComboBox::currentIndexChanged), this, &TEditorDelegate::CommitInt);
        //connect(el, QOverload<int>().of(&TEnumComboBox::currentIndexChanged), waitInt.DelayFun());
        curWidget = el;
        return el;
    }

    switch(userType)
    {
        case QVariant::Bool:
        {
            TBoolCheckBox* cb = new TBoolCheckBox(parent);
            connect(cb, QOverload<int>().of(&TBoolCheckBox::stateChanged), this, &TEditorDelegate::CommitInt);
            curWidget = cb;
            return cb;
        }

        case QVariant::Int:
        case QVariant::UInt:
        case QVariant::ULongLong:
        case QVariant::LongLong:
        {
            QSpinBox *sb = new QSpinBox(parent);
            sb->setFrame(false);
            int m = (userType == QVariant::ULongLong || userType == QVariant::UInt) ? 0 : INT_MIN;
            sb->setMinimum((prop && std::isnan(prop->min) == false) ? prop->min : m);
            sb->setMaximum((prop && std::isnan(prop->max) == false) ? prop->max : INT_MAX);
            sb->setSizePolicy(QSizePolicy::Ignored, sb->sizePolicy().verticalPolicy());
            connect(sb, QOverload<int>().of(&QSpinBox::valueChanged), this, &TEditorDelegate::CommitInt);
            //connect(sb, QOverload<int>().of(&QSpinBox::valueChanged), waitInt.DelayFun());
            curWidget = sb;
            return sb;
        }
        case QVariant::Double:
        {
            QDoubleSpinBox *sb = new QDoubleSpinBox(parent);
            sb->setDecimals(3);
            sb->setFrame(false);
            sb->setMinimum((prop && std::isnan(prop->min) == false ) ? prop->min : INT_MIN);
            sb->setMaximum((prop && std::isnan(prop->max) == false ) ? prop->max : INT_MAX);
            sb->setSizePolicy(QSizePolicy::Ignored, sb->sizePolicy().verticalPolicy());
            connect(sb, QOverload<double>().of(&QDoubleSpinBox::valueChanged), waitDouble.DelayFun());
            curWidget = sb;
            return sb;
        }
        default:
        {
            QLineEdit* res = (QLineEdit*)QItemDelegate::createEditor(parent, option, index);
            auto button = new TButtonForEditor(res);
            if(userType == QVariant::Color)
                QObject::connect(button, &QPushButton::clicked, [res](){ res->setText(QColorDialog::getColor().name()); });
            else
                QObject::connect(button, &QPushButton::clicked, [res](){
                    bool isOk = false;
                    auto text = QInputDialog::getMultiLineText(res, TRANSR(APP->Title()), TRANSR("Input text"), res->text(), &isOk);
                    if(isOk)
                        res->setText(text);
                });
            connect(res, QOverload<const QString&>().of(&QLineEdit::textChanged), waitString.DelayFun());
            curWidget = res;
            return res;
        }
    }
}

void TEditItemDelegate::destroyEditor(QWidget *editor, const QModelIndex &index) const
{
    editingIndex = QModelIndex();
    curWidget = nullptr;
    QAbstractItemDelegate::destroyEditor(editor, index);
}

void TEditItemDelegate::CommitInt(int value)
{
    isChanged = true;
    commitData(qobject_cast<QWidget*>(sender()));
}

void TEditItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if(isChanged)
    {
        isChanged = false;
        QItemDelegate::setModelData(editor, model, index);
    }
}

void TEditItemDelegate::CommitValue()
{
    if(curWidget)
        commitData(curWidget);
}

void TEditItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QItemDelegate::setEditorData(editor, index);
    isChanged = false;
}
//----------------------------------TEditorDelegate---------------------------------------------------------------------

QWidget* TEditorDelegate::createEditor(QWidget *parent,
                                       const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const
{

    TCustProp* prop = CustProp(index);

    if(prop->comboFun)
    {
        TStringListComboBox* sl = new TStringListComboBox(parent, prop->comboFun(IndexTo(index)), std::isnan(prop->min) == false);
        connect(sl, QOverload<int>().of(&TStringListComboBox::currentIndexChanged), this, &TEditItemDelegate::CommitInt);
        curWidget = sl;
        return sl;
    }
    return TEditItemDelegate::createEditor(parent, option, index);
}

TCustProp *TEditorDelegate::CustProp(const QModelIndex &index) const
{
    TObjTree* obj = IndexTo(index);
    return &obj->PropCustoms()->CustProp(obj->Name());
}


//--------------------TEnumComboBox-------------------------------------------------------------------------------------
TEnumComboBox::TEnumComboBox(QWidget *parent, bool isDef ) : QComboBox(parent)
{
    if(isDef == false)
    {
        setFrame(false);
        setSizePolicy(QSizePolicy::Ignored, sizePolicy().verticalPolicy());
    }
}

void TEnumComboBox::setValue(QVariant value)
{
    enumValue = qvariant_cast<TEnum>(value);
    clear();
    //заполняем возможные варианты перечисления
    const TEnumInfo& info = TEnumInfo::EnumInfo(enumValue.Info());
    if(info.ConvertCategory() == 0 || !FunConvert())
        for(auto i = begin; i < info.CountNames(); i++)
            addItem(TRANSR(info.Name(i)), i);
    else
    {
        TVecString res = FunConvert()(info);
        for(auto i = begin; i < res.size(); i++)
            addItem(TRANSR(res[i]), i);
    }

    if(count()) setCurrentIndex(enumValue.Index() - begin);
}

QVariant TEnumComboBox::value() const
{
    TEnum res = enumValue;
    return QVariant::fromValue(res.SetIndex(currentData().toInt()));
    //return currentData();
}
//-----------------------TBoolCheckBox----------------------------------------------------------------------------------
TBoolCheckBox::TBoolCheckBox(QWidget *parent) : QCheckBox(parent)
{
    setTristate(false);
    connect(this, QOverload<int>().of(&TBoolCheckBox::stateChanged),
            [this](int)
            {
                setText( checkState() == Qt::Checked ? TRANSR("true") : TRANSR("false"));
            });
    setChecked(false);
}

void TBoolCheckBox::setChecked(bool value)
{
    setCheckState(value ? Qt::Checked : Qt::Unchecked);
}

bool TBoolCheckBox::checked() const
{
    return checkState() == Qt::Checked;
}
//-------------------------TStringListComboBox--------------------------------------------------------------------------

TStringListComboBox::TStringListComboBox(QWidget *parent, const TVecString &list, bool isResInt) :
    QComboBox(parent), isInt(isResInt)
{
    setFrame(false);
    setSizePolicy(QSizePolicy::Ignored, sizePolicy().verticalPolicy());
    for(const auto& it : list)
        addItem(it.c_str());
}

void TStringListComboBox::setValue(QVariant value)
{
    if(isInt)
        setCurrentIndex(value.toInt());
    else
        setCurrentText(value.toString());
}

QVariant TStringListComboBox::value() const
{
    if(isInt)
        return currentIndex();
    else
        return currentText();
}
//-----------------------------TButtonForEditor--------------------------------------------------------------------
TButtonForEditor::TButtonForEditor(QWidget* parent): QPushButton(parent)
{
    setText("...");
    setMaximumWidth(20);
    if(parent == nullptr) return;

    parent->setLayout(new QHBoxLayout(parent));
    parent->layout()->setMargin(0);
    parent->layout()->addWidget(this);
    parent->layout()->setAlignment(this, Qt::AlignRight);
}

//----------------------------------------------------------------------------------------------------------------------
TEditorViewQt::TEditorViewQt():QDialog(nullptr, Qt::Tool)
{
    setMinimumSize(400, 300);
    setLayout(new QVBoxLayout());
    layout()->setMargin(0);
    widget = new TEditorWidgetQt(this);
    widget->SetIsShowToolBar(true);
    layout()->addWidget(widget);
}

void TEditorViewQt::SetEditObject(const TPtrPropertyClass &value)
{
    widget->SetWidgetObject(value);
    exec();
}

TPropertyEditor *TEditorViewQt::Editor()
{
    return widget;
}