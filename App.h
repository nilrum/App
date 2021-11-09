//
// Created by user on 08.11.2021.
//

#ifndef BASEAPP_APP_H
#define BASEAPP_APP_H

#include "Property/GlobalCustom.h"
#include "Property/Result.h"
#include "Translator.h"
#include "Timer.h"
#include "MenuTree.h"
#include "Progress.h"

class TAppItem : public TPropertyClass{
public:
    const TString& Title() const;
    virtual void SetTitle(const TString& value);

    PROPERTIES(TAppItem, TPropertyClass,
            PROPERTY(TString, title, Title, SetTitle);
    )
protected:
    TString title;//отображаемое имя
};


enum class TMessResult{ Yes, No };
enum class TAppResult {
    Ok,
    NoCustom
};

class TApp : public TAppItem{
public:
    TApp(const TString& nameApp, const TString& titleApp = TString());
    ~TApp();


    const TString& SecondTitle() const;//дополнительный текст для вывода рядом с меткой
    virtual void SetSecondTitle(const TString& value);
    /* Загружает настройки приложения
     * Если есть настройки в каталоге CustomDir то грузим из него
     * Если нет то грузим из файла по умолчанию <name>.xml
     * def принудительно грузить из настроек по умолчанию
     */
    virtual TResult LoadCustoms(bool def = false);

    /* Сохраняет настройки приложения
     * Сохраняет в файл настроек в каталоге CustomDir
     * def принудительно сохранять в файл настроек по умолчанию
     */
    virtual TResult SaveCustoms(bool def = false);

    virtual int Run();              //Запускает выполнение приложения, если конфигурации нет, то создает по умолчанию
    virtual bool Close();           //Завершает работу приложения
    bool IsClose();                 //можно ли закрыть приложение, возвращает true если будет закрыто

    const TString& CustomDir() const;       //Каталог с основными настройками приложения
    void SetCustomDir(const TString& value);//Установить основной каталог приложения

    TResult InitCustomDir();                //Инициализирует CustomDir исходя из профиля пользователя

    //Возвращает каталог настроек с проверкой на существование
    TString CustomDir(const TString& childDir, const TString& fileName = TString(), bool autoDir = false);

    /* Отображает сообщение или вопрос
     * Если это сообщение isQuestion == false то будет только кнопка OK
     * Если это вопрос isQuestion == true, то будут кнопки Yes No,
     * Если isCancel и вопрос то добавится еще кнопка Cancel
     */
    virtual TResult ShowMessage(const TString& message, bool isQuestion, bool isCancel);
    inline  TResult ShowMessage(const TString& message, bool isQuestion = false)
    {
        return ShowMessage(message, isQuestion, false);
    }

    virtual TResult InputText(const TString& message, TString& value);

    /* Выводит список для выбора пользователем
     * Возвращает выбранный или введеный текст в value
     * Возвращает выбранный индекс или -1 если текст ввели
     * isEdit определяет возможность ввода текста пользователем
     * Если передан index == -1, а value не пустое значение то пытается установить его по умолчанию
     */
    virtual TResult InputItems(const TString& message, const TVecString& items, TString& value, int& index, bool isEdit);

    /* Вызывает диалог выбора файла
     * filter может состоять как просто из маски *.txt, так и из маски с названием Txt files;*.txt
     * ext определяет диалог открытия или сохранения файла.
     * Если ext пустой то диалог открытия, иначе сохранения и расширение файла по умолчанию
     */
    virtual TResult SelectFile(const TString &filter, const TString &ext, TString &res);

    inline TResult SelectOpen(const TString &filter, TString &res)
    {
        return SelectFile(filter, TString(), res);
    }

    //Создает новый прогресс или возвращает не используемый
    virtual TPtrProgress GetProgress();
    void DeleteProgress(const TPtrProgress& value);

    //Запускает таймер для функции
    virtual TPtrTimer CreateTimer(const TTimer::TInterval& interval, const TTimerFunction& fun);

    //Добавляет информацию в лог приложения
    void Log(const TString& value){};

    //Сообщение о приложении
    void About();

    PROPERTIES_CREATE(TApp, TAppItem, NO_CREATE(),
      //PROPERTY(TString, customDir, CustomDir, SetCustomDir);
      PROPERTY(TString, lastSaveDir, LastSaveDir, SetLastSaveDir);
      PROPERTY(TString, lastOpenDir, LastOpenDir, SetLastOpenDir);
      PROPERTY_READ(TTranslator, trans, Trans);
      PROPERTY_READ(TGlobalCustom, globalCustoms, GlobalCustoms);
    )

    TPtrMenuTree Menu() const;
    TPtrTranslator Trans() const;
    TPtrGlobalCustom GlobalCustoms() const;

    TOnNotify BeforeShowing;                    //событие перед показом главной формы

    PROPERTY_FUN(TString, lastSaveDir, LastSaveDir, SetLastSaveDir);
    PROPERTY_FUN(TString, lastOpenDir, LastOpenDir, SetLastOpenDir);

    STATIC_ARG(TApp*, Single, nullptr);
protected:
    void Options();
    TString secondTitle;
    TString customDir;
    TPtrMenuTree menu = TMenuTree::Create();
    TPtrTranslator trans = std::make_shared<TTranslator>();

    TString lastSaveDir;    //последний каталог сохранения
    TString lastOpenDir;    //последний каталог открытия

    std::vector<TPtrProgress> progresses;

    inline TString FileName() const { return name + ".xml"; } //имя файла настроек

    void BeforeRun();
    void AfterRun();
};

#endif //BASEAPP_APP_H
