//
// Created by user on 08.11.2021.
//

#include "App.h"
#include "Property/Serialization.h"
#include "Algorithms.h"
#include "FileSystem/FileSystem.h"
#include "App/EditorWidget.h"

INIT_PROPERTYS(TApp)

const TString &TAppItem::Title() const
{
    return title;
}

void TAppItem::SetTitle(const TString &value)
{
    title = value;
}
//----------------------------------------------------------------------------------------------------------------------

TApp::TApp(const TString &nameApp, const TString &titleApp)
{
    Single() = this;
    name = nameApp;
    title = titleApp;
    customDir = (fs::current_path() / "customs").string();

    menu->AddItem("File|-", TFunItemCall(), lowPriority );
    menu->AddItem("File|Close", [this]() { Close(); }, lowPriority)->SetIndexImg(2).SetShortcut("Ctrl+Q");
    menu->AddItem("Edit");
    menu->AddItem("Tools|Options", [this]() { Options(); })->SetShortcut("Ctrl+Alt+W");//SetIndexImg(5).SetVisible(false);
    menu->AddItem("Tools|Language|Russian", [this](){ trans->SetLang(lgRus); menu->OnChange(); });
    menu->AddItem("Tools|Language|English", [this](){ trans->SetLang(lgEng); menu->OnChange(); });
    menu->AddItem("Help|About", [this]() { About(); });

    //HISTORY_IF(HISTORY->SetTrans([](const TString& value){ return TRANS(value); });)
}

TApp::~TApp()
{
    Single() = nullptr;
}

TResult TApp::LoadCustoms(bool def)
{
    fs::path customPath;
    if(def)
    {
        customPath = FileName();
    }
    else
    {
        customPath = customDir;
        customPath /= FileName();
        if(fs::exists(customPath) == false)
            customPath = FileName();
    }

    if(fs::exists(customPath) == false) return TAppResult::NoCustom;

    return TSerialization().LoadFromFile(customPath.string(), this);
}

TResult TApp::SaveCustoms(bool def)
{
    fs::path customPath;
    if(def)
    {
        customPath = FileName();
    }
    else
    {
        customPath = customDir;
        if(fs::exists(customPath) == false) fs::create_directories(customPath);
        customPath /= FileName();
    }
    return TSerialization().SaveToFile(customPath.string(), this);
}

int TApp::Run()
{
    BeforeRun();
    AfterRun();
    return 0;
}

void TApp::BeforeRun()
{
    menu->OnChange();
}

void TApp::AfterRun()
{
}

const TString &TApp::CustomDir() const
{
    return customDir;
}

void TApp::SetCustomDir(const TString &value)
{
    customDir = value;
}

TResult TApp::InitCustomDir()
{
    return TResult();
}

TPtrMenuTree TApp::Menu() const
{
    return menu;
}

TPtrTranslator TApp::Trans() const
{
    return trans;
}

TResult TApp::ShowMessage(const TString &message, bool isQuestion, bool isCancel)
{
    return TResult();
}

TResult TApp::InputText(const TString &message, TString &value)
{
    return TResult();
}

TResult TApp::InputItems(const TString &message, const TVecString &items, TString &value, int &index, bool isEdit)
{
    return TResult();
}

bool TApp::IsClose()
{
    return true;
}

void TApp::Options()
{
    auto view = TEditorView::CreateFunc()();
    view->Editor()->SetIsShowType(true);
    view->SetEditObject(SafePtrInterf(this));
}

bool TApp::Close()
{
    return IsClose();
}

TResult TApp::SelectFile(const TString &filter, const TString &ext, TString &res)
{
    return TResult();
}

const TString &TApp::SecondTitle() const
{
    return secondTitle;
}

void TApp::SetSecondTitle(const TString &value)
{
    secondTitle = value;
}

TPtrGlobalCustom TApp::GlobalCustoms() const
{
    return TGlobalCustom::Single();
}

TString TApp::CustomDir(const TString &childDir, const TString& fileName, bool autoDir)
{
    fs::path res(CustomDir());
    res /= childDir;
    if(fs::exists(res) == false && autoDir)
        fs::create_directories(res);
    if(fileName.empty() == false)
        res /= fileName;
    return res.string();
}

TPtrProgress TApp::GetProgress()
{
    auto res = std::make_shared<TProgress>();
    progresses.emplace_back(res);
    return res;
}

void TApp::DeleteProgress(const TPtrProgress &value)
{
    RemoveVal(progresses, value);
}

TPtrTimer TApp::CreateTimer(const TTimer::TInterval& interval, const TTimerFunction& fun)
{
    return std::make_shared<TTimer>();
}

void TApp::About()
{
#ifdef APP_VER
    ShowMessage(Name() + "\nDeveloper: Minnullin I Z\nVersion: " APP_VER);
#endif
}

TPtrWidget TApp::CreateWidget(const TString &type)
{
    auto& m = MapAliasWidgets();
    auto it = m.find(type);
    if(it != m.end())
        return ::CreateFromType<TPtrWidget>(it->second);
    else
        return ::CreateFromType<TPtrWidget>(type);
}

bool TApp::ShowError(const TResult &value)
{
    if(value.IsNoError())
        return false;
    if(value.IsCancel() == false)
        ShowMessage(TRANS(TResult::TextError(value)));
    return true;
}


//----------------------------------------------------------------------------------------------------------------------
TResult TThreadProgress::Run()
{
    auto res = InitFunction();
    if(APP->ShowError(res)) return res;

    prog = APP->GetProgress();                                              //создаем прогресс
    prog->OnResult.connect(&TThreadProgress::ResultFunction, this);     //указываем функцию результата
    if(NoThread() == false)
    {
        std::thread t(&TThreadProgress::ThreadFuncImpl, this);
        t.detach();
    }
    else
        NoThreadFuncImpl();
    return TResult();
}

void TThreadProgress::Reset()
{
    prog.reset();
}

void TThreadProgress::ThreadFuncImpl()
{
    StartThread();
    NoThreadFuncImpl();
    FinishThread();
}

void TThreadProgress::NoThreadFuncImpl()
{
    auto res = ThreadFunction();
    if(prog->IsCancel() == false)
        prog->SetResult(res);
    else
        Reset();
}

TResult TThreadProgress::InitFunction()
{
    return TResult();
}

TResult TThreadProgress::ThreadFunction()
{
    return TResult();
}

void TThreadProgress::ResultFunction(TResult res)
{
    APP->ShowError(res);
    Reset();
}
