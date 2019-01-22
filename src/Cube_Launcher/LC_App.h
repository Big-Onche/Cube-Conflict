/***************************************************************
 * Name:      LC_App.h
 * Author:    VuSurTF1
 * Created:   2014-06-23
 **************************************************************/

#ifndef LC_APP_H
#define LC_APP_H
#define __GXX_ABI_VERSION 1002

#include <wx/app.h>

class LC_App : public wxApp
{
    public:
        virtual bool OnInit();
};

#endif // LC_APP_H
