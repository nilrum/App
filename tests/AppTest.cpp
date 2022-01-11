//
// Created by user on 08.01.2021.
//

#include "MockClasses.h"
#include "Base/FileSystem/FileSystem.h"
TEST(App, Init)
{
    EXPECT_EQ(TApp::Single(), nullptr);
    {
        TApp app("testApp", "Application test");
        EXPECT_EQ(TApp::Single(), &app);
        EXPECT_EQ(app.Name(), TString("testApp"));
        EXPECT_EQ(app.Title(), TString("Application test"));
        EXPECT_EQ(app.SecondTitle(), TString());
        EXPECT_EQ(app.CustomDir(), (fs::current_path() / "customs").string());
        EXPECT_EQ(app.Menu() != nullptr, true);
        EXPECT_EQ(app.Trans() != nullptr, true);
        EXPECT_EQ(app.GlobalCustoms() != nullptr, true);

        EXPECT_EQ(app.Menu()->CountItems(), 4);//File, Edit, Tools, Help
    }
    EXPECT_EQ(TApp::Single(), nullptr);
}

TEST(App, LoadSaveCustoms)
{
    //файл настроек по умолчанию с завода
    const TString defFile = (fs::current_path() / "testApp.xml").string();
    //файл создаваемый у пользователя
    const TString custFile = (fs::current_path() / "customs/testApp.xml").string();

    //если файл настроек есть удаляем его
    if(fs::exists(defFile))
        fs::remove(defFile);

    if(fs::exists(custFile))
        fs::remove(custFile);

    {
        //загрузка без каких либо файлов настройки
        TApp app("testApp", "Test application");
        app.SetSecondTitle("Second title");
        TResult res = app.LoadCustoms();

        //Т.к. файла нет должны получить код ошибки
        EXPECT_EQ(res.Is(TAppResult::NoCustom), true);
        EXPECT_EQ(app.Title(), "Test application");
        EXPECT_EQ(app.SecondTitle(), TString("Second title"));
        //принудительно сохраняем файл по умолчанию
        res = app.SaveCustoms(true);

        //должен был сохранится файл настроек по умолчанию
        EXPECT_EQ(res.IsNoError(), true);
        EXPECT_EQ(fs::exists(defFile), true);
        EXPECT_EQ(fs::exists(custFile), false);
    }
    {
        TApp app("testApp");
        TResult res = app.LoadCustoms();

        //Файл есть значит ошибки загрузки быть не должно
        EXPECT_EQ(res.IsNoError(), true);
        //Title должен был загрузится
        EXPECT_EQ(app.Title(), TString("Test application"));
        //SecondTitle не сохраняется
        EXPECT_EQ(app.SecondTitle(), TString());

        app.SetTitle("New app title");
        //сохряняем в каталог пользователя
        res = app.SaveCustoms();

        //должен был сохранится файл настроек пользователя
        EXPECT_EQ(res.IsNoError(), true);
        EXPECT_EQ(fs::exists(defFile), true);
        EXPECT_EQ(fs::exists(custFile), true);
    }
    {
        TApp app("testApp");
        TResult res = app.LoadCustoms();

        //Файл есть значит ошибки загрузки быть не должно
        EXPECT_EQ(res.IsNoError(), true);
        //Title должен был загрузится
        EXPECT_EQ(app.Title(), TString("New app title"));
    }
}

TEST(App, InitCustomDir)
{

}
/*
TEST(App, Run)
{
    {
        //если конфига нет то создается по умолчанию
        TApp app("testApp");
        EXPECT_EQ(app.Config() == nullptr, true);
        EXPECT_EQ(app.Run(), 0);
        EXPECT_EQ(app.Config() != nullptr, true);
    }
    {
        TApp app("testApp");
        auto conf = std::make_shared<TConfigMock>();
        EXPECT_CALL(*conf, CreateWidgets()).Times(1);
        EXPECT_CALL(*conf, DeleteWidgets()).Times(1);
        app.SetConfig(conf);
        EXPECT_EQ(app.Run(), 0);
        EXPECT_EQ(app.Config(), conf);
    }
}

TEST(App, Config)
{
    TApp app("testApp");
    EXPECT_EQ(app.Config(), nullptr);

    auto conf = std::make_shared<TConfig>();
    app.SetConfig(conf);

    EXPECT_EQ(app.Config(), conf);
}

TEST(App, CreateWidget)
{
    TApp app("testApp");
    TPtrWidget w = app.CreateWidget("TWidget");
    EXPECT_EQ(w != nullptr, true);
}

TEST(App, IsClose)
{
    {
        TAppMock app("testApp");
        EXPECT_CALL(app, ShowMessage(_, true, true)).Times(0);

        //конфигурации нет значит и закрываемся по умолчанию
        EXPECT_EQ(app.IsClose(), true);

        app.SetConfig(std::make_shared<TConfig>());
        //объект конфигурации есть, но он не изменен то закрываемся без вопроса
        EXPECT_EQ(app.IsClose(), true);
    }

    {
        TAppMock app("testApp");
        EXPECT_CALL(app, ShowMessage(_, true, true)).Times(1);

        app.SetConfig(std::make_shared<TConfig>());
        app.Config()->SetIsModified();
        //объект конфигурации есть, но он не изменен то закрываемся без вопроса
        EXPECT_EQ(app.IsClose(), true);
    }
}

//--------------------------------------------TProfile------------------------------------------------------------------
TEST(Profile, Init)
{
    TProfile profile;
    EXPECT_EQ(profile.Name(), TString("default"));
}
//--------------------------------------------TProfileList------------------------------------------------------------------
TEST(ProfileList, Init)
{
    TProfileList list;
    EXPECT_EQ(list.Name(), TString("ProfileList"));
    EXPECT_EQ(list.CountProfiles(), 1);
    EXPECT_EQ(list.IndexCurrentProfile(), 0);
    //Проверку на добавление и удаление сделана в конфиге
}

//--------------------------------------------TConfig-------------------------------------------------------------------

TEST(Config, Init)
{
    TConfig conf;
    EXPECT_EQ(conf.Title(), TString());
    EXPECT_EQ(conf.IsModified(), false);
    EXPECT_EQ(conf.CountProfiles(), 1);
    EXPECT_EQ(conf.IndexCurrentProfile(), 0);
    EXPECT_EQ(conf.CountWidgets(), 0);
}

TEST(Config, IsModifed)
{

}

TEST(Config, IsClose)
{

}

TEST(Config, AddDelIndexProfile)
{
    int countCall = 0;
    auto funCheck = [&countCall](){ countCall++; };

    TConfig conf;
    conf.OnProfiles.connect(funCheck);

    //для создания профиля необходимо использовать метод конфига
    TPtrProfile newProfile = conf.CreateProfile();
    ASSERT_EQ(newProfile != nullptr, true);

    //количество профилей в списке не меняет
    EXPECT_EQ(conf.CountProfiles(), 1);
    EXPECT_EQ(conf.IndexCurrentProfile(), 0);
    EXPECT_EQ(countCall, 0);
    EXPECT_EQ(conf.IsModified(), false);

    //добавляем новый профиль
    conf.AddProfile(newProfile);
    EXPECT_EQ(conf.CountProfiles(), 2);
    EXPECT_EQ(conf.IndexCurrentProfile(), 0);
    EXPECT_EQ(countCall, 1);
    EXPECT_EQ(conf.IsModified(), true);

    conf.SetIsModified(false);//сбрасываем для проверки
    EXPECT_EQ(conf.IsModified(), false);

    //удаляем профиль
    conf.DelProfile(newProfile);
    EXPECT_EQ(conf.CountProfiles(), 1);
    EXPECT_EQ(conf.IndexCurrentProfile(), 0);
    EXPECT_EQ(countCall, 2);
    EXPECT_EQ(conf.IsModified(), true);

    conf.SetIsModified(false);//сбрасываем для проверки
    EXPECT_EQ(conf.IsModified(), false);

    //удаляем профиль по умолчанию
    TPtrProfile oldProfile = conf.Profile(conf.IndexCurrentProfile());
    conf.DelProfile(oldProfile);
    EXPECT_EQ(conf.CountProfiles(), 1);//количество профилей не изменилось
    EXPECT_EQ(conf.IndexCurrentProfile(), 0);
    EXPECT_EQ(countCall, 3);
    EXPECT_EQ(conf.Profile(conf.IndexCurrentProfile()) != oldProfile, true);//был создан новый профиль по умолчанию
    EXPECT_EQ(conf.IsModified(), true);

    //проверяем что индекс тоже корректируется
    conf.AddProfile(newProfile);
    EXPECT_EQ(countCall, 4);

    conf.SetIsModified(false);//сбрасываем для проверки
    EXPECT_EQ(conf.IsModified(), false);

    conf.SetIndexCurrentProfile(1);
    EXPECT_EQ(conf.IndexCurrentProfile(), 1);
    EXPECT_EQ(countCall, 5);
    EXPECT_EQ(conf.IsModified(), true);

    conf.DelProfile(newProfile);
    EXPECT_EQ(conf.CountProfiles(), 1);
    EXPECT_EQ(conf.IndexCurrentProfile(), 0);
    EXPECT_EQ(countCall, 6);

}

TEST(Config, SaveLoadProfile)
{

}

TEST(Config, AddWidget)
{
    TConfig conf;
    EXPECT_EQ(conf.AddWidget(TPtrWidget()) == nullptr, true);
    EXPECT_EQ(conf.CountWidgets(), 0);

    auto wid = std::make_shared<TWidget>();
    EXPECT_EQ(conf.AddWidget(wid) == wid, true);
    EXPECT_EQ(conf.CountWidgets(), 1);
    EXPECT_EQ(conf.Widget(0), wid);

    EXPECT_EQ(conf.FindWidget("Widget") == wid, true);
}*/