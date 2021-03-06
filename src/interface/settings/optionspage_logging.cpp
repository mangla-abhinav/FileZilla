#include "../filezilla.h"
#include "../Options.h"
#include "settingsdialog.h"
#include "optionspage.h"
#include "optionspage_logging.h"
#include "../xrc_helper.h"

#include <wx/filedlg.h>

BEGIN_EVENT_TABLE(COptionsPageLogging, COptionsPage)
EVT_BUTTON(XRCID("ID_BROWSE"), COptionsPageLogging::OnBrowse)
EVT_CHECKBOX(XRCID("ID_LOGFILE"), COptionsPageLogging::OnCheck)
EVT_CHECKBOX(XRCID("ID_DOLIMIT"), COptionsPageLogging::OnCheck)
END_EVENT_TABLE()

bool COptionsPageLogging::LoadPage()
{
	bool failure = false;

	SetCheck(XRCID("ID_TIMESTAMPS"), m_pOptions->get_int(OPTION_MESSAGELOG_TIMESTAMP) ? true : false, failure);

	std::wstring const filename = m_pOptions->get_string(OPTION_LOGGING_FILE);
	SetCheck(XRCID("ID_LOGFILE"), !filename.empty(), failure);
	SetText(XRCID("ID_FILENAME"), filename, failure);

	int limit = m_pOptions->get_int(OPTION_LOGGING_FILE_SIZELIMIT);
	if (limit < 0 || limit > 2000) {
		limit = 0;
	}
	SetCheck(XRCID("ID_DOLIMIT"), limit > 0, failure);
	if (!failure) {
		XRCCTRL(*this, "ID_LIMIT", wxTextCtrl)->SetMaxLength(4);
	}

	std::wstring v;
	if (limit > 0) {
		v = fz::to_wstring(limit);
	}
	SetText(XRCID("ID_LIMIT"), v, failure);

	if (!failure) {
		SetCtrlState();
	}

	return !failure;
}

bool COptionsPageLogging::SavePage()
{
	m_pOptions->set(OPTION_MESSAGELOG_TIMESTAMP, GetCheck(XRCID("ID_TIMESTAMPS")) ? 1 : 0);

	wxString filename;
	if (GetCheck(XRCID("ID_LOGFILE"))) {
		filename = GetText(XRCID("ID_FILENAME"));
	}
	m_pOptions->set(OPTION_LOGGING_FILE, filename.ToStdWstring());

	if (GetCheck(XRCID("ID_DOLIMIT"))) {
		m_pOptions->set(OPTION_LOGGING_FILE_SIZELIMIT, xrc_call(*this, "ID_LIMIT", &wxTextCtrl::GetValue).ToStdWstring());
	}
	else {
		m_pOptions->set(OPTION_LOGGING_FILE_SIZELIMIT, 0);
	}

	return true;
}

bool COptionsPageLogging::Validate()
{
	bool log_to_file = GetCheck(XRCID("ID_LOGFILE"));
	bool limit = GetCheck(XRCID("ID_DOLIMIT"));

	if (log_to_file) {
		wxTextCtrl *pFileName = XRCCTRL(*this, "ID_FILENAME", wxTextCtrl);
		if (pFileName->GetValue().empty()) {
			return DisplayError(pFileName, _("You need to enter a name for the log file."));
		}

		wxFileName fn(pFileName->GetValue());
		if (!fn.IsOk() || !fn.DirExists()) {
			return DisplayError(pFileName, _("Directory containing the logfile does not exist or filename is invalid."));
		}

		if (limit) {
			unsigned long v;
			wxTextCtrl *pLimit = XRCCTRL(*this, "ID_LIMIT", wxTextCtrl);
			if (!pLimit->GetValue().ToULong(&v) || v < 1 || v > 2000) {
				return DisplayError(pLimit, _("The limit needs to be between 1 and 2000 MiB"));
			}
		}
	}
	return true;
}

void COptionsPageLogging::SetCtrlState()
{
	bool log_to_file = GetCheck(XRCID("ID_LOGFILE"));
	bool limit = GetCheck(XRCID("ID_DOLIMIT"));

	XRCCTRL(*this, "ID_FILENAME", wxTextCtrl)->Enable(log_to_file);
	XRCCTRL(*this, "ID_BROWSE", wxButton)->Enable(log_to_file);
	XRCCTRL(*this, "ID_DOLIMIT", wxCheckBox)->Enable(log_to_file);
	XRCCTRL(*this, "ID_LIMIT", wxTextCtrl)->Enable(log_to_file && limit);
}

void COptionsPageLogging::OnBrowse(wxCommandEvent&)
{
	wxFileDialog dlg(this, _("Log file"), wxString(), _T("filezilla.log"), _T("Log files (*.log)|*.log"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	if (dlg.ShowModal() != wxID_OK) {
		return;
	}

	XRCCTRL(*this, "ID_FILENAME", wxTextCtrl)->ChangeValue(dlg.GetPath());
}

void COptionsPageLogging::OnCheck(wxCommandEvent&)
{
	SetCtrlState();
}
