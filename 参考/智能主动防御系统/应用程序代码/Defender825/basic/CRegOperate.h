// 注册表操作类
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
	// 枚举子项
	// 参数:KeyPath-要枚举的项的路径
	// 返回值：枚举到的子项的数目
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

		//清空保存结果的数组
		m_RegArray.RemoveAll();

		//打开指定的子项,返回打开的句柄保存在hkey中
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
	// 枚举指定项下的键
	// 参数:KeyPath-要枚举的键的的路径
	// 返回值：枚举到的键值的数目
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
		
		//清空保存结果的数组
		m_RegArray.RemoveAll();
		
		//打开指定的子项,返回打开的句柄保存在hkey中
		if (::RegOpenKeyEx(hKey, lpSubKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
			return 0;
	
		DWORD dwIndex = 0;
		LONG lRet;
		DWORD cbName = 256;
		TCHAR szSubValueName[256];
		
		//枚举指定键下的子键
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
					if(REG_SZ==dwType) //(REG_SZ类型)
					{
						m_RegArray.Add(szSubValueName); //键名
						m_RegArray.Add(szDisplayValue);	//键值	
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
	// 删除指定项
	// 参数:KeyPath-要删除的项的路径
	// 返回值：成功删除返回0,反之返回-1
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
		if(IDNO==::MessageBox(0,"确定要删除该项吗?","智能主动防御系统",MB_YESNO))
		   return -1;

		//删除指定项
		if (::RegDeleteKey(hKey, lpSubKey) != ERROR_SUCCESS)
			return -1;
		return 0;
	}

	//-------------------------------------------------------------------------------------
	// DeleteValue
	// 删除指定键
	// 参数:KeyPath-要删除的键的路径 lpValueName-键名称
	// 返回值：成功删除返回0,反之返回-1
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
		//if(IDNO==::MessageBox(0,"确定要删除该项吗?","智能主动防御系统",MB_YESNO))
		//  return -1;
	
		//打开指定的子项,返回打开的句柄保存在hkey中
		if (::RegOpenKeyEx(hKey, lpSubKey, 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
			return 0;

		//删除指定键
		if (::RegDeleteValue(hKey, lpValueName) != ERROR_SUCCESS)
			return -1;
		return 0;
	}
	//-------------------------------------------------------------------------------------
	// GetKeyValue
	// 获取指定键值
	// 参数:KeyPath-要获取的键的路径 lpValueName-键名称
	// 返回值：成功删除返回0,反之返回-1
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
	
		//清空保存结果的数组
		m_RegArray.RemoveAll();

		//打开指定的子项,返回打开的句柄保存在hkey中
		if (::RegOpenKeyEx(hKey, lpSubKey, 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
			return -1;

		TCHAR szDisplayValue[256];
		DWORD dwSize = 256;
		DWORD dwType;
		
		if (::RegQueryValueEx(hKey, lpValueName, NULL, &dwType,
			(LPBYTE)szDisplayValue, &dwSize) == ERROR_SUCCESS)
		{
			if(REG_SZ==dwType) //(REG_SZ类型)
			{
				m_RegArray.Add(szDisplayValue);	//键值	
			}
		}
        return 0;
	}

};
