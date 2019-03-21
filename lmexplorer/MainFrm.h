// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include <atlimage.h>

class CMainFrame : 
	public CRibbonFrameWindowImpl<CMainFrame>, 
	public CMessageFilter, public CIdleHandler
{
public:
	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

	CView m_view;
	CCommandBarCtrl m_CmdBar;
	CRibbonCommandCtrl<wtl_LMICE_SCRIPT> m_ribbon1;
	//HBITMAP m_ribbonImg;
	//ICONINFOEX iconinfo;

	//TODO: Declare ribbon controls

	// Ribbon control map
	BEGIN_RIBBON_CONTROL_MAP(CMainFrame)
		RIBBON_CONTROL(m_ribbon1)
	END_RIBBON_CONTROL_MAP()

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		if(CRibbonFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
			return TRUE;

		return m_view.PreTranslateMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		return FALSE;
	}

	BEGIN_MSG_MAP(CMainFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
		COMMAND_ID_HANDLER(ID_FILE_NEW, OnFileNew)
		COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
		COMMAND_ID_HANDLER(ID_VIEW_RIBBON, OnViewRibbon)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		CHAIN_MSG_MAP(CRibbonFrameWindowImpl<CMainFrame>)
		
		RIBBON_COLOR_CONTROL_HANDLER(wtl_LMICE_SCRIPT, OnRibbonCommandLmiceScript)
	END_MSG_MAP()
		LRESULT OnRibbonCommandLmiceScript(UI_EXECUTIONVERB verb, WORD wID, COLORREF color, BOOL& bHandled) {
			MessageBox(L"welcome");
			return 0;
		}

		HRESULT SaveBitmap(HBITMAP hIcon, const wchar_t* path) {
			// Create the IPicture intrface
			PICTDESC desc = { sizeof(PICTDESC) };
			desc.picType = PICTYPE_BITMAP;
			desc.bmp.hbitmap = hIcon;
			IPicture* pPicture = 0;
			HRESULT hr = OleCreatePictureIndirect(&desc, IID_IPicture, FALSE, (void**)&pPicture);
			if (FAILED(hr)) return hr;

			// Create a stream and save the image
			IStream* pStream = 0;
			CreateStreamOnHGlobal(0, TRUE, &pStream);
			LONG cbSize = 0;
			hr = pPicture->SaveAsFile(pStream, TRUE, &cbSize);

			// Write the stream content to the file
			if (!FAILED(hr)) {
				HGLOBAL hBuf = 0;
				GetHGlobalFromStream(pStream, &hBuf);
				void* buffer = GlobalLock(hBuf);
				HANDLE hFile = CreateFile(path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
				if (!hFile) hr = HRESULT_FROM_WIN32(GetLastError());
				else {
					DWORD written = 0;
					WriteFile(hFile, buffer, cbSize, &written, 0);
					CloseHandle(hFile);
				}
				GlobalUnlock(buffer);
			}
			// Cleanup
			pStream->Release();
			pPicture->Release();
			return hr;

		}
		HRESULT SaveIcon(HICON hIcon, const wchar_t* path) {
			// Create the IPicture intrface
			PICTDESC desc = { sizeof(PICTDESC) };
			desc.picType = PICTYPE_ICON;
			desc.icon.hicon = hIcon;
			IPicture* pPicture = 0;
			HRESULT hr = OleCreatePictureIndirect(&desc, IID_IPicture, FALSE, (void**)&pPicture);
			if (FAILED(hr)) return hr;

			// Create a stream and save the image
			IStream* pStream = 0;
			CreateStreamOnHGlobal(0, TRUE, &pStream);
			LONG cbSize = 0;
			hr = pPicture->SaveAsFile(pStream, TRUE, &cbSize);

			// Write the stream content to the file
			if (!FAILED(hr)) {
				HGLOBAL hBuf = 0;
				GetHGlobalFromStream(pStream, &hBuf);
				void* buffer = GlobalLock(hBuf);
				HANDLE hFile = CreateFile(path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
				if (!hFile) hr = HRESULT_FROM_WIN32(GetLastError());
				else {
					DWORD written = 0;
					WriteFile(hFile, buffer, cbSize, &written, 0);
					CloseHandle(hFile);
				}
				GlobalUnlock(buffer);
			}
			// Cleanup
			pStream->Release();
			pPicture->Release();
			return hr;

		}
// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		m_hWndClient = m_view.Create(m_hWnd);

		//// create command bar window
		HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
		//// atach menu
		m_CmdBar.AttachMenu(GetMenu());
		//// load command bar images
		m_CmdBar.LoadImages(IDR_MAINFRAME);
		//// remove old menu
		SetMenu(NULL);

		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);

		bool bRibbonUI = RunTimeHelper::IsRibbonUIAvailable();
		if (bRibbonUI)
		{
			// UI Setup and adjustments
			UIAddMenu(m_CmdBar.GetMenu(), true);
			UIRemoveUpdateElement(ID_FILE_MRU_FIRST);
		}
		else
			CMenuHandle(m_CmdBar.GetMenu()).DeleteMenu(ID_VIEW_RIBBON, MF_BYCOMMAND);

		// Toolbar
		HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);

		CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
		AddSimpleReBarBand(hWndCmdBar);
		AddSimpleReBarBand(hWndToolBar, NULL, TRUE);

		CreateSimpleStatusBar();
		//m_sbar.SubclassWindow(m_hWndStatusBar);
		
		// Update UI & Checkbox status
		UIAddToolBar(hWndToolBar);
		// UISetCheck(ID_VIEW_TOOLBAR, 1);
		UISetCheck(ID_VIEW_STATUS_BAR, 1);

		

		if (bRibbonUI)
		{

			//HICON himg = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 48,48, LR_SHARED);
			
			//HICON himg = AtlLoadIconImage(L"D:\\work\\lmalpha\\lmexplorer\\res\\lmexplorer.ico", 
				//LR_COLOR|LR_LOADFROMFILE|LR_SHARED| LR_CREATEDIBSECTION, 32, 32);
			
				//HICON himg = AtlLoadSysIcon(IDI_QUESTION);

			//SaveIcon(himg, L"d:\\test.ico");
			//HICON himg = LoadIcon(NULL, IDI_APPLICATION);
			//ICONINFO iconinfo;

			//ICONINFOEX iconinfo;
			//memset((void*)&iconinfo, 0, sizeof(ICONINFOEX));
			//iconinfo.cbSize = sizeof(ICONINFOEX);
			//GetIconInfoEx(himg, &iconinfo);
			//HBITMAP hbmp = iconinfo.hbmColor;
			//SaveBitmap(hbmp, L"D:\\work\\lmalpha\\win64\\x64\\Release\\test2.bmp");

			//hbmp = (HBITMAP)LoadImage(NULL, L"d:\\test.bmp", IMAGE_BITMAP, 0 ,0, LR_LOADFROMFILE| LR_SHARED);
			//SaveBitmap(hbmp, L"D:\\work\\lmalpha\\win64\\x64\\Release\\test2.bmp");

			//hbmp = GetCommandBarBitmap(ID_APP_ABOUT);
			//wchar_t msg[128];
			//wsprintf(msg, L"msg icon(%p) image %p", himg, hbmp);
			//MessageBox(msg);
			

			m_ribbon1.SetText(UI_PKEY_TooltipTitle, L"ribon title");
			m_ribbon1.SetText(UI_PKEY_TooltipDescription, L"ribon <b>test</b>");

			//m_ribbon1.SetImage(UI_PKEY_SmallImage, hbmp);
			//m_ribbon1.SetImage(UI_PKEY_SmallImage, AtlLoadBitmapImage(IDR_MAINFRAME, LR_CREATEDIBSECTION));

			//HBITMAP hbmp = AtlLoadBitmapImage(L"D:\\work\\lmalpha\\win64\\x64\\Release\\test2.bmp", 
			//	LR_LOADFROMFILE);
			


			//m_ribbon1.m_hbm[0].Attach(hbmp);
			//m_ribbon1.m_hbm[1].Attach(hbmp);
			//m_ribbon1.m_hbm[2].Attach(hbmp);
			//m_ribbon1.m_hbm[3].Attach(hbmp);
			//m_ribbon1.GetWndRibbon().InvalidateProperty(m_ribbon1.GetID(), UI_PKEY_LargeImage);
			//m_ribbon1.GetWndRibbon().InvalidateProperty(m_ribbon1.GetID(), UI_PKEY_LargeHighContrastImage);
			//m_ribbon1.GetWndRibbon().InvalidateProperty(m_ribbon1.GetID(), UI_PKEY_SmallImage);
			//m_ribbon1.GetWndRibbon().InvalidateProperty(m_ribbon1.GetID(), UI_PKEY_SmallHighContrastImage);
			

		}
		//PROPVARIANT pv;
		//m_ribbon1.OnGetImage(UI_PKEY_LargeImage, &pv);
		//HBITMAP hbmp;
		//((IUIImage*)pv.punkVal)->GetBitmap(&hbmp);
		//SaveBitmap(hbmp, L"D:\\work\\lmalpha\\win64\\x64\\Release\\test3.bmp");

		// Show Ribbon
		ShowRibbonUI(bRibbonUI);
		UISetCheck(ID_VIEW_RIBBON, bRibbonUI);
		

		/** 让Ribbon Command 生效/失效 */
		//PROPVARIANT pv;
		//UIInitPropertyFromBoolean(UI_PKEY_Enabled, (VARIANT_FALSE), &pv);
		//GetIUIFrameworkPtr()->SetUICommandProperty(wtl_LMICE_SCRIPT, UI_PKEY_Enabled, pv);
		//PropVariantClear(&pv);
		m_ribbon1.SetProperty(UI_PKEY_Enabled, VARIANT_FALSE);

		/** 让 Command 属性 失效 */
		//GetIUIFrameworkPtr()->InvalidateUICommand(wtl_LMICE_SCRIPT, UI_INVALIDATIONS_PROPERTY, &UI_PKEY_SmallImage);
		
		return 0;
	}
	STDMETHODIMP CMainFrame::UpdateProperty1(
		UINT32 uCmdID, REFPROPERTYKEY key, const PROPVARIANT* pCurrVal,
		PROPVARIANT* pNewVal) {
		bool bHandled = false;
		switch (uCmdID) {
		case wtl_LMICE_SCRIPT:
			
			if (UI_PKEY_LargeImage == key || UI_PKEY_SmallImage == key) {
				//return S_OK;
				bool bLargeIcon = (key == UI_PKEY_LargeImage) ? true : false;
				int cx, cy;
				cx =  GetSystemMetrics(bLargeIcon ? SM_CXICON : SM_CXSMICON);
				cy = GetSystemMetrics(bLargeIcon ? SM_CYICON : SM_CYSMICON);


				//HICON icon = AtlLoadIconImage(L"D:\\work\\lmalpha\\lmexplorer\\res\\analysis.ico",
				//LR_LOADFROMFILE | LR_DEFAULTCOLOR|LR_SHARED, cx, cy);
				
				//HICON icon = (HICON)LoadImage(NULL,
				//	L"D:\\work\\lmalpha\\lmexplorer\\res\\lmexplorer.ico",
				//	IMAGE_ICON,
				//	cx,
				//	cy,
				//	LR_LOADFROMFILE| LR_DEFAULTSIZE);
				
				//HICON icon = AtlLoadSysIcon(IDI_HAND);
				
				//HICON icon = AtlLoadIconImage(IDI_ANALYSIS, 0, cx, cy);
				HICON icon = AtlLoadIconImage(IDI_ANALYSIS, 0, cx, cy);

				HRESULT hr;
				// Draw the icon into a 32bpp CImage.
				CImage img;

				img.Create(cx, cy, 32, CImage::createAlphaChannel);

				BOOL bdraw = DrawIconEx(CImageDC(img), 0, 0, icon, cx, cy, 0, 0, DI_NORMAL);

				// Create a UIRibbonImageFromBitmapFactory COM object and get an
				// IUIImageFromBitmap interface.
				CComPtr<IUIImageFromBitmap> pifb;

				hr = pifb.CoCreateInstance(CLSID_UIRibbonImageFromBitmapFactory);
				//if (FAILED(hr)) return false;

				// Create a new IUIImage, telling it to copy the HBITMAP that we pass in.
				CComPtr<IUIImage> pImage;

				hr = pifb->CreateImage(img, UI_OWNERSHIP_COPY, &pImage);
				//if (FAILED(hr)) return false;

				// Return the image to the Ribbon.
				hr = UIInitPropertyFromInterface(key, pImage, pNewVal);
				
				bHandled = true;
				MessageBox(L"Get UI");
				//GetIUIFrameworkPtr()->InvalidateUICommand(uCmdID, UI_INVALIDATIONS_PROPERTY, &key);
				
			}
			break;
		default:
			break;
		}
		// We don't respond to queries for other buttons or properties.
		return bHandled ? S_OK : DoUpdateProperty(uCmdID, key, pCurrVal, pNewVal);
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		// unregister message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->RemoveMessageFilter(this);
		pLoop->RemoveIdleHandler(this);

		bHandled = FALSE;
		return 1;
	}

	LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		PostMessage(WM_CLOSE);
		return 0;
	}

	LRESULT OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		// TODO: add code to initialize document
		//bool bLargeIcon = false;
		auto setImage = [&](bool islarge) {
			int cx, cy;
			cx = GetSystemMetrics(islarge ? SM_CXICON : SM_CXSMICON);
			cy = GetSystemMetrics(islarge ? SM_CYICON : SM_CYSMICON);

			HICON icon = AtlLoadIconImage(IDI_ANALYSIS, 0, cx, cy);
			ICONINFOEX iconinfo;
			memset((void*)&iconinfo, 0, sizeof(ICONINFOEX));
			iconinfo.cbSize = sizeof(ICONINFOEX);
			GetIconInfoEx(icon, &iconinfo);
			HBITMAP hbmp = iconinfo.hbmColor;
			if (islarge) {
				m_ribbon1.SetImage(UI_PKEY_LargeImage, hbmp,true);
			} else
			m_ribbon1.SetImage(UI_PKEY_SmallImage, hbmp,true);
			
			//DeleteObject(iconinfo.hbmColor);
			DeleteObject(iconinfo.hbmMask);
			DestroyIcon(icon);
			
		};

		setImage(true);
		setImage(false);
		//m_ribbon1.Invalidate();
		//GetIUIFrameworkPtr()->InvalidateUICommand(
		//	wtl_LMICE_SCRIPT, 
		//	UI_INVALIDATIONS_PROPERTY, 
		//	&UI_PKEY_SmallImage);
		
		m_ribbon1.SetProperty(UI_PKEY_Enabled, VARIANT_TRUE);
		int cx, cy;
		cx = GetSystemMetrics( SM_CXICON );
		cy = GetSystemMetrics(SM_CXSMICON);
		wchar_t buffer[512];
		memset(buffer, 0, sizeof(buffer));
		wsprintf(buffer, L"size %d  %d\n", cx, cy);
		MessageBox(buffer);
		return 0;
	}

	
	
	LRESULT OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
		::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
		UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
		UpdateLayout();
		return 0;
	}

	LRESULT OnViewRibbon(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		ShowRibbonUI(!IsRibbonUI());
		UISetCheck(ID_VIEW_RIBBON, IsRibbonUI());
		return 0;
	}

	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CAboutDlg dlg;
		dlg.DoModal();
		return 0;
	}
};


//HRESULT CMainFrame::CRibbonCommandCtrl<wtl_LMICE_SCRIPT>::DoUpdateProperty(UINT nCmdID, REFPROPERTYKEY key,
//
//	const PROPVARIANT* ppropvarCurrentValue, PROPVARIANT* ppropvarNewValue) {
//
//}