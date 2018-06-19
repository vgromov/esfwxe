#include <esfwxe/target.h>
#pragma hdrstop

#include <esfwxe/utils.h>
#include <esfwxe/indicationIntf.h>

#ifdef USE_SOUND_INDICATION
// play binary sequence, from end to start
void beepBytes(const esU8* buff, esU8 len, esU32 freq0, esU32 freq1, esU32 duration0, esU32 duration1, esU32 pause)
{
	if( buff && len )
	{
		esBL playedOnce = FALSE;
		beepStop();
		while(len--)
		{							
			esU8 b = buff[len];
			if( 0 != b || (0 == b && playedOnce) )
			{
				esU8 bit = 8;
				while( bit-- )
				{
					if( b & (1 << bit) )
					{
						playedOnce = TRUE;
						beep(freq1, duration1);
						beep(0, pause);
					}
					else if( playedOnce )
					{
						beep(freq0, duration0);
						beep(0, pause);
					}
				}	
			}
		}
	}
}
#endif
