
#include "stdafx.h"
#include "SystemImageList.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CImageList * CSystemImageList::m_gpLargeImageList = NULL;
CImageList * CSystemImageList::m_gpSmallImageList = NULL;

int CSystemImageList::m_gRefCount = 0;

void CSystemImageList::DestroyGlobalList( CImageList *& rpImageList )
{
    if( rpImageList )
    {
		rpImageList->Detach();

		delete rpImageList;
		rpImageList = NULL;
    }
}

BOOL CSystemImageList::InitGlobalList( CImageList *& rpImageList, BOOL bSmall /* = TRUE */ )
{
    BOOL bRet = FALSE;

    if( ! rpImageList )
    {
		// Create the singleton CImageList object, which will be
		// passed around to anyone who needs it
		rpImageList = new CImageList;
    
		if( rpImageList )
		{
			SHFILEINFO sfi;
			DWORD dwFlags = SHGFI_USEFILEATTRIBUTES|SHGFI_SYSICONINDEX;
			
			if( bSmall )
				dwFlags |= SHGFI_SMALLICON;

			size_t size = sizeof( SHFILEINFO );

			rpImageList->Attach( (HIMAGELIST)SHGetFileInfo( NULL, FILE_ATTRIBUTE_NORMAL, 
						&sfi, size, dwFlags ) );
			
			// Make the background colour transparent, works better for lists etc.
			rpImageList->SetBkColor( CLR_NONE );

			bRet = TRUE;
		}
    }

    return( bRet );
}

CSystemImageList::CSystemImageList()
{
    // Initialise both lists when the first instance is created
    if( m_gRefCount == 0 )
    {
		ASSERT( ! m_gpSmallImageList && ! m_gpLargeImageList );

		InitGlobalList( m_gpSmallImageList, TRUE );
		InitGlobalList( m_gpLargeImageList, FALSE );
    }

    // Keep a reference count
    m_gRefCount++;
}

CSystemImageList::~CSystemImageList()
{
    // One less instance using the lists
    m_gRefCount--;

    // Nobody's using the list now, so throw 'em away
    if( m_gRefCount == 0 )
    {
		DestroyGlobalList( m_gpLargeImageList );
		DestroyGlobalList( m_gpSmallImageList );
    }
}

// Helper function to get the icon index of a particular file/object
HICON CSystemImageList::GetIcon( const CString& sName, BOOL bOpen /* = FALSE */,  BOOL bSmall /* = TRUE */  ) const
{
    SHFILEINFO sfi;

    DWORD dwFlags = SHGFI_DISPLAYNAME | SHGFI_ICON | SHGFI_LARGEICON;
    size_t size = sizeof( SHFILEINFO );

    if( bSmall )
		dwFlags |= SHGFI_SMALLICON;
    
    if( bOpen )
		dwFlags |= SHGFI_OPENICON;
    
    SHGetFileInfo( sName, NULL , &sfi, size, dwFlags);

    return( sfi.hIcon );
}

CImageList * CSystemImageList::GetImageList( BOOL bSmall /* = TRUE */ ) const 
{
    CImageList * pList = NULL;
    
    if( bSmall )
		pList = m_gpSmallImageList;
    else
		pList = m_gpLargeImageList;

    // Ought to return a const pointer here (so noone tries to delete
    // the lists). However, SetImageList in the common controls requires
    // a non-const pointer, so it wouldn't be much use to anyone.
    return( pList ); 
}
