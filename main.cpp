
#ifndef GUI_INIT_HDR
#include "App.h"
using TApplication = TApp;
int main(int argc, char* argv[])
#else
#include GUI_INIT_HDR
#endif
{
    TApplication app;
    app.LoadCustoms();
    auto res = app.Run();
    app.SaveCustoms();
    return res;
}