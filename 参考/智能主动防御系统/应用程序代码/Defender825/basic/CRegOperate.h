// ע��������
class CRegOperate
{
public:
	CStringArray m_RegArray;
public:
	CRegOperate(void)
	{
	}

	//-------------------------------------------------------------------------------------
	// EnumKey
	// ö������
	// ����:KeyPath-Ҫö�ٵ����·��
	// ����ֵ��ö�ٵ����������Ŀ
	//--------------------------------------------------------------------------------------
	int EnumKey(LPCTSTR KeyPath)
	{
		HKEY hKey;
		LPCTSTR lpSubKey;
		if(!strncmp(KeyPath,"HKCU",4))
			hKey=HKEY_CURRENT_USER;
		else if(!strncmp(KeyPath,"HKLM",4))
			hKey=HKEY_LOCAL_MACHINE;
		else if(!strncmp(KeyPath,"HKCR",4))
			hKey=HKEY_CLASSES_ROOT;
		else if(!strncmp(KeyPath,"HKCC",4))
			hKey=HKEY_CURRENT_CONFIG;
		else
			hKey=HKEY_USERS;				
		lpSubKey=KeyPath+5;

		//��ձ�����������
		m_RegArray.RemoveAll();

		//��ָ��������,���ش򿪵ľ��������hkey��
		if (::RegOpenKeyEx(hKey, lpSubKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
			return 0;

		DWORD dwIndex = 0;
		LONG lRet;
		DWORD cbName = 256;
		TCHAR szSubKeyName[256];
		
		while ((lRet = ::RegEnumKeyEx(hKey, dwIndex, szSubKeyName, &cbName, NULL,
			NULL, NULL, NULL)) != ERROR_NO_MORE_ITEMS)
		{
			if (lRet == ERROR_SUCCESS)
			{
				m_RegArray.Add(szSubKeyName);					
			}
			dwIndex++;
			cbName =256;
		}
		::RegCloseKey(hKey);
		return m_RegArray.GetSize();
	}

	//-------------------------------------------------------------------------------------
	// EnumValue
	// ö��ָ�����µļ�
	// ����:KeyPath-Ҫö�ٵļ��ĵ�·��
	// ����ֵ��ö�ٵ��ļ�ֵ����Ŀ
	//--------------------------------------------------------------------------------------
	int EnumValue(LPCTSTR KeyPath)
	{	
		HKEY hKey;
		LPCTSTR lpSubKey;
		if(!strncmp(KeyPath,"HKCU",4))
			hKey=HKEY_CURRENT_USER;
		else if(!strncmp(KeyPath,"HKLM",4))
			hKey=HKEY_LOCAL_MACHINE;
		else if(!strncmp(KeyPath,"HKCR",4))
			hKey=HKEY_CLASSES_ROOT;
		else if(!strncmp(KeyPath,"HKCC",4))
			hKey=HKEY_CURRENT_CONFIG;
		else
			hKey=HKEY_USERS;				
		lpSubKey=KeyPath+5;
		
		//��ձ�����������
		m_RegArray.RemoveAll();
		
		//��ָ��������,���ش򿪵ľ��������hkey��
		if (::RegOpenKeyEx(hKey, lpSubKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
			return 0;
	
		DWORD dwIndex = 0;
		LONG lRet;
		DWORD cbName = 256;
		TCHAR szSubValueName[256];
		
		//ö��ָ�����µ��Ӽ�
		while((lRet = ::RegEnumValue(hKey,dwIndex,szSubValueName,&cbName, 
                NULL,NULL,NULL,NULL))!=ERROR_NO_MORE_ITEMS) 
        { 
            if(lRet==ERROR_SUCCESS ) 
            { 
				TCHAR szDisplayValue[256];
				DWORD dwSize = 256;
				DWORD dwType;
				
				if (::RegQueryValueEx(hKey, szSubValueName, NULL, &dwType,
					(LPBYTE)szDisplayValue, &dwSize) == ERROR_SUCCESS)
				{
					if(REG_SZ==dwType) //(REG_SZ����)
					{
						m_RegArray.Add(szSubValueName); //����
						m_RegArray.Add(szDisplayValue);	//��ֵ	
					}
				}

			}
			dwIndex++;
			cbName =256;
		}
		return m_RegArray.GetSize();
	}

	//-------------------------------------------------------------------------------------
	// DeleteKey
	// ɾ��ָ����
	// ����:KeyPath-Ҫɾ�������·��
	// ����ֵ���ɹ�ɾ������0,��֮����-1
	//--------------------------------------------------------------------------------------
	int DeleteKey(LPCTSTR KeyPath)
	{
		HKEY hKey;
		LPCTSTR lpSubKey;
		if(!strncmp(KeyPath,"HKCU",4))
			hKey=HKEY_CURRENT_USER;
		else if(!strncmp(KeyPath,"HKLM",4))
			hKey=HKEY_LOCAL_MACHINE;
		else if(!strncmp(KeyPath,"HKCR",4))
			hKey=HKEY_CLASSES_ROOT;
		else if(!strncmp(KeyPath,"HKCC",4))
			hKey=HKEY_CURRENT_CONFIG;
		else
			hKey=HKEY_USERS;				
		lpSubKey=KeyPath+5;
		if(IDNO==::MessageBox(0,"ȷ��Ҫɾ��������?","������������ϵͳ",MB_YESNO))
		   return -1;

		//ɾ��ָ����
		if (::RegDeleteKey(hKey, lpSubKey) != ERROR_SUCCESS)
			return -1;
		return 0;
	}

	//-------------------------------------------------------------------------------------
	// DeleteValue
	// ɾ��ָ����
	// ����:KeyPath-Ҫɾ���ļ���·�� lpValueName-������
	// ����ֵ���ɹ�ɾ������0,��֮����-1
	//--------------------------------------------------------------------------------------
	int DeleteValue(LPCTSTR KeyPath, LPCTSTR lpValueName)
	{
		HKEY hKey;
		LPCTSTR lpSubKey;
		if(!strncmp(KeyPath,"HKCU",4))
			hKey=HKEY_CURRENT_USER;
		else if(!strncmp(KeyPath,"HKLM",4))
			hKey=HKEY_LOCAL_MACHINE;
		else if(!strncmp(KeyPath,"HKCR",4))
			hKey=HKEY_CLASSES_ROOT;
		else if(!strncmp(KeyPath,"HKCC",4))
			hKey=HKEY_CURRENT_CONFIG;
		else
			hKey=HKEY_USERS;				
		lpSubKey=KeyPath+5;
		//if(IDNO==::MessageBox(0,"ȷ��Ҫɾ��������?","������������ϵͳ",MB_YESNO))
		//  return -1;
	
		//��ָ��������,���ش򿪵ľ��������hkey��
		if (::RegOpenKeyEx(hKey, lpSubKey, 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
			return 0;

		//ɾ��ָ����
		if (::RegDeleteValue(hKey, lpValueName) != ERROR_SUCCESS)
			return -1;
		return 0;
	}
	//-------------------------------------------------------------------------------------
	// GetKeyValue
	// ��ȡָ����ֵ
	// ����:KeyPath-Ҫ��ȡ�ļ���·�� lpValueName-������
	// ����ֵ���ɹ�ɾ������0,��֮����-1
	//--------------------------------------------------------------------------------------
	int GetKeyValue(LPCTSTR KeyPath, LPCTSTR lpValueName)
	{
		HKEY hKey;
		LPCTSTR lpSubKey;
		if(!strncmp(KeyPath,"HKCU",4))
			hKey=HKEY_CURRENT_USER;
		else if(!strncmp(KeyPath,"HKLM",4))
			hKey=HKEY_LOCAL_MACHINE;
		else if(!strncmp(KeyPath,"HKCR",4))
			hKey=HKEY_CLASSES_ROOT;
		else if(!strncmp(KeyPath,"HKCC",4))
			hKey=HKEY_CURRENT_CONFIG;
		else
			hKey=HKEY_USERS;				
		lpSubKey=KeyPath+5;
	
		//��ձ�����������
		m_RegArray.RemoveAll();

		//��ָ��������,���ش򿪵ľ��������hkey��
		if (::RegOpenKeyEx(hKey, lpSubKey, 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
			return -1;

		TCHAR szDisplayValue[256];
		DWORD dwSize = 256;
		DWORD dwType;
		
		if (::RegQueryValueEx(hKey, lpValueName, NULL, &dwType,
			(LPBYTE)szDisplayValue, &dwSize) == ERROR_SUCCESS)
		{
			if(REG_SZ==dwType) //(REG_SZ����)
			{
				m_RegArray.Add(szDisplayValue);	//��ֵ	
			}
		}
        return 0;
	}

};
