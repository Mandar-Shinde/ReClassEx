#include "stdafx.h"
#include "CNodeWORD.h"

CNodeWORD::CNodeWORD( )
{
	m_nodeType = nt_uint16;
}

void CNodeWORD::Update( const HotSpot& Spot )
{
	StandardUpdate( Spot );
	WORD v = (WORD)_tcstoul( Spot.Text.GetString( ), NULL, g_bUnsignedHex ? 16 : 10 );
	if (Spot.ID == 0)
		ReClassWriteMemory( (LPVOID)Spot.Address, &v, sizeof( unsigned short ) );
}

NodeSize CNodeWORD::Draw( const ViewInfo& View, int x, int y )
{
	if (m_bHidden)
		return DrawHidden( View, x, y );

	NodeSize drawnSize = { 0 };
	WORD* pMemory = (WORD*)&View.pData[m_Offset];
	AddSelection( View, 0, y, g_FontHeight );
	AddDelete( View, x, y );
	AddTypeDrop( View, x, y );
	//AddAdd(View,x,y);

	int tx = x + TXOFFSET;
	tx = AddIcon( View, tx, y, ICON_UNSIGNED, HS_NONE, HS_NONE );
	tx = AddAddressOffset( View, tx, y );
	tx = AddText( View, tx, y, g_crType, HS_NONE, _T( "WORD  " ) );
	tx = AddText( View, tx, y, g_crName, HS_NAME, _T( "%s" ), m_strName );
	tx = AddText( View, tx, y, g_crName, HS_NONE, _T( " = " ) );
	tx = AddText( View, tx, y, g_crValue, HS_EDIT, g_bUnsignedHex ? _T( "0x%X" ) : _T( "%u" ), pMemory[0] ) + g_FontWidth;
	tx = AddComment( View, tx, y );

	drawnSize.x = tx;
	drawnSize.y = y + g_FontHeight;
	return drawnSize;
}
