#include <common/target.h>
#pragma hdrstop

#include <common/utils.h>
#include <common/indicationIntf.h>

// play binary sequence, from end to start
void beepBytes(const BYTE* buff, BYTE len, DWORD freq0, DWORD freq1, DWORD duration0, DWORD duration1, DWORD pause)
{
	if( buff && len )
	{
		BOOL playedOnce = FALSE;
		while(len--)
		{							
			BYTE b = buff[len];
			if( 0 != b || (0 == b && playedOnce) )
			{
				BYTE bit = 8;
				while( bit-- )
				{
					if( b & (1 << bit) )
					{
						playedOnce = TRUE;
						beep(freq1, duration1);
						usDelay((pause+duration1)*1000);
					}
					else if( playedOnce )
					{
						beep(freq0, duration0);
						usDelay((pause+duration0)*1000);
					}
				}	
			}
		}
	}
}
