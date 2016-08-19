#pragma once

#include "Debug.h"
#include "bits.h"
#include "Symbols.h"

#define TXOFFSET 16

// Global node index
extern DWORD NodeCreateIndex;

// Forward declarations
class CMemory;
class CNodeBase;
class CNodeIcon;
class CNodeClass;
class CNodeHex64;
class CNodeHex32;
class CNodeHex16;
class CNodeHex8;
class CNodeBits;
class CNodeVTable;
class CNodeFunctionPtr;
class CNodePtr;
class CNodeInt64;
class CNodeInt32;
class CNodeInt8;
class CNodeQWORD;
class CNodeDWORD;
class CNodeWORD;
class CNodeByte;
class CNodeText;
class CNodeCharPtr;
class CNodeWCharPtr;
class CNodeUnicode;
class CNodeFloat;
class CNodeDouble;
class CNodeVec2;
class CNodeVec3;
class CNodeQuat;
class CNodeMatrix;
class CNodeArray;
class CNodeClassInstance;
class CNodeCustom;

#include "ChildFrm.h"

size_t ConvertStrToAddress(CString Address);

struct ViewInfo
{
	CDC* dc;
	CRect* client;
	std::vector<HotSpot>* HotSpots;
	std::vector<CNodeClass*>* Classes;
	size_t Address;
	unsigned char* pData;
	UINT Level;
	bool bMultiSelected;
};

class CNodeBase
{
public:
	CNodeBase()
	{
		nodeType = nt_base;
		pParent = NULL;
		offset = 0;
		strOffset = "0";
		bHidden = false;
		// Optimized
		bOpen.resize(32, false);
		bOpen[0] = true;

		bSelected = false;
		// This is the class name
		Name.Format(_T("N%0.8X"), NodeCreateIndex);
		NodeCreateIndex++;		
	}

	~CNodeBase() { }

	virtual int Draw(ViewInfo& View, int x, int y) = 0;
	virtual int GetMemorySize() = 0;
	virtual void Update(HotSpot& Spot) = 0;

	NodeType GetType() { return nodeType; }
	size_t GetOffset() { return offset; }

	NodeType nodeType;

	size_t offset;
	CString strOffset;

	CString Name;
	CString Comment;
	CNodeBase* pParent;
	std::vector<CNodeBase*> Nodes;

	bool bHidden;
	bool bSelected;
	std::vector<bool> bOpen;

	// Incorrect view.address
	void AddHotSpot(ViewInfo& View, CRect& Spot, CString Text, int ID, int Type)
	{
		if (Spot.top > View.client->bottom || Spot.bottom < 0) 
			return;

		HotSpot spot;
		spot.Rect = Spot;
		spot.Text = Text;
		spot.Address = View.Address + offset;
		spot.ID = ID;
		spot.Type = Type;
		spot.object = this;
		spot.Level = View.Level;
		View.HotSpots->push_back(spot);
	}

	int AddText(ViewInfo& View, int x, int y, DWORD color, int HitID, const wchar_t *fmt, ...)
	{
		if ( fmt == NULL )
			return x;

		wchar_t wcsbuf[1024];

		va_list va_alist;
		va_start(va_alist, fmt);
		_vsnwprintf(wcsbuf, sizeof(wcsbuf), fmt, va_alist);
		va_end(va_alist);

		int width = static_cast<int>(wcslen(wcsbuf)) * g_FontWidth;

		if ((y >= -(g_FontHeight)) && (y + g_FontHeight <= View.client->bottom + g_FontHeight))
		{
			CRect pos;

			if (HitID != HS_NONE)
			{
				if (width >= g_FontWidth * 2)
					pos.SetRect(x, y, x + width, y + g_FontHeight);
				else
					pos.SetRect(x, y, x + g_FontWidth * 2, y + g_FontHeight);

				AddHotSpot(View, pos, wcsbuf, HitID, HS_EDIT);
			}

			pos.SetRect(x, y, 0, 0);
			View.dc->SetTextColor(color);
			View.dc->SetBkMode(TRANSPARENT);
			View.dc->DrawText(wcsbuf, pos, DT_LEFT | DT_NOCLIP | DT_NOPREFIX);
		}

		return x + width;
	}

	int AddText(ViewInfo& View, int x, int y, DWORD color, int HitID, const char* fmt, ...)
	{
		char buffer[ 1024 ] = { 0 };
		TCHAR finalBuffer[ 1024 ] = { 0 };

		va_list va_alist;
		size_t converted;

		if (fmt == NULL)
			return x;

		va_start(va_alist, fmt);
		_vsnprintf_s(buffer, 1024, fmt, va_alist);
		va_end(va_alist);

		#ifdef UNICODE
		mbstowcs_s(&converted, finalBuffer, buffer, 1024);
		#else
		memcpy(&finalBuffer, buffer, 1024);
		#endif

		int width = static_cast<int>(strlen(buffer)) * g_FontWidth;

		if ((y >= -g_FontHeight) && (y + g_FontHeight <= View.client->bottom + g_FontHeight))
		{
			CRect pos;
			if (HitID != HS_NONE)
			{
				if (width >= g_FontWidth * 2)
					pos.SetRect(x, y, x + width, y + g_FontHeight);
				else
					pos.SetRect(x, y, x + g_FontWidth * 2, y + g_FontHeight);

				AddHotSpot(View, pos, finalBuffer, HitID, HS_EDIT);
			} 

			pos.SetRect(x, y, 0, 0);
			View.dc->SetTextColor(color);
			View.dc->SetBkMode(TRANSPARENT);
			View.dc->DrawText(finalBuffer, pos, DT_LEFT | DT_NOCLIP | DT_NOPREFIX);
		}

		return x + width;
	}

	int AddAddressOffset(ViewInfo& View, int x, int y)
	{
		if (gbOffset)
		{
			#ifdef _WIN64
			// goto 722
			// just the left side 0000
			// TODO: fix the ghetto rig FontWidth * x
			// where x = characters over 8
			//x += FontWidth; // we need this either way
			//int numdigits = Utils::NumDigits(View.Address);
			//if (numdigits < 8 && numdigits > 4)
			//	x -= ((8 - numdigits) * FontWidth);
			//if (numdigits > 8)
			//	x += ((numdigits - 8) * FontWidth);

			x = AddText(View, x, y, crOffset, HS_NONE, _T("%0.4X"), offset) + g_FontWidth;
			#else
			x = AddText(View, x, y, crOffset, HS_NONE, _T("%0.4X"), offset) + g_FontWidth;
			#endif
		}

		if (gbAddress)
		{
			#ifdef _WIN64
			x = AddText(View, x, y, crAddress, HS_ADDRESS, _T("%0.9I64X"), View.Address + offset) + g_FontWidth;
			#else
			x = AddText(View, x, y, crAddress, HS_ADDRESS, _T("%0.8X"), View.Address + offset) + g_FontWidth;
			#endif
		}

		return x;
	}

	void AddSelection(ViewInfo& View, int x, int y, int Height)
	{
		if ((y > View.client->bottom) || (y + Height < 0))
			return;

		if (bSelected)
			View.dc->FillSolidRect(0, y, View.client->right, Height, crSelect);

		CRect pos(0, y, 1024, y + Height);
		AddHotSpot(View, pos, CString(), 0, HS_SELECT);
	}

	int AddIcon(ViewInfo& View, int x, int y, int idx, int ID, int Type)
	{
		if ((y > View.client->bottom) || (y + 16 < 0))
			return x + 16;

		DrawIconEx(View.dc->m_hDC, x, y, Icons[idx], 16, 16, 0, NULL, DI_NORMAL);

		if (ID != -1)
		{
			CRect pos(x, y, x + 16, y + 16);
			AddHotSpot(View, pos, CString(), ID, Type);
		}

		return x + 16;
	}

	int AddOpenClose(ViewInfo& View, int x, int y)
	{
		if ((y > View.client->bottom) || (y + 16 < 0))
			return x + 16;
		return bOpen[View.Level] ? AddIcon(View, x, y, ICON_OPEN, 0, HS_OPENCLOSE) : AddIcon(View, x, y, ICON_CLOSED, 0, HS_OPENCLOSE);
	}

	void AddDelete(ViewInfo& View, int x, int y)
	{
		if ((y > View.client->bottom) || (y + 16 < 0))
			return;

		if (bSelected)
			AddIcon(View, View.client->right - 16, y, ICON_DELETE, 0, HS_DELETE);
	}

	//void AddAdd(ViewInfo& View,int x,int y)
	//{
	//	if ( (y > View.client->bottom) || (y+16 < 0) ) return;
	//	if (bSelected)AddIcon(View,16,y,ICON_ADD,HS_NONE,HS_NONE);
	//}

	void AddTypeDrop(ViewInfo& View, int x, int y)
	{
		if (View.bMultiSelected)
			return;
		if ((y > View.client->bottom) || (y + 16 < 0))
			return;

		if (bSelected)
			AddIcon(View, 0, y, ICON_DROPARROW, 0, HS_DROP);
	}

	int ResolveRTTI(size_t Val, int &x, ViewInfo& View, int y)
	{
	#ifdef _WIN64
		size_t ModuleBase = 0x0;
		//Find module Val is in, then get module base
		for (int i = 0; i < MemMapModule.size(); i++)
		{
			MemMapInfo MemInfo = MemMapModule[i];
			if (Val >= MemInfo.Start && Val <= MemInfo.End)
			{
				ModuleBase = MemInfo.Start;
				break;
			}
		}

		size_t pRTTIObjectLocator = Val - 8; //Val is Ptr to first VFunc, pRTTI is at -0x8
		if (!IsValidPtr(pRTTIObjectLocator))
			return x;

		size_t RTTIObjectLocator;
		ReClassReadMemory((LPVOID)pRTTIObjectLocator, &RTTIObjectLocator, sizeof(DWORD_PTR));

		DWORD dwTypeDescriptorOffset;
		ReClassReadMemory((LPVOID)(RTTIObjectLocator + 0x0C), &dwTypeDescriptorOffset, sizeof(DWORD));
		size_t TypeDescriptor = ModuleBase + dwTypeDescriptorOffset;

		DWORD dwObjectBaseOffset;
		ReClassReadMemory((LPVOID)(RTTIObjectLocator + 0x14), &dwObjectBaseOffset, sizeof(DWORD));
		size_t ObjectBase = ModuleBase + dwObjectBaseOffset;


		DWORD dwClassHierarchyDescriptorOffset;
		ReClassReadMemory((LPVOID)(RTTIObjectLocator + 0x10), &dwClassHierarchyDescriptorOffset, sizeof(DWORD));

		//Offsets are from base
		size_t ClassHierarchyDescriptor = ModuleBase + dwClassHierarchyDescriptorOffset;
		if (!IsValidPtr(ClassHierarchyDescriptor) || !dwClassHierarchyDescriptorOffset)
			return x;

		DWORD NumBaseClasses;
		ReClassReadMemory((LPVOID)(ClassHierarchyDescriptor + 0x8), &NumBaseClasses, sizeof(DWORD));
		if (NumBaseClasses < 0 || NumBaseClasses > 25)
			NumBaseClasses = 0;

		DWORD BaseClassArrayOffset;
		ReClassReadMemory((LPVOID)(ClassHierarchyDescriptor + 0xC), &BaseClassArrayOffset, sizeof(DWORD));

		size_t BaseClassArray = ModuleBase + BaseClassArrayOffset;
		if (!IsValidPtr(BaseClassArray) || !BaseClassArrayOffset)
			return x;

		//x = AddText(View, x, y, crOffset, HS_NONE, " RTTI:");
		CString RTTIString;
		for (unsigned int i = 0; i < NumBaseClasses; i++)
		{
			if (i != 0 && i != NumBaseClasses)
			{
				RTTIString += _T(" : ");
				//x = AddText(View, x, y, crOffset, HS_NONE, " inherits:");
			}

			DWORD BaseClassDescriptorOffset;
			ReClassReadMemory((LPVOID)(BaseClassArray + (0x4 * i)), &BaseClassDescriptorOffset, sizeof(DWORD));

			size_t BaseClassDescriptor = ModuleBase + BaseClassDescriptorOffset;
			if (!IsValidPtr(BaseClassDescriptor) || !BaseClassDescriptorOffset)
				continue;

			DWORD TypeDescriptorOffset;
			ReClassReadMemory((LPVOID)BaseClassDescriptor, &TypeDescriptorOffset, sizeof(DWORD));

			size_t TypeDescriptor = ModuleBase + TypeDescriptorOffset;
			if (!IsValidPtr(TypeDescriptor) || !TypeDescriptorOffset)
				continue;

			CString RTTIName;
			bool FoundEnd = false;
			char LastChar = ' ';
			for (int j = 1; j < 45; j++)
			{
				char RTTINameChar;
				ReClassReadMemory((LPVOID)(TypeDescriptor + 0x10 + j), &RTTINameChar, 1);
				if (RTTINameChar == '@' && LastChar == '@') //Names seem to be ended with @@
				{
					FoundEnd = true;
					RTTIName += RTTINameChar;
					break;
				}
				RTTIName += RTTINameChar;
				LastChar = RTTINameChar;
			}
			//Did we find a valid rtti name or did we just reach end of loop
			if (!FoundEnd)
				continue;

			TCHAR Demangled[MAX_PATH];
			if (_UnDecorateSymbolName(RTTIName, Demangled, MAX_PATH, UNDNAME_NAME_ONLY) == 0)
				RTTIString += RTTIName;
			else
				RTTIString += Demangled;
			//x = AddText(View, x, y, crOffset, HS_RTTI, "%s", RTTIName.c_str());
		}
		x = AddText(View, x, y, crOffset, HS_RTTI, RTTIString);
		return x; 
	#else	
		size_t pRTTIObjectLocator = Val - 4;
		if (!IsValidPtr(pRTTIObjectLocator))
			return x;

		size_t RTTIObjectLocator;
		ReClassReadMemory((LPVOID)pRTTIObjectLocator, &RTTIObjectLocator, sizeof(size_t));

		size_t pClassHierarchyDescriptor = RTTIObjectLocator + 0x10;
		if (!IsValidPtr(pClassHierarchyDescriptor))
			return x;

		size_t ClassHierarchyDescriptor;
		ReClassReadMemory((LPVOID)pClassHierarchyDescriptor, &ClassHierarchyDescriptor, sizeof(size_t));

		int NumBaseClasses;
		ReClassReadMemory((LPVOID)(ClassHierarchyDescriptor + 0x8), &NumBaseClasses, sizeof(int));
		if (NumBaseClasses < 0 || NumBaseClasses > 25)
			NumBaseClasses = 0;

		size_t pBaseClassArray = ClassHierarchyDescriptor + 0xC;
		if (!IsValidPtr(pBaseClassArray))
			return x;

		size_t BaseClassArray;
		ReClassReadMemory((LPVOID)pBaseClassArray, &BaseClassArray, sizeof(size_t));

		//x = AddText(View, x, y, crOffset, HS_NONE, " RTTI: ");
		CString RTTIString;
		for (int i = 0; i < NumBaseClasses; i++)
		{
			if (i != 0 && i != NumBaseClasses)
			{
				RTTIString += " : ";
				//x = AddText(View, x, y, crOffset, HS_RTTI, " : ");
			}

			size_t pBaseClassDescriptor = BaseClassArray + (4 * i);
			if (!IsValidPtr(pBaseClassDescriptor))
				continue;

			size_t BaseClassDescriptor;
			ReClassReadMemory((LPVOID)pBaseClassDescriptor, &BaseClassDescriptor, sizeof(size_t));

			if (!IsValidPtr(BaseClassDescriptor))
				continue;

			size_t TypeDescriptor; //pointer at 0x00 in BaseClassDescriptor
			ReClassReadMemory((LPVOID)BaseClassDescriptor, &TypeDescriptor, sizeof(size_t));

			CString RTTIName;
			bool FoundEnd = false;
			char LastChar = ' ';
			for (int j = 1; j < 45; j++)
			{
				char RTTINameChar;
				ReClassReadMemory((LPVOID)(TypeDescriptor + 0x08 + j), &RTTINameChar, 1);
				if (RTTINameChar == '@' && LastChar == '@') // Names seem to be ended with @@
				{
					FoundEnd = true;
					RTTIName += RTTINameChar;
					break;
				}

				RTTIName += RTTINameChar;
				LastChar = RTTINameChar;
			}
			//Did we find a valid rtti name or did we just reach end of loop
			if (!FoundEnd)
				continue;

			TCHAR Demangled[MAX_PATH];
			if (_UnDecorateSymbolName(RTTIName, Demangled, MAX_PATH, UNDNAME_NAME_ONLY) == 0)
				RTTIString += RTTIName;
			else
				RTTIString += Demangled;
			//x = AddText(View, x, y, crOffset, HS_RTTI, "%s", RTTIName.c_str());
		}

		x = AddText(View, x, y, crOffset, HS_RTTI, _T("%s"), RTTIString);
		return x;
	#endif
	}

	int AddComment(ViewInfo& View, int x, int y)
	{
		x = AddText(View, x, y, crComment, HS_NONE, _T("//"));
		// Need the extra whitespace in "%s " anfter the %s to edit.
		x = AddText(View, x, y, crComment, HS_COMMENT, _T("%s "), Comment);

		if (GetType() == nt_hex64)
		{
			float flVal = *((float*)&View.pData[offset]);
			// TODO: Change to int64
			__int64 intVal = *((__int64*)&View.pData[offset]);

			if (gbFloat)
			{
				if (flVal > -99999.0 && flVal < 99999.0)
					x = AddText(View, x, y, crValue, HS_NONE, _T("(%0.3f)"), flVal);
				else
					x = AddText(View, x, y, crValue, HS_NONE, _T("(%0.3f)"), 0.0f);
			}

			if (gbInt)
			{
				if (intVal > 0x6FFFFFFF && intVal < 0x7FFFFFFFFFFF)
					x = AddText(View, x, y, crValue, HS_NONE, _T("(%I64d|0x%IX)"), intVal, intVal);
				else if (intVal == 0)
					x = AddText(View, x, y, crValue, HS_NONE, _T("(%I64d)"), intVal);
				else
					x = AddText(View, x, y, crValue, HS_NONE, _T("(%I64d|0x%X)"), intVal, intVal);
			}

			// *** this is probably broken, let's fix it after
			size_t uintVal = (size_t)intVal;
			CString a(GetAddressName(uintVal, false));
			if (a.GetLength() > 0)
			{
				if (gbPointers)
				{
					//printf( "<%p> here\n", Val );
					if (uintVal > 0x6FFFFFFF && uintVal < 0x7FFFFFFFFFFF)
					{
						x = AddText(View, x, y, crOffset, HS_NONE, _T("*->%s "), a);
						if (gbRTTI)
							x = ResolveRTTI(uintVal, x, View, y);

						// Print out info from PDB at address
						if (sym.IsInitialized())
						{
							CString moduleName = GetModuleName(uintVal);
							if (!moduleName.IsEmpty())
							{
								SymbolReader* symbols = sym.GetSymbolsForModule(moduleName);
								if (symbols)
								{
									CString nameOut;
									if (symbols->GetSymbolStringWithVA(uintVal, nameOut))
									{
										x = AddText(View, x, y, crOffset, HS_EDIT, _T("%s "), nameOut);
									}
								}
							}
						}
					}
				}

				if (gbString)
				{
					bool bAddStr = true;
					char txt[64];
					ReClassReadMemory((LPVOID)uintVal, txt, 64);

					for (int i = 0; i < 8; i++)
					{
						if (!isprint((unsigned char)txt[i]))
							bAddStr = false;
					}

					if (bAddStr)
					{
						txt[63] = '\0';
						x = AddText(View, x, y, crChar, HS_NONE, _T("'%hs'"), txt);
					}
				}
			}
		}
		else if (GetType() == nt_hex32)
		{
			float flVal = *((float*)&View.pData[offset]);
			int intVal = *((int*)&View.pData[offset]);

			if (gbFloat)
			{
				if (flVal > -99999.0 && flVal < 99999.0)
					x = AddText(View, x, y, crValue, HS_NONE, _T("(%0.3f)"), flVal);
				else
					x = AddText(View, x, y, crValue, HS_NONE, _T("(%0.3f)"), 0.0f);
			}

			if (gbInt)
			{
				#ifdef _WIN64
				if (intVal > 0x140000000 && intVal < 0x7FFFFFFFFFFF) // in 64 bit address range
					x = AddText(View, x, y, crValue, HS_NONE, _T("(%i|0x%IX)"), intVal, intVal);
				else if (intVal > 0x400000 && intVal < 0x140000000)
					x = AddText(View, x, y, crValue, HS_NONE, _T("(%i|0x%X)"), intVal, intVal);
				else if (intVal == 0)
					x = AddText(View, x, y, crValue, HS_NONE, _T("(%i)"), intVal);
				else
					x = AddText(View, x, y, crValue, HS_NONE, _T("(%i|0x%X)"), intVal, intVal); 
				#else
				x = (intVal == 0) ? AddText(View, x, y, crValue, HS_NONE, _T("(%i)"), intVal) : AddText(View, x, y, crValue, HS_NONE, _T("(%i|0x%X)"), intVal, intVal);
				#endif
			}

			// *** this is probably broken, let's fix it after
			size_t uintVal = (size_t)intVal;
			CString addressStr(GetAddressName(uintVal, false));
			if (addressStr.GetLength() > 0)
			{
				if (gbPointers)
				{	
					//#ifdef _WIN64
					// If set max to 0x140000000 a bunch of invalid pointers come up
					// Set to 0x110000000 instead
					if (uintVal > 0x400000 && uintVal < 0x110000000)
					{
						x = AddText(View, x, y, crOffset, HS_NONE, _T("*->%s "), addressStr);
						if (gbRTTI)
							x = ResolveRTTI(uintVal, x, View, y);

						// Print out info from PDB at address
						if (sym.IsInitialized())
						{
							CString moduleName = GetModuleName(uintVal);
							if (!moduleName.IsEmpty())
							{
								SymbolReader* symbols = sym.GetSymbolsForModule(moduleName);
								if (symbols)
								{
									CString nameOut;
									if (symbols->GetSymbolStringWithVA(uintVal, nameOut))
									{
										x = AddText(View, x, y, crOffset, HS_EDIT, _T("%s "), nameOut);
									}
								}
							}
						}

					}
				}

				if (gbString)
				{
					bool bAddStr = true;
					char txt[64];
					ReClassReadMemory((LPVOID)uintVal, txt, 64); // TODO: find out why it looks wrong

					for (int i = 0; i < 4; i++)
					{
						if (!isprint((unsigned char)txt[i]))
							bAddStr = false;
					}

					if (bAddStr)
					{
						txt[63] = '\0'; // null terminte (even though we prolly dont have to)
						x = AddText(View, x, y, crChar, HS_NONE, _T("'%hs'"), txt);
					}
				}
			}

		}

		return x;
	}

	void StandardUpdate(HotSpot &Spot)
	{
		if (Spot.ID == HS_NAME)
			Name = Spot.Text;
		else if (Spot.ID == HS_COMMENT)
			Comment = Spot.Text;
	}

	CStringA GetStringFromMemoryA(char* pMemory, int Length)
	{
		CStringA ascii;
		for (int i = 0; i < Length; i++)
		{
			ascii += (isprint(pMemory[i] & 0xFF)) ? (char)pMemory[i] : '.';
		}
		return ascii;
	}

	CStringW GetStringFromMemoryW(wchar_t* pMemory, int Length)
	{
		CStringW widechar;
		for (int i = 0; i < Length; i++) 
		{
			widechar += (iswprint(pMemory[i]) > 0) ? (wchar_t)pMemory[i] : (wchar_t)(L'.');
		}
		return widechar;
	}

	int DrawHidden(ViewInfo& View, int x, int y)
	{
		if (bSelected)
			View.dc->FillSolidRect(0, y, View.client->right, 1, crSelect);
		else
			View.dc->FillSolidRect(0, y, View.client->right, 1, crHidden);
		return y + 1;
	}
};

class CNodeIcon : public CNodeBase
{
public:
	virtual int Draw(ViewInfo& View, int x, int y)
	{
		for (UINT i = 0; i < 21; i++)
			x = AddIcon(View, x, y, i, -1, -1);
		return y += g_FontHeight;
	}
};

class CNodeClass : public CNodeBase
{
public:
	CNodeClass()
	{
		nodeType = nt_class;
		offset = GetBase();
		TCHAR szOffset[128];
		#ifdef _WIN64
		_ui64tot_s(offset, szOffset, 128, 16);
		#else
		_ultot_s(offset, szOffset, 128, 16);
		#endif
		strOffset = szOffset;
		RequestPosition = -1;
		idx = 0;
		pChildWindow = nullptr;
	}

	virtual void Update(HotSpot& Spot)
	{
		StandardUpdate(Spot);
		if (Spot.ID == 0)
		{
			strOffset = Spot.Text;
			offset = ConvertStrToAddress(Spot.Text);
		}

		if (Spot.ID == 1)
		{
			RequestPosition = _tcstol(Spot.Text, NULL, 10); // RequestPosition = ConvertStrToAddress( Spot.Text );
		}
	}

	virtual int GetMemorySize()
	{
		int size = 0;
		for (UINT i = 0; i < Nodes.size(); i++)
			size += Nodes[i]->GetMemorySize();
		return size;
	}

	virtual int Draw(ViewInfo& View, int x, int y)
	{
		offset = ConvertStrToAddress(strOffset);

		AddSelection(View, 0, y, g_FontHeight);
		x = AddOpenClose(View, x, y);

		// Save tx here
		int tx = x;

		x = AddIcon(View, x, y, ICON_CLASS, -1, -1);
		x = AddText(View, x, y, crOffset, 0, _T("%s"), strOffset) + g_FontWidth;

		// x += ( NumDigits( offset ) ) * FontWidth;
		// TODO, figure this out better
		// x += ( ( NumDigits( offset ) - 7 ) * FontWidth ) / 2;
		// printf( "Print %s at %d\n", strOffset, x );

		x = AddText(View, x, y, crIndex, HS_NONE, _T("("));
		x = AddText(View, x, y, crIndex, HS_OPENCLOSE, _T("%i"), idx);
		x = AddText(View, x, y, crIndex, HS_NONE, _T(")"));

		x = AddText(View, x, y, crType, HS_NONE, _T("Class "));
		x = AddText(View, x, y, crName, HS_NAME, Name) + g_FontWidth;
		x = AddText(View, x, y, crValue, HS_NONE, _T("[%i]"), GetMemorySize()) + g_FontWidth;
		x = AddComment(View, x, y);

		y += g_FontHeight;
		if (bOpen[View.Level])
		{
			ViewInfo nv;
			nv = View;
			nv.Level++;
			for (UINT i = 0; i < Nodes.size(); i++)
				y = Nodes[i]->Draw(nv, tx, y);
		}

		return y;
	}

public:
	int idx;
	int RequestPosition;
	CString Code;
	CChildFrame* pChildWindow;
};

class CNodeHex64 : public CNodeBase
{
public:
	CNodeHex64() { nodeType = nt_hex64; }

	virtual void Update(HotSpot& Spot)
	{
		StandardUpdate(Spot);
		unsigned char v = (unsigned char)(_tcstoul(Spot.Text, NULL, 16) & 0xFF);
		if (Spot.ID >= 0 && Spot.ID < 8)
			ReClassWriteMemory((LPVOID)(Spot.Address + Spot.ID), &v, 1);
	}

	virtual int GetMemorySize() { return 8; }

	virtual int Draw(ViewInfo& View, int x, int y)
	{
		if (bHidden)
			return DrawHidden(View, x, y);

		unsigned char* pMemory = (unsigned char*)&View.pData[offset];
		AddSelection(View, 0, y, g_FontHeight);
		AddDelete(View, x, y);
		AddTypeDrop(View, x, y);
		//AddAdd(View,x,y);

		int tx = x + TXOFFSET + 16;
		tx = AddAddressOffset(View, tx, y);

		if (gbText)
		{
			CStringA str = GetStringFromMemoryA((char*)pMemory, 8) + " ";
			tx = AddText(View, tx, y, crChar, HS_NONE, "%s", str.GetBuffer());
		}

		tx = AddText(View, tx, y, crHex, 0, _T("%0.2X"), pMemory[0]) + g_FontWidth;
		tx = AddText(View, tx, y, crHex, 1, _T("%0.2X"), pMemory[1]) + g_FontWidth;
		tx = AddText(View, tx, y, crHex, 2, _T("%0.2X"), pMemory[2]) + g_FontWidth;
		tx = AddText(View, tx, y, crHex, 3, _T("%0.2X"), pMemory[3]) + g_FontWidth;
		tx = AddText(View, tx, y, crHex, 4, _T("%0.2X"), pMemory[4]) + g_FontWidth;
		tx = AddText(View, tx, y, crHex, 5, _T("%0.2X"), pMemory[5]) + g_FontWidth;
		tx = AddText(View, tx, y, crHex, 6, _T("%0.2X"), pMemory[6]) + g_FontWidth;
		tx = AddText(View, tx, y, crHex, 7, _T("%0.2X"), pMemory[7]) + g_FontWidth;
		tx = AddComment(View, tx, y);

		return y += g_FontHeight;
	}
};

class CNodeHex32 : public CNodeBase
{
public:
	CNodeHex32() { nodeType = nt_hex32; }

	virtual void Update(HotSpot& Spot)
	{
		StandardUpdate(Spot);
		unsigned char v = (unsigned char)(_tcstoul(Spot.Text, NULL, 16) & 0xFF);
		if (Spot.ID >= 0 && Spot.ID < 4)
			ReClassWriteMemory((LPVOID)(Spot.Address + Spot.ID), &v, 1);
	}

	virtual int GetMemorySize() { return 4; }

	virtual int Draw(ViewInfo& View, int x, int y)
	{
		if (bHidden)
			return DrawHidden(View, x, y);

		unsigned char* pMemory = (unsigned char*)&View.pData[offset];
		AddSelection(View, 0, y, g_FontHeight);
		AddDelete(View, x, y);
		AddTypeDrop(View, x, y);
		//AddAdd(View,x,y);

		int tx = x + TXOFFSET + 16;
		tx = AddAddressOffset(View, tx, y);

		if (gbText)
		{
			// TODO these are the dots, do alignment instead of 4
			CStringA str = GetStringFromMemoryA((char*)pMemory, 4);
			str += "     ";
			tx = AddText(View, tx, y, crChar, HS_NONE, "%s", str);
		}

		tx = AddText(View, tx, y, crHex, 0, _T("%0.2X"), pMemory[0]) + g_FontWidth;
		tx = AddText(View, tx, y, crHex, 1, _T("%0.2X"), pMemory[1]) + g_FontWidth;
		tx = AddText(View, tx, y, crHex, 2, _T("%0.2X"), pMemory[2]) + g_FontWidth;
		tx = AddText(View, tx, y, crHex, 3, _T("%0.2X"), pMemory[3]) + g_FontWidth;
		tx = AddComment(View, tx, y);

		return y += g_FontHeight;
	}
};

class CNodeHex16 : public CNodeBase
{
public:
	CNodeHex16() { nodeType = nt_hex16; }

	virtual void Update(HotSpot& Spot)
	{
		StandardUpdate(Spot);
		unsigned char v = (unsigned char)(_tcstoul(Spot.Text, NULL, 16) & 0xFF);
		if (Spot.ID == 0) 
			ReClassWriteMemory((LPVOID)Spot.Address, &v, 1);
		if (Spot.ID == 1)
			ReClassWriteMemory((LPVOID)(Spot.Address + 1), &v, 1);
	}

	virtual int GetMemorySize() { return 2; }

	virtual int Draw(ViewInfo& View, int x, int y)
	{
		if (bHidden) 
			return DrawHidden(View, x, y);

		unsigned char* pMemory = (unsigned char*)&View.pData[offset];
		AddSelection(View, 0, y, g_FontHeight);
		AddDelete(View, x, y);
		AddTypeDrop(View, x, y);
		//AddAdd(View,x,y);

		int tx = x + TXOFFSET + 16;
		tx = AddAddressOffset(View, tx, y);

		if (gbText)
		{
			CStringA str = GetStringFromMemoryA((char*)pMemory, 2);
			str += "       ";
			tx = AddText(View, tx, y, crChar, HS_NONE, "%s", str.GetBuffer());
		}

		tx = AddText(View, tx, y, crHex, 0, _T("%0.2X"), pMemory[0] & 0xFF) + g_FontWidth;
		tx = AddText(View, tx, y, crHex, 1, _T("%0.2X"), pMemory[1] & 0xFF) + g_FontWidth;
		tx = AddComment(View, tx, y);

		return y += g_FontHeight;
	}
};

class CNodeHex8 : public CNodeBase
{
public:
	CNodeHex8() { nodeType = nt_hex8; }

	virtual void Update(HotSpot& Spot)
	{
		StandardUpdate(Spot);
		unsigned char v = (unsigned char)(_tcstoul(Spot.Text, NULL, 16) & 0xFF);
		if (Spot.ID == 0)
			ReClassWriteMemory((LPVOID)Spot.Address, &v, 1);
	}

	virtual int GetMemorySize() { return 1; }

	virtual int Draw(ViewInfo& View, int x, int y)
	{
		if (bHidden) 
			return DrawHidden(View, x, y);

		unsigned char* pMemory = (unsigned char*)&((unsigned char*)View.pData)[offset];
		AddSelection(View, 0, y, g_FontHeight);
		AddDelete(View, x, y);
		AddTypeDrop(View, x, y);
		//AddAdd(View,x,y);

		int tx = x + TXOFFSET + 16;
		tx = AddAddressOffset(View, tx, y);

		if (gbText)
		{
			CStringA str = GetStringFromMemoryA((char*)pMemory, 1);
			str += "        ";
			tx = AddText(View, tx, y, crChar, HS_NONE, "%s", str.GetBuffer());
		}

		tx = AddText(View, tx, y, crHex, 0, _T("%0.2X"), pMemory[0] & 0xFF) + g_FontWidth;
		tx = AddComment(View, tx, y);

		return y += g_FontHeight;
	}
};

class CNodeBits : public CNodeBase
{
public:
	CNodeBits() { nodeType = nt_bits; }

	virtual void Update(HotSpot& Spot)
	{
		StandardUpdate(Spot);
		unsigned char v = (unsigned char)(_tcstoul(Spot.Text, NULL, 16) & 0xFF);
		if (Spot.ID == 0)
			ReClassWriteMemory((LPVOID)Spot.Address, &v, 1);
	}

	virtual int GetMemorySize() { return 1; }

	virtual int Draw(ViewInfo& View, int x, int y)
	{
		if (bHidden)
			return DrawHidden(View, x, y);

		unsigned char* pMemory = (unsigned char*)&View.pData[offset];
		AddSelection(View, 0, y, g_FontHeight);
		AddDelete(View, x, y);
		AddTypeDrop(View, x, y);
		//AddAdd(View,x,y);

		int tx = x + TXOFFSET + 16;
		tx = AddAddressOffset(View, tx, y);

		if (gbText)
		{
			BitArray<unsigned char> bits;
			bits.SetValue(pMemory[0]);

			CStringA str = bits.GetBitsReverseString();
			str += ' ';
			tx = AddText(View, tx, y, crChar, HS_NONE, "%s", str.GetBuffer());
		}

		tx = AddText(View, tx, y, crHex, 0, _T("%0.2X"), pMemory[0]) + g_FontWidth;
		tx = AddComment(View, tx, y);

		return y += g_FontHeight;
	}
};

class CNodeVTable : public CNodeBase
{
public:
	CNodeVTable() { nodeType = nt_vtable; }

	virtual void Update(HotSpot& Spot) { StandardUpdate(Spot); }

	int GetMemorySize()
	{
		return sizeof(size_t);
	}

	int Draw(ViewInfo& View, int x, int y)
	{
		if (bHidden)
			return DrawHidden(View, x, y);

		size_t* pMemory = (size_t*)&View.pData[offset];
		AddSelection(View, 0, y, g_FontHeight);
		AddDelete(View, x, y);
		AddTypeDrop(View, x, y);

		x = AddOpenClose(View, x, y);
		x = AddIcon(View, x, y, ICON_VTABLE, -1, -1);

		int tx = x;
		x = AddAddressOffset(View, x, y);
		x = AddText(View, x, y, crVTable, HS_NONE, _T("VTable[%i]"), Nodes.size()) + g_FontWidth;

		//if (Name.IsEmpty())
		x = AddText(View, x, y, crName, HS_NAME, _T("%s"), Name) + g_FontWidth;
		//else
		//	x = AddText(View, x, y, crName, HS_NONE, _T("%s_vtable"), pParent->Name) + FontWidth;

		x = AddComment(View, x, y);

		y += g_FontHeight;
		if (bOpen[View.Level])
		{
			// vtable stuff
			DWORD NeededSize = (int)Nodes.size() * sizeof(size_t);

			Memory.SetSize(NeededSize);
			ViewInfo newView;
			newView = View;
			newView.pData = Memory.pMemory;

			newView.Address = pMemory[0];
			ReClassReadMemory((LPVOID)newView.Address, newView.pData, NeededSize);

			for (UINT i = 0; i < Nodes.size(); i++)
			{
				Nodes[i]->offset = i * sizeof(size_t);
				y = Nodes[i]->Draw(newView, tx, y);
			}
		}

		return y;
	}

public:
	CMemory Memory;
};

class CNodeFunctionPtr : public CNodeBase
{
public:
	CNodeFunctionPtr()
	{
		nodeType = nt_function;
		Name = _T("");
	}

	virtual void Update(HotSpot& Spot)
	{
		StandardUpdate(Spot);
		if (Spot.ID == 0)
		{
			Assembly.clear();

			size_t addy = Spot.Address;
			ReClassReadMemory((LPVOID)addy, &addy, sizeof(size_t));
			char* code[1536]; // max 1536 lines
			ReClassReadMemory((LPVOID)addy, code, 1536);
			char** EndCodeSection = (code + 1536);

			DISASM MyDisasm;
			memset(&MyDisasm, 0, sizeof(DISASM));

			MyDisasm.EIP = (size_t)code;

			MyDisasm.VirtualAddr = (unsigned __int64)addy;
			#ifdef _WIN64
			MyDisasm.Archi = 64;
			#else
			MyDisasm.Archi = 0;
			#endif
			MyDisasm.Options = PrefixedNumeral;

			bool Error = 0;
			while (!Error)
			{
				MyDisasm.SecurityBlock = (unsigned __int32)((size_t)EndCodeSection - (size_t)MyDisasm.EIP);

				int len = Disasm(&MyDisasm);
				if (len == OUT_OF_BLOCK)
					Error = 1;
				else if (len == UNKNOWN_OPCODE)
					Error = 1;
				else
				{
					char szInstruction[96];
					sprintf_s(szInstruction, "%p  %s", (void*)MyDisasm.VirtualAddr, MyDisasm.CompleteInstr);
					Assembly.emplace_back(szInstruction);

					MyDisasm.EIP = MyDisasm.EIP + len;
					MyDisasm.VirtualAddr = MyDisasm.VirtualAddr + len;
					if (MyDisasm.EIP >= (UIntPtr)EndCodeSection)
						break;

					unsigned char opcode;
					ReClassReadMemory((LPVOID)MyDisasm.VirtualAddr, &opcode, sizeof(unsigned char));
					if (opcode == 0xCC) // INT3 instruction
						break;
				}
			}
		}
	}

	virtual int GetMemorySize()
	{
		return sizeof(size_t);
	}

	virtual int Draw(ViewInfo& View, int x, int y)
	{
		if (bHidden)
			return DrawHidden(View, x, y);

		AddSelection(View, 0, y, g_FontHeight);
		AddDelete(View, x, y);
		AddTypeDrop(View, x, y);
		//AddAdd(View,x,y);

		int tx = x + TXOFFSET;
		tx = AddIcon(View, tx, y, ICON_METHOD, -1, -1);
		int ax = tx;
		tx = AddAddressOffset(View, tx, y);

		if (pParent->GetType() != nt_vtable)
			tx = AddText(View, tx, y, crType, HS_NONE, _T("Function"));
		else
			tx = AddText(View, tx, y, crFunction, HS_NONE, _T("(%i)"), offset / sizeof(size_t));

		tx = AddIcon(View, tx, y, ICON_CAMERA, HS_EDIT, HS_CLICK);
		tx += g_FontWidth;

		if (Name.IsEmpty())
			tx = AddText(View, tx, y, crName, HS_NAME, _T("Function_%i"), offset / sizeof(size_t));
		else
			tx = AddText(View, tx, y, crName, HS_NAME, _T("%s"), Name);

		tx += g_FontWidth;

		if (Assembly.size() > 0)
			tx = AddOpenClose(View, tx, y);

		tx += g_FontWidth;

		tx = AddComment(View, tx, y);

		if (bOpen[View.Level])
		{
			for (UINT i = 0; i < Assembly.size(); i++)
			{
				y += g_FontHeight;
				AddText(View, ax, y, crHex, HS_EDIT, "%s", Assembly[i].GetBuffer());
			}
		}

		return y += g_FontHeight;
	}

public:
	std::vector<CStringA> Assembly;
};

class CNodePtr : public CNodeBase
{
public:
	CNodePtr() { nodeType = nt_pointer; }

	virtual void Update(HotSpot& Spot) { StandardUpdate(Spot); }

	virtual int GetMemorySize()
	{
		return sizeof(size_t);
	}

	virtual int Draw(ViewInfo& View, int x, int y)
	{
		if (bHidden)
			return DrawHidden(View, x, y);

		size_t* pMemory = (size_t*)&View.pData[offset];

		//printf( "read ptr: %p\n", View.pData );
		AddSelection(View, 0, y, g_FontHeight);
		AddDelete(View, x, y);
		AddTypeDrop(View, x, y);

		x = AddOpenClose(View, x, y);
		x = AddIcon(View, x, y, ICON_POINTER, -1, -1);

		int tx = x;
		tx = AddAddressOffset(View, tx, y);
		tx = AddText(View, tx, y, crType, HS_NONE, _T("Ptr "));
		tx = AddText(View, tx, y, crName, HS_NAME, _T("%s"), Name);
		tx = AddText(View, tx, y, crValue, HS_NONE, _T(" <%s>"), pNode->Name);
		tx = AddIcon(View, tx, y, ICON_CHANGE, HS_CLICK, HS_CHANGE_A);

		tx += g_FontWidth;
		tx = AddComment(View, tx, y);

		y += g_FontHeight;

		if (bOpen[View.Level])
		{
			DWORD NeededSize = pNode->GetMemorySize();
			Memory.SetSize(NeededSize);

			ViewInfo newView;
			newView = View;
			newView.pData = Memory.pMemory;
			newView.Address = pMemory[0];

			ReClassReadMemory((LPVOID)newView.Address, newView.pData, NeededSize);

			y = pNode->Draw(newView, x, y);
		}
		return y;
	}

public:
	CNodeBase* pNode;
	CMemory Memory;
};

class CNodeInt64 : public CNodeBase
{
public:
	CNodeInt64() { nodeType = nt_int64; }

	virtual void Update(HotSpot& Spot)
	{
		StandardUpdate(Spot);
		__int64 v = _ttoi64(Spot.Text);
		if (Spot.ID == 0)
			ReClassWriteMemory((LPVOID)Spot.Address, &v, sizeof(__int64));
	}

	virtual int GetMemorySize() { return sizeof(__int64); }

	virtual int Draw(ViewInfo& View, int x, int y)
	{
		if (bHidden) return DrawHidden(View, x, y);
		__int64* pMemory = (__int64*)&View.pData[offset];
		AddSelection(View, 0, y, g_FontHeight);
		AddDelete(View, x, y);
		AddTypeDrop(View, x, y);
		//AddAdd(View,x,y);

		int tx = x + TXOFFSET;
		tx = AddIcon(View, tx, y, ICON_INTEGER, HS_NONE, HS_NONE);
		tx = AddAddressOffset(View, tx, y);
		tx = AddText(View, tx, y, crType, HS_NONE, _T("Int64 "));
		tx = AddText(View, tx, y, crName, HS_NAME, _T("%s"), Name);
		tx = AddText(View, tx, y, crName, HS_NONE, _T(" = "));
		tx = AddText(View, tx, y, crValue, HS_EDIT, _T("%lli"), pMemory[0]) + g_FontWidth;
		tx = AddComment(View, tx, y);

		return y += g_FontHeight;
	}
};

class CNodeInt32 : public CNodeBase
{
public:
	CNodeInt32() { nodeType = nt_int32; }

	virtual void Update(HotSpot& Spot)
	{
		StandardUpdate(Spot);
		__int32 v = _ttoi(Spot.Text);
		if (Spot.ID == 0)
			ReClassWriteMemory((LPVOID)Spot.Address, &v, sizeof(long));
	}

	virtual int GetMemorySize() { return sizeof(long); }

	virtual int Draw(ViewInfo& View, int x, int y)
	{
		if (bHidden) 
			return DrawHidden(View, x, y);

		__int32* pMemory = (__int32*)&View.pData[offset];
		AddSelection(View, 0, y, g_FontHeight);
		AddDelete(View, x, y);
		AddTypeDrop(View, x, y);
		//AddAdd(View,x,y);

		int tx = x + TXOFFSET;
		tx = AddIcon(View, tx, y, ICON_INTEGER, HS_NONE, HS_NONE);
		tx = AddAddressOffset(View, tx, y);
		tx = AddText(View, tx, y, crType, HS_NONE, _T("Int32 "));
		tx = AddText(View, tx, y, crName, HS_NAME, _T("%s"), Name);
		tx = AddText(View, tx, y, crName, HS_NONE, _T(" = "));
		tx = AddText(View, tx, y, crValue, HS_EDIT, _T("%i"), pMemory[0]) + g_FontWidth;
		tx = AddComment(View, tx, y);

		return y += g_FontHeight;
	}
};
class CNodeInt16 : public CNodeBase
{
public:
	CNodeInt16() { nodeType = nt_int16; }

	virtual void Update(HotSpot& Spot)
	{
		StandardUpdate(Spot);
		__int16 v = _ttoi(Spot.Text);
		if (Spot.ID == 0)
			ReClassWriteMemory((LPVOID)Spot.Address, &v, sizeof(short));
	}

	virtual int GetMemorySize() { return sizeof(short); }

	virtual int Draw(ViewInfo& View, int x, int y)
	{
		if (bHidden) 
			return DrawHidden(View, x, y);

		__int16* pMemory = (__int16*)&View.pData[offset];
		AddSelection(View, 0, y, g_FontHeight);
		AddDelete(View, x, y);
		AddTypeDrop(View, x, y);
		//AddAdd(View,x,y);

		int tx = x + TXOFFSET;
		tx = AddIcon(View, tx, y, ICON_INTEGER, HS_NONE, HS_NONE);
		tx = AddAddressOffset(View, tx, y);
		tx = AddText(View, tx, y, crType, HS_NONE, _T("Int16 "));
		tx = AddText(View, tx, y, crName, HS_NAME, _T("%s"), Name);
		tx = AddText(View, tx, y, crName, HS_NONE, _T(" = "));
		tx = AddText(View, tx, y, crValue, HS_EDIT, _T("%i"), pMemory[0]) + g_FontWidth;
		tx = AddComment(View, tx, y);

		return y += g_FontHeight;
	}
};

class CNodeInt8 : public CNodeBase
{
public:
	CNodeInt8() { nodeType = nt_int8; }

	virtual void Update(HotSpot& Spot)
	{
		StandardUpdate(Spot);
		__int8 v = _ttoi(Spot.Text);
		if (Spot.ID == 0)
			ReClassWriteMemory((LPVOID)Spot.Address, &v, sizeof(char));
	}

	virtual int GetMemorySize() { return sizeof(char); }

	virtual int Draw(ViewInfo& View, int x, int y)
	{
		if (bHidden) 
			return DrawHidden(View, x, y);

		__int8* pMemory = (__int8*)&View.pData[offset];
		AddSelection(View, 0, y, g_FontHeight);
		AddDelete(View, x, y);
		AddTypeDrop(View, x, y);
		//AddAdd(View,x,y);

		int tx = x + TXOFFSET;
		tx = AddIcon(View, tx, y, ICON_INTEGER, HS_NONE, HS_NONE);
		tx = AddAddressOffset(View, tx, y);
		tx = AddText(View, tx, y, crType, HS_NONE, _T("Int8  "));
		tx = AddText(View, tx, y, crName, HS_NAME, _T("%s"), Name);
		tx = AddText(View, tx, y, crName, HS_NONE, _T(" = "));
		tx = AddText(View, tx, y, crValue, HS_EDIT, _T("%i"), pMemory[0]) + g_FontWidth;
		tx = AddComment(View, tx, y);

		return y += g_FontHeight;
	}
};

class CNodeQWORD : public CNodeBase
{
public:
	CNodeQWORD() { nodeType = nt_uint64; }

	virtual void Update(HotSpot& Spot)
	{
		StandardUpdate(Spot);
		DWORD64 v = _ttoi64(Spot.Text);
		if(Spot.ID == 0)
			ReClassWriteMemory((LPVOID)Spot.Address, &v, sizeof(DWORD64));
	}

	virtual int GetMemorySize( ) { return sizeof(DWORD64); }

	virtual int Draw(ViewInfo& View, int x, int y)
	{
		if (bHidden) 
			return DrawHidden(View, x, y);

		DWORD64* pMemory = (DWORD64*)&View.pData[offset];
		AddSelection(View, 0, y, g_FontHeight);
		AddDelete(View, x, y);
		AddTypeDrop(View, x, y);
		//AddAdd(View,x,y);

		int tx = x + TXOFFSET;
		tx = AddIcon(View, tx, y, ICON_UNSIGNED, HS_NONE, HS_NONE);
		tx = AddAddressOffset(View, tx, y);
		tx = AddText(View, tx, y, crType, HS_NONE, _T("QWORD "));
		tx = AddText(View, tx, y, crName, HS_NAME, _T("%s"), Name);
		tx = AddText(View, tx, y, crName, HS_NONE, _T(" = "));
		tx = AddText(View, tx, y, crValue, HS_EDIT, _T("%llu"), pMemory[0]) + g_FontWidth;
		tx = AddComment(View, tx, y);

		return y += g_FontHeight;
	}
};


class CNodeDWORD : public CNodeBase
{
public:
	CNodeDWORD() { nodeType = nt_uint32; }

	virtual void Update(HotSpot& Spot)
	{
		StandardUpdate(Spot);
		DWORD v = _ttoi(Spot.Text);
		if (Spot.ID == 0)
			ReClassWriteMemory((LPVOID)Spot.Address, &v, sizeof(unsigned long));
	}

	virtual int GetMemorySize() { return sizeof(unsigned long); }

	virtual int Draw(ViewInfo& View, int x, int y)
	{
		if (bHidden) 
			return DrawHidden(View, x, y);

		DWORD* pMemory = (DWORD*)&View.pData[offset];
		AddSelection(View, 0, y, g_FontHeight);
		AddDelete(View, x, y);
		AddTypeDrop(View, x, y);
		//AddAdd(View,x,y);

		int tx = x + TXOFFSET;
		tx = AddIcon(View, tx, y, ICON_UNSIGNED, HS_NONE, HS_NONE);
		tx = AddAddressOffset(View, tx, y);
		tx = AddText(View, tx, y, crType, HS_NONE, _T("DWORD "));
		tx = AddText(View, tx, y, crName, HS_NAME, _T("%s"), Name);
		tx = AddText(View, tx, y, crName, HS_NONE, _T(" = "));
		tx = AddText(View, tx, y, crValue, HS_EDIT, _T("%u"), pMemory[0]) + g_FontWidth;
		tx = AddComment(View, tx, y);

		return y += g_FontHeight;
	}
};

class CNodeWORD : public CNodeBase
{
public:
	CNodeWORD() { nodeType = nt_uint16; }

	virtual void Update(HotSpot& Spot)
	{
		StandardUpdate(Spot);
		WORD v = _ttoi(Spot.Text);
		if (Spot.ID == 0)
			ReClassWriteMemory((LPVOID)Spot.Address, &v, sizeof(unsigned short));

	}

	virtual int GetMemorySize() { return sizeof(unsigned short); }

	virtual int Draw(ViewInfo& View, int x, int y)
	{
		if (bHidden)
			return DrawHidden(View, x, y);

		WORD* pMemory = (WORD*)&View.pData[offset];
		AddSelection(View, 0, y, g_FontHeight);
		AddDelete(View, x, y);
		AddTypeDrop(View, x, y);
		//AddAdd(View,x,y);

		int tx = x + TXOFFSET;
		tx = AddIcon(View, tx, y, ICON_UNSIGNED, HS_NONE, HS_NONE);
		tx = AddAddressOffset(View, tx, y);
		tx = AddText(View, tx, y, crType, HS_NONE, _T("WORD  "));
		tx = AddText(View, tx, y, crName, HS_NAME, _T("%s"), Name);
		tx = AddText(View, tx, y, crName, HS_NONE, _T(" = "));
		tx = AddText(View, tx, y, crValue, HS_EDIT, _T("%u"), pMemory[0]) + g_FontWidth;
		tx = AddComment(View, tx, y);

		return y += g_FontHeight;
	}
};

class CNodeByte : public CNodeBase
{
public:
	CNodeByte() { nodeType = nt_uint8; }

	virtual void Update(HotSpot& Spot)
	{
		StandardUpdate(Spot);
		unsigned char v = _ttoi(Spot.Text);
		if (Spot.ID == 0)
			ReClassWriteMemory((LPVOID)Spot.Address, &v, sizeof(unsigned char));
	}

	virtual int GetMemorySize() { return sizeof(unsigned char); }

	virtual int Draw(ViewInfo& View, int x, int y)
	{
		if (bHidden)
			return DrawHidden(View, x, y);

		unsigned char* pMemory = (unsigned char*)&View.pData[offset];
		AddSelection(View, 0, y, g_FontHeight);
		AddDelete(View, x, y);
		AddTypeDrop(View, x, y);
		//AddAdd(View,x,y);

		int tx = x + TXOFFSET;
		tx = AddIcon(View, tx, y, ICON_UNSIGNED, HS_NONE, HS_NONE);
		tx = AddAddressOffset(View, tx, y);
		tx = AddText(View, tx, y, crType, HS_NONE, _T("BYTE  "));
		tx = AddText(View, tx, y, crName, HS_NAME, _T("%s"), Name);
		tx = AddText(View, tx, y, crName, HS_NONE, _T(" = "));
		tx = AddText(View, tx, y, crValue, HS_EDIT, _T("%u"), pMemory[0]) + g_FontWidth;
		tx = AddComment(View, tx, y);

		return y += g_FontHeight;
	}
};

class CNodeText : public CNodeBase
{
public:
	CNodeText()
	{
		nodeType = nt_text;
		Name = _T("Text");
		memsize = 16;
	}

	virtual void Update(HotSpot& Spot)
	{
		StandardUpdate(Spot);
		if (Spot.ID == 0) 
		{
			memsize = _ttoi(Spot.Text);
		}
		else if (Spot.ID == 1)
		{
			DWORD Length = Spot.Text.GetLength() + 1;
			if (Length > memsize)
				Length = memsize;
			ReClassWriteMemory((LPVOID)Spot.Address, Spot.Text.GetBuffer(), Length);
		}
	}

	virtual int GetMemorySize() 
	{ 
		return memsize; 
	}

	virtual int Draw(ViewInfo& View, int x, int y)
	{
		if (bHidden)
			return DrawHidden(View, x, y);

		char* pMemory = (char*)&View.pData[offset];

		AddSelection(View, 0, y, g_FontHeight);
		AddDelete(View, x, y);
		AddTypeDrop(View, x, y);
		//AddAdd(View,x,y);

		int tx = x + TXOFFSET;
		tx = AddIcon(View, tx, y, ICON_TEXT, HS_NONE, HS_NONE);
		tx = AddAddressOffset(View, tx, y);
		tx = AddText(View, tx, y, crType, HS_NONE, _T("Text "));
		tx = AddText(View, tx, y, crName, HS_NAME, _T("%s"), Name);
		tx = AddText(View, tx, y, crIndex, HS_NONE, _T("["));
		tx = AddText(View, tx, y, crIndex, HS_EDIT, _T("%i"), GetMemorySize());
		tx = AddText(View, tx, y, crIndex, HS_NONE, _T("]"));

		if ( VALID( pMemory ) )
		{
			CStringA str = GetStringFromMemoryA( pMemory, GetMemorySize( ) );
			tx = AddText( View, tx, y, crChar, HS_NONE, _T( " = '" ) );
			tx = AddText( View, tx, y, crChar, 1, "%.150s", str.GetBuffer( ) );
			tx = AddText( View, tx, y, crChar, HS_NONE, _T( "' " ) ) + g_FontWidth;
		}

		tx = AddComment(View, tx, y);
		return y += g_FontHeight;
	}

public:
	DWORD memsize;
};

class CNodeUnicode : public CNodeBase
{
public:
	CNodeUnicode( )
	{
		nodeType = nt_unicode;
		Name = "Unicode";
		memsize = 8 * sizeof( wchar_t );
	}

	virtual void Update( HotSpot& Spot )
	{
		StandardUpdate( Spot );
		if ( Spot.ID == 0 )
		{
			memsize = _ttoi( Spot.Text ) * sizeof( wchar_t );
		} 
		else if ( Spot.ID == 1 )
		{
			DWORD Length = Spot.Text.GetLength( );
			if ( Length > ( memsize / sizeof( wchar_t ) ) )
				Length = ( memsize / sizeof( wchar_t ) );

			// Has to be done this way in order to make it compatible in mbs and unicode mode (ghetto)
			TCHAR* pSource = Spot.Text.GetBuffer( );
			wchar_t* pwszConverted = new wchar_t[ Length + 1 ];
			for ( UINT i = 0; i <= Length; i++ )
				pwszConverted[ i ] = (wchar_t) pSource[ i ];

			ReClassWriteMemory( (LPVOID) Spot.Address, pwszConverted, Length );

			delete pwszConverted;
		}
	}

	virtual int GetMemorySize( void )
	{
		return memsize;
	}

	virtual int Draw( ViewInfo& View, int x, int y )
	{
		if ( bHidden )
			return DrawHidden( View, x, y );

		wchar_t* pMemory = ( wchar_t* )&( (unsigned char*) View.pData )[ offset ];
		AddSelection( View, 0, y, g_FontHeight );
		AddDelete( View, x, y );
		AddTypeDrop( View, x, y );
		//AddAdd(View,x,y);

		int tx = x + TXOFFSET;
		tx = AddIcon( View, tx, y, ICON_TEXT, HS_NONE, HS_NONE );
		tx = AddAddressOffset( View, tx, y );
		tx = AddText( View, tx, y, crType, HS_NONE, _T( "Unicode " ) );
		tx = AddText( View, tx, y, crName, HS_NAME, _T( "%s" ), Name );
		tx = AddText( View, tx, y, crIndex, HS_NONE, _T( "[" ) );
		tx = AddText( View, tx, y, crIndex, HS_EDIT, _T( "%i" ), memsize / sizeof( wchar_t ) );
		tx = AddText( View, tx, y, crIndex, HS_NONE, _T( "]" ) );

		if ( VALID( pMemory ) )
		{
			CStringW str = GetStringFromMemoryW( pMemory, memsize / sizeof(wchar_t));
			tx = AddText( View, tx, y, crChar, HS_NONE, _T( " = '" ) );
			tx = AddText( View, tx, y, crChar, HS_OPENCLOSE, _T( "%.150ls" ), str ); // ls cause its unicode
			tx = AddText( View, tx, y, crChar, HS_NONE, _T( "' " ) ) + g_FontWidth;
		}

		tx = AddComment( View, tx, y );
		return y += g_FontHeight;
	}

public:
	DWORD memsize;
};

class CNodeCharPtr : public CNodeBase
{
public:
	CNodeCharPtr() 
	{
		nodeType = nt_pchar;
		Name = "PChar";
	}

	virtual void Update(HotSpot& Spot)
	{
		StandardUpdate(Spot);
#ifdef _WIN64
		size_t ptr = _ttoi64(Spot.Text);
#else
		size_t ptr = _ttoi(Spot.Text);
#endif
		if (Spot.ID == 0)
			ReClassWriteMemory((LPVOID)Spot.Address, &ptr, sizeof(size_t));
	}

	virtual int GetMemorySize()
	{
		return sizeof(size_t);
	}

	virtual int Draw(ViewInfo& View, int x, int y)
	{
		if (bHidden)
			return DrawHidden(View, x, y);

		size_t* pMemory = (size_t*)&View.pData[offset];

		AddSelection(View, 0, y, g_FontHeight);
		AddDelete(View, x, y);
		AddTypeDrop(View, x, y);
		//AddAdd(View, x, y);

		int tx = x + TXOFFSET;
		tx = AddIcon(View, tx, y, ICON_INTEGER, HS_NONE, HS_NONE);
		tx = AddAddressOffset(View, tx, y);
		tx = AddText(View, tx, y, crType, HS_NONE, _T("PCHAR "));
		tx = AddText(View, tx, y, crName, HS_NAME, Name);

		//tx = AddText(View,tx,y,crName,HS_NONE," = ");
		//tx = AddText(View,tx,y,crValue,0,"%lli",pMemory[0]) + FontWidth;
		//tx = AddComment(View,tx,y);

		/*
		int tx = x + 16;
		tx = AddIcon(View,tx,y,ICON_TEXT,HS_NONE,HS_NONE);
		tx = AddAddressOffset(View,tx,y);
		tx = AddText(View,tx,y,crType,HS_NONE,"Text ");
		tx = AddText(View,tx,y,crName,69,"%s",Name);
		tx = AddText(View,tx,y,crIndex,HS_NONE,"[");
		tx = AddText(View,tx,y,crIndex,0,"%i",memsize);
		tx = AddText(View,tx,y,crIndex,HS_NONE,"]");
		*/

		tx = AddText(View, tx, y, crChar, HS_NONE, _T(" = '"));
		if (VALID(pMemory))
		{
			CStringA sc = ReadMemoryStringA((size_t)pMemory[0], 128);
			tx = AddText(View, tx, y, crChar, 1, "%s", sc.GetBuffer());
		}

		tx = AddText(View, tx, y, crChar, HS_NONE, _T("' ")) + g_FontWidth;
		tx = AddComment(View, tx, y);

		return y += g_FontHeight;
	}

public:
	CNodeBase* pNode;
	CMemory Memory;
};

// TODO: Fix WCharPtr node type
class CNodeWCharPtr : public CNodeBase
{
public:
	CNodeWCharPtr() 
	{
		nodeType = nt_pwchar;
		Name = "PWChar";
	}

	virtual void Update(HotSpot& Spot)
	{
		StandardUpdate(Spot);
		__int64 v = _ttoi64(Spot.Text);
		if (Spot.ID == 0)
			ReClassWriteMemory((LPVOID)Spot.Address, &v, sizeof(size_t));
	}

	virtual int GetMemorySize()
	{
		return sizeof(size_t);
	}

	virtual int Draw(ViewInfo& View, int x, int y)
	{
		if (bHidden)
			return DrawHidden(View, x, y);

		size_t* pMemory = (size_t*)&View.pData[offset];

		AddSelection(View, 0, y, g_FontHeight);
		AddDelete(View, x, y);
		AddTypeDrop(View, x, y);
		//AddAdd(View, x, y);

		int tx = x + TXOFFSET;
		tx = AddIcon(View, tx, y, ICON_INTEGER, HS_NONE, HS_NONE);
		tx = AddAddressOffset(View, tx, y);
		tx = AddText(View, tx, y, crType, HS_NONE, _T("PWCHAR "));
		tx = AddText(View, tx, y, crName, HS_NAME, Name);

		//tx = AddText(View,tx,y,crName,HS_NONE," = ");
		//tx = AddText(View,tx,y,crValue,0,"%lli",pMemory[0]) + FontWidth;
		//tx = AddComment(View,tx,y);

		/*
		int tx = x + 16;
		tx = AddIcon(View,tx,y,ICON_TEXT,HS_NONE,HS_NONE);
		tx = AddAddressOffset(View,tx,y);
		tx = AddText(View,tx,y,crType,HS_NONE,"Text ");
		tx = AddText(View,tx,y,crName,69,"%s",Name);
		tx = AddText(View,tx,y,crIndex,HS_NONE,"[");
		tx = AddText(View,tx,y,crIndex,0,"%i",memsize);
		tx = AddText(View,tx,y,crIndex,HS_NONE,"]");
		*/

		tx = AddText(View, tx, y, crChar, HS_NONE, _T(" = '"));
		if (VALID(pMemory))
		{
			CStringW sc = ReadMemoryStringW((size_t)pMemory[0], 128);
			tx = AddText(View, tx, y, crChar, 1, "%ls", sc.GetBuffer());
		}

		tx = AddText(View, tx, y, crChar, HS_NONE, _T("' ")) + g_FontWidth;
		tx = AddComment(View, tx, y);

		return y += g_FontHeight;
	}

public:
	CNodeBase* pNode;
	CMemory Memory;
};

class CNodeFloat : public CNodeBase
{
public:
	CNodeFloat() { nodeType = nt_float; }

	virtual void Update(HotSpot& Spot)
	{
		StandardUpdate(Spot);
		float v = (float)_ttof(Spot.Text);
		if (Spot.ID == HS_EDIT)
			ReClassWriteMemory((LPVOID)Spot.Address, &v, 4);
	}

	virtual int GetMemorySize() { return sizeof(float); }

	virtual int Draw(ViewInfo& View, int x, int y)
	{
		if (bHidden)
			return DrawHidden(View, x, y);

		float* pMemory = (float*)&View.pData[offset];
		AddSelection(View, 0, y, g_FontHeight);
		AddDelete(View, x, y);
		AddTypeDrop(View, x, y);
		//AddAdd(View,x,y);

		int tx = x + TXOFFSET;
		tx = AddIcon(View, tx, y, ICON_FLOAT, HS_NONE, HS_NONE);
		tx = AddAddressOffset(View, tx, y);
		tx = AddText(View, tx, y, crType, HS_NONE, _T("float "));
		tx = AddText(View, tx, y, crName, HS_NAME, _T("%s"), Name);
		tx = AddText(View, tx, y, crName, HS_NONE, _T(" = "));
		//tx = AddText(View,tx,y,crValue,0,"%.4f",pMemory[0]) + FontWidth;

		//if ( *pMemory > -99999.0f && *pMemory < 99999.0f )
		//	*pMemory = 0;

		tx = AddText(View, tx, y, crValue, HS_EDIT, _T("%4.3f"), pMemory[0]) + g_FontWidth;
		tx = AddComment(View, tx, y);

		return y += g_FontHeight;
	}
};

class CNodeDouble : public CNodeBase
{
public:
	CNodeDouble() { nodeType = nt_double; }

	virtual void Update(HotSpot& Spot)
	{
		StandardUpdate(Spot);
		double v = _ttof(Spot.Text);
		if (Spot.ID == 0)
			ReClassWriteMemory((LPVOID)Spot.Address, &v, 8);
	}

	virtual int GetMemorySize(void)
	{
		// doubles are always 64 bits ffs
		return sizeof(double);
	}

	virtual int Draw(ViewInfo& View, int x, int y)
	{
		if (bHidden)
			return DrawHidden(View, x, y);

		double* pMemory = (double*)&View.pData[offset];
		
		AddSelection(View, 0, y, g_FontHeight);
		AddDelete(View, x, y);
		AddTypeDrop(View, x, y);
		//AddAdd(View,x,y);

		int tx = x + TXOFFSET;
		tx = AddIcon(View, tx, y, ICON_FLOAT, HS_NONE, HS_NONE);
		tx = AddAddressOffset(View, tx, y);
		tx = AddText(View, tx, y, crType, HS_NONE, _T("double "));
		tx = AddText(View, tx, y, crName, HS_NAME, _T("%s"), Name);
		tx = AddText(View, tx, y, crName, HS_NONE, _T(" = "));
		//tx = AddText(View, tx, y, crValue, 0, "%.4lg", pMemory[0]) + FontWidth;
		tx = AddText(View, tx, y, crValue, HS_EDIT, _T("%.4g"), pMemory[0]) + g_FontWidth;
		tx = AddComment(View, tx, y);

		return y += g_FontHeight;
	}
};

class CNodeVec2 : public CNodeBase
{
public:
	CNodeVec2()
	{
		nodeType = nt_vec2;
		for (UINT i = 0; i < bOpen.size(); i++)
			bOpen[i] = true;
	}

	virtual void Update(HotSpot& Spot)
	{
		StandardUpdate(Spot);
		float v = (float)_ttof(Spot.Text);
		if (Spot.ID >= 0 && Spot.ID < 2)
			ReClassWriteMemory((LPVOID)(Spot.Address + (Spot.ID * 4)), &v, 4);
	}

	virtual int GetMemorySize() { return sizeof(float) * 2; }

	virtual int Draw(ViewInfo& View, int x, int y)
	{
		if (bHidden)
			return DrawHidden(View, x, y);

		float* pMemory = (float*)&View.pData[offset];
		AddSelection(View, 0, y, g_FontHeight);
		AddDelete(View, x, y);
		AddTypeDrop(View, x, y);

		int tx = x + TXOFFSET;
		tx = AddIcon(View, tx, y, ICON_VECTOR, HS_NONE, HS_NONE);
		tx = AddAddressOffset(View, tx, y);
		tx = AddText(View, tx, y, crType, HS_NONE, _T("Vec2 "));
		tx = AddText(View, tx, y, crName, HS_NAME, _T("%s"), Name);
		tx = AddOpenClose(View, tx, y);
		if (bOpen[View.Level])
		{
			tx = AddText(View, tx, y, crName, HS_NONE, _T("("));
			tx = AddText(View, tx, y, crValue, HS_EDIT, _T("%0.3f"), pMemory[0]);
			tx = AddText(View, tx, y, crName, HS_NONE, _T(","));
			tx = AddText(View, tx, y, crValue, HS_OPENCLOSE, _T("%0.3f"), pMemory[1]);
			tx = AddText(View, tx, y, crName, HS_NONE, _T(")"));
		}
		tx += g_FontWidth;
		tx = AddComment(View, tx, y);
		return (y + g_FontHeight);
	}
};

class CNodeVec3 : public CNodeBase
{
public:
	CNodeVec3()
	{
		nodeType = nt_vec3;
		for (UINT i = 0; i < bOpen.size(); i++)
			bOpen[i] = true;
	}

	virtual void Update(HotSpot& Spot)
	{
		StandardUpdate(Spot);
		float v = (float)_ttof(Spot.Text);
		if (Spot.ID >= 0 && Spot.ID < 3)
			ReClassWriteMemory((LPVOID)(Spot.Address + (Spot.ID * sizeof(float))), &v, sizeof(float));
	}

	virtual int GetMemorySize() { return sizeof(float) * 3; }

	virtual int Draw(ViewInfo& View, int x, int y)
	{
		if (bHidden) return DrawHidden(View, x, y);
		float* pMemory = (float*)&View.pData[offset];
		AddSelection(View, 0, y, g_FontHeight);
		AddDelete(View, x, y);
		AddTypeDrop(View, x, y);

		int tx = x + TXOFFSET;
		tx = AddIcon(View, tx, y, ICON_VECTOR, HS_NONE, HS_NONE);
		tx = AddAddressOffset(View, tx, y);
		tx = AddText(View, tx, y, crType, HS_NONE, _T("Vec3 "));
		tx = AddText(View, tx, y, crName, HS_NAME, _T("%s"), Name);
		tx = AddOpenClose(View, tx, y);
		if (bOpen[View.Level])
		{
			tx = AddText(View, tx, y, crName, HS_NONE, _T("("));
			tx = AddText(View, tx, y, crValue, 0, _T("%0.3f"), pMemory[0]);
			tx = AddText(View, tx, y, crName, HS_NONE, _T(","));
			tx = AddText(View, tx, y, crValue, 1, _T("%0.3f"), pMemory[1]);
			tx = AddText(View, tx, y, crName, HS_NONE, _T(","));
			tx = AddText(View, tx, y, crValue, 2, _T("%0.3f"), pMemory[2]);
			tx = AddText(View, tx, y, crName, HS_NONE, _T(")"));
		}
		tx += g_FontWidth;
		tx = AddComment(View, tx, y);

		return y + g_FontHeight;
	}
};

class CNodeQuat : public CNodeBase
{
public:
	CNodeQuat()
	{
		nodeType = nt_quat;
		for (UINT i = 0; i < bOpen.size(); i++)
			bOpen[i] = true;
	}

	virtual void Update(HotSpot& Spot)
	{
		StandardUpdate(Spot);
		float v = (float)_ttof(Spot.Text);
		if (Spot.ID >= 0 && Spot.ID < 4)
			ReClassWriteMemory((LPVOID)(Spot.Address + (Spot.ID * sizeof(float))), &v, sizeof(float));
	}

	virtual int GetMemorySize() { return sizeof(float) * 4; }

	virtual int Draw(ViewInfo& View, int x, int y)
	{
		if (bHidden)
			return DrawHidden(View, x, y);

		float* pMemory = (float*)&View.pData[offset];
		AddSelection(View, 0, y, g_FontHeight);
		AddDelete(View, x, y);
		AddTypeDrop(View, x, y);

		int tx = x + TXOFFSET;
		tx = AddIcon(View, tx, y, ICON_VECTOR, -1, -1);
		tx = AddAddressOffset(View, tx, y);
		tx = AddText(View, tx, y, crType, HS_NONE, _T("Vec4 "));
		tx = AddText(View, tx, y, crName, 69, _T("%s"), Name);
		tx = AddOpenClose(View, tx, y);
		if (bOpen[View.Level])
		{
			tx = AddText(View, tx, y, crName, HS_NONE, _T("("));
			tx = AddText(View, tx, y, crValue, 0, _T("%0.3f"), pMemory[0]);
			tx = AddText(View, tx, y, crName, HS_NONE, _T(","));
			tx = AddText(View, tx, y, crValue, 1, _T("%0.3f"), pMemory[1]);
			tx = AddText(View, tx, y, crName, HS_NONE, _T(","));
			tx = AddText(View, tx, y, crValue, 2, _T("%0.3f"), pMemory[2]);
			tx = AddText(View, tx, y, crName, HS_NONE, _T(","));
			tx = AddText(View, tx, y, crValue, 3, _T("%0.3f"), pMemory[3]);
			tx = AddText(View, tx, y, crName, HS_NONE, _T(")"));
		}
		tx += g_FontWidth;
		tx = AddComment(View, tx, y);

		return y + g_FontHeight;
	}
};

class CNodeMatrix : public CNodeBase
{
public:
	CNodeMatrix() { nodeType = nt_matrix; }

	virtual void Update(HotSpot& Spot)
	{
		StandardUpdate(Spot);
		if (Spot.ID < 16)
		{
			float v = (float)_ttof(Spot.Text);
			ReClassWriteMemory((LPVOID)(Spot.Address + (Spot.ID * sizeof(float))), &v, sizeof(float));
		}
	}

	virtual int GetMemorySize() { return 4 * 4 * sizeof(float); }

	virtual int Draw(ViewInfo& View, int x, int y)
	{
		if (bHidden) return DrawHidden(View, x, y);
		float* pMemory = (float*)&View.pData[offset];
		AddSelection(View, 0, y, g_FontHeight);
		AddDelete(View, x, y);
		AddTypeDrop(View, x, y);

		int tx = x + TXOFFSET;
		tx = AddIcon(View, tx, y, ICON_MATRIX, HS_NONE, HS_NONE);
		int mx = tx;
		tx = AddAddressOffset(View, tx, y);
		tx = AddText(View, tx, y, crType, HS_NONE, _T("Matrix "));
		tx = AddText(View, tx, y, crName, 69, _T("%s"), Name);
		tx = AddOpenClose(View, tx, y);
		tx += g_FontWidth;
		tx = AddComment(View, tx, y);

		if (bOpen[View.Level])
		{
			y += g_FontHeight;
			tx = mx;
			tx = AddText(View, tx, y, crName, HS_NONE, _T("|"));
			tx = AddText(View, tx, y, crValue, 0, _T("% 14.3f"), pMemory[0]);
			tx = AddText(View, tx, y, crName, HS_NONE, _T(","));
			tx = AddText(View, tx, y, crValue, 1, _T("% 14.3f"), pMemory[1]);
			tx = AddText(View, tx, y, crName, HS_NONE, _T(","));
			tx = AddText(View, tx, y, crValue, 2, _T("% 14.3f"), pMemory[2]);
			tx = AddText(View, tx, y, crName, HS_NONE, _T(","));
			tx = AddText(View, tx, y, crValue, 3, _T("% 14.3f"), pMemory[3]);
			tx = AddText(View, tx, y, crName, HS_NONE, _T("|"));
			y += g_FontHeight;
			tx = mx;
			tx = AddText(View, tx, y, crName, HS_NONE, _T("|"));
			tx = AddText(View, tx, y, crValue, 4, _T("% 14.3f"), pMemory[4]);
			tx = AddText(View, tx, y, crName, HS_NONE, _T(","));
			tx = AddText(View, tx, y, crValue, 5, _T("% 14.3f"), pMemory[5]);
			tx = AddText(View, tx, y, crName, HS_NONE, _T(","));
			tx = AddText(View, tx, y, crValue, 6, _T("% 14.3f"), pMemory[6]);
			tx = AddText(View, tx, y, crName, HS_NONE, _T(","));
			tx = AddText(View, tx, y, crValue, 7, _T("% 14.3f"), pMemory[7]);
			tx = AddText(View, tx, y, crName, HS_NONE, _T("|"));
			y += g_FontHeight;
			tx = mx;
			tx = AddText(View, tx, y, crName, HS_NONE, _T("|"));
			tx = AddText(View, tx, y, crValue, 8, _T("% 14.3f"), pMemory[8]);
			tx = AddText(View, tx, y, crName, HS_NONE, _T(","));
			tx = AddText(View, tx, y, crValue, 9, _T("% 14.3f"), pMemory[9]);
			tx = AddText(View, tx, y, crName, HS_NONE, _T(","));
			tx = AddText(View, tx, y, crValue, 10, _T("% 14.3f"), pMemory[10]);
			tx = AddText(View, tx, y, crName, HS_NONE, _T(","));
			tx = AddText(View, tx, y, crValue, 11, _T("% 14.3f"), pMemory[11]);
			tx = AddText(View, tx, y, crName, HS_NONE, _T("|"));
			y += g_FontHeight;
			tx = mx;
			tx = AddText(View, tx, y, crName, HS_NONE, _T("|"));
			tx = AddText(View, tx, y, crValue, 12, _T("% 14.3f"), pMemory[12]);
			tx = AddText(View, tx, y, crName, HS_NONE, _T(","));
			tx = AddText(View, tx, y, crValue, 13, _T("% 14.3f"), pMemory[13]);
			tx = AddText(View, tx, y, crName, HS_NONE, _T(","));
			tx = AddText(View, tx, y, crValue, 14, _T("% 14.3f"), pMemory[14]);
			tx = AddText(View, tx, y, crName, HS_NONE, _T(","));
			tx = AddText(View, tx, y, crValue, 15, _T("% 14.3f"), pMemory[15]);
			tx = AddText(View, tx, y, crName, HS_NONE, _T("|"));
		}

		return y + g_FontHeight;
	}
};

class CNodeArray : public CNodeBase
{
public:
	CNodeArray()
	{
		nodeType = nt_array;
		Total = 1;
		Current = 0;
	}

	virtual void Update(HotSpot& Spot)
	{
		StandardUpdate(Spot);
		int v = _ttoi(Spot.Text);
		if (v < 0)
			return;

		if (Spot.ID == 0)
		{
			if (v == 0)
				return;
			Total = (DWORD)v;
		}
		else if (Spot.ID == 1)
		{
			if (v >= (int)Total)
				return;
			Current = (DWORD)v;
		}
		else if (Spot.ID == 2)
		{
			if (Current > 0)
				Current--;
		}
		else if (Spot.ID == 3)
		{
			if (Current < Total - 1)
				Current++;
		}
	};

	virtual int GetMemorySize() { return pNode->GetMemorySize() * Total; }

	virtual int Draw(ViewInfo& View, int x, int y)
	{
		if (bHidden)
			return DrawHidden(View, x, y);
		AddSelection(View, 0, y, g_FontHeight);
		AddDelete(View, x, y);
		AddTypeDrop(View, x, y);

		x = AddOpenClose(View, x, y);
		x = AddIcon(View, x, y, ICON_ARRAY, -1, -1);

		int tx = x;
		tx = AddAddressOffset(View, tx, y);

		tx = AddText(View, tx, y, crType, HS_NONE, _T("Array "));
		tx = AddText(View, tx, y, crName, HS_NAME, _T("%s"), Name);
		tx = AddText(View, tx, y, crIndex, HS_NONE, _T("["));
		tx = AddText(View, tx, y, crIndex, HS_EDIT, _T("%i"), Total);
		tx = AddText(View, tx, y, crIndex, HS_NONE, _T("]"));

		tx = AddIcon(View, tx, y, ICON_LEFT, HS_SELECT, HS_CLICK);
		tx = AddText(View, tx, y, crIndex, HS_NONE, _T("("));
		tx = AddText(View, tx, y, crIndex, 1, _T("%i"), Current);
		tx = AddText(View, tx, y, crIndex, HS_NONE, _T(")"));
		tx = AddIcon(View, tx, y, ICON_RIGHT, HS_DROP, HS_CLICK);

		tx = AddText(View, tx, y, crValue, HS_NONE, _T("<%s Size=%i>"), pNode->Name, GetMemorySize());
		tx = AddIcon(View, tx, y, ICON_CHANGE, HS_CLICK, HS_CHANGE_X);

		tx += g_FontWidth;
		tx = AddComment(View, tx, y);

		y += g_FontHeight;
		if (bOpen[View.Level])
		{
			ViewInfo newView;
			newView = View;
			newView.Address = View.Address + offset + pNode->GetMemorySize() * Current;
			newView.pData = (unsigned char*)((size_t)View.pData + offset + pNode->GetMemorySize() * Current);
			y = pNode->Draw(newView, x, y);
		};
		return y;
	}

public:
	CNodeBase* pNode;
	DWORD Total;
	DWORD Current;
};

class CNodeClassInstance : public CNodeBase
{
public:
	CNodeClassInstance() { nodeType = nt_instance; }

	virtual void Update(HotSpot& Spot) { StandardUpdate(Spot); }

	virtual int GetMemorySize() { return pNode->GetMemorySize(); }

	virtual int Draw(ViewInfo& View, int x, int y)
	{
		if (bHidden)
			return DrawHidden(View, x, y);

		AddSelection(View, 0, y, g_FontHeight);
		AddDelete(View, x, y);
		AddTypeDrop(View, x, y);

		x = AddOpenClose(View, x, y);
		x = AddIcon(View, x, y, ICON_CLASS, -1, -1);

		int tx = x;
		tx = AddAddressOffset(View, tx, y);

		tx = AddText(View, tx, y, crType, HS_NONE, _T("Instance "));
		tx = AddText(View, tx, y, crName, HS_NAME, _T("%s"), Name);
		tx = AddText(View, tx, y, crValue, HS_NONE, _T("<%s>"), pNode->Name);
		tx = AddIcon(View, tx, y, ICON_CHANGE, HS_CLICK, HS_CHANGE_X);

		tx += g_FontWidth;
		tx = AddComment(View, tx, y);

		y += g_FontHeight;
		if (bOpen[View.Level])
		{
			ViewInfo newView;
			newView = View;
			newView.Address = View.Address + offset;
			newView.pData = (unsigned char*)((size_t)newView.pData + offset);
			y = pNode->Draw(newView, x, y);
		}

		return y;
	}

public:
	CNodeClass* pNode;
};

class CNodeCustom : public CNodeBase
{
public:
	CNodeCustom()
	{
		nodeType = nt_custom;
		Name = _T("Custom");
		memsize = sizeof(size_t);
	}

	virtual void Update(HotSpot& Spot)
	{
		StandardUpdate(Spot);
		if (Spot.ID == 0)
			memsize = _ttoi(Spot.Text);
	}

	virtual int GetMemorySize() { return memsize; }

	virtual int Draw(ViewInfo& View, int x, int y)
	{
		if (bHidden)
			return DrawHidden(View, x, y);

		AddSelection(View, 0, y, g_FontHeight);
		AddDelete(View, x, y);
		AddTypeDrop(View, x, y);
		//AddAdd(View,x,y);

		int tx = x + TXOFFSET;
		tx = AddIcon(View, tx, y, ICON_CUSTOM, HS_NONE, HS_NONE);
		tx = AddAddressOffset(View, tx, y);
		tx = AddText(View, tx, y, crType, HS_NONE, _T("Custom "));
		tx = AddText(View, tx, y, crIndex, HS_NONE, _T("["));
		tx = AddText(View, tx, y, crIndex, HS_EDIT, _T("%i"), memsize);
		tx = AddText(View, tx, y, crIndex, HS_NONE, _T("] "));
		tx = AddText(View, tx, y, crName, HS_NAME, _T("%s"), Name) + g_FontWidth;
		tx = AddComment(View, tx, y);
		return y += g_FontHeight;
	}

public:
	DWORD memsize;
};

