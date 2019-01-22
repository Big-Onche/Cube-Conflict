/***************************************************************
 * Name:      LC_Main.h
 * Author:    VuSurTF1
 * Created:   2014-06-23
 **************************************************************/

#ifndef LC_MAIN_H
#define LC_MAIN_H

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include "LC_App.h"

enum
{
    TEXTE,

    BTN_LANCER_CISLATTACK_32,
    BTN_LANCER_CISLATTACK_64,
    BTN_LANCER_CISLATTACK_S,

    BTN_LANCER_INDEV,
    BTN_LANCER_SITEWEB,
    BTN_LANCER_QUITTER,

    BTN_RESOLUTION,
};

class LC_Frame: public wxFrame
{
    public:
        LC_Frame(wxFrame *frame, const wxString& title);
        ~LC_Frame();
    private:
        enum
        {
            idMenuQuit = 1000,
            idMenuAbout
        };
        void OnClose(wxCloseEvent& event);
        void OnAbout(wxCommandEvent& event);


        void OnButtonClickedLancerVersionNormale(wxCommandEvent &event);
        void OnButtonClickedLancerVersion64bits(wxCommandEvent &event);
        void OnButtonClickedLancerVersionServeur(wxCommandEvent &event);

        void OnButtonClickedLancerIndev(wxCommandEvent &event);
        void OnButtonClickedLancerSiteWeb(wxCommandEvent &event);
        void OnButtonClickedLancerQuitter(wxCommandEvent &event);

        DECLARE_EVENT_TABLE()
};


#endif // LC_MAIN_H
