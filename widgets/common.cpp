
#include "common.h"

#include <stdlib.h>
#include <stdio.h>

#include <afx.h>


/* ���ں��ֱ���

�Ŀ� http://topic.csdn.net/t/20061025/17/5108934.html

�ڴ����״����ʱ�õ�TextOut����TextOut�޷��Զ����У�ֻ�ö���д���룬���Ǵ������ֲ���ʱ��ס�ˣ����������жϺ��ֵ����ӣ������ɲ��꣬����һ������о������������һ���򵥵Ľ̡̳�   
    
  ��ʵ�жϺ��ֵĴ���ܼ򵥣��򵥵�ֻ��һ�оͿ����ˣ�������⺺�ֱ���Ļ�������һ�д���ȴ�����޷���⡣   
    
  ���õ��ַ��������������֣�   
  1��ASCII   ֻ֧��Ӣ�ģ�ȫ��Ϊ8λ   
  2��DBCS     ֧��Ӣ�ĺ����ģ���������Ҫ�����ֽڣ�16λ��   
  3��UNICODE   ֧��Ӣ�ĺ����ģ�Ӣ�ĺ����Ķ���Ҫ�����ֽ�   
    
  ASCII��DOSʱ���ģ��޷�֧�����ġ�DBCS��Win9x֧�ֵ��ַ�����UNICODE��win2k��xp֧�ֵ��ַ�����   
  �����ֵı���Ŀǰ��GB2312-1980��GB18030-2000��GB2312����DBCS���͵ĺ��ֱ��룬GB18030����UNICODE�ĺ��ֱ��룬��ȻGB18030����GB2312��Ҳ����˵GB18030Ҳ֧��DBCS���ַ�����ʽ����ȻGB18030��2000���ǿ��ִ�еĹ��ұ�׼����Ŀǰʹ�����Ļ���GB2312���룬����GB2312Ҳ�㹻���������õ��ĺ����ˡ�����GBK������GB2312����ǿ�档   
    
  �����ҽ�֧��GB2312������GB2312���й���½�ƶ��ı�׼�����Է������Ĳ�����GB2312�ı����У������ĳ�����Ҫ֧�ַ������ģ�����Ҫ����Big5���롣��ʵҲ�ܼ򵥡�   
    
  ����˵һ����C����δ���GB2312����ĺ��֡���VC6.0Ϊ���������������������Ϊwchar_t����UNICODE���룬�����char����DBCS��������ҵ�һ�����������ǣ�   
  void   Justify   (HDC   hdc,   PTSTR   pText,   RECT   *   prc,   int   iAlign)           
  ���е�pText��PTSTR���ͣ�PTSTR��WINNT.H�����������壨WINNT.H�е���δ�����ɾ�����м��޹صĲ��֣�   
  #ifdef     UNICODE   
  typedef   LPWSTR   PTSTR,   LPTSTR; //�����UNICODE���룬����PTSTRΪLPWSTR����   
  #else   
  typedef   LPSTR   PTSTR,   LPTSTR; //����ǲ���UNICODE���룬����PTSTRΪLPSTR����   
  #endif   
    
  LPSTR����ΪCHAR��ָ��   
  LPSWSTR����ΪWCHAR��ָ��   
  CHAR����Ϊchar����   
  WCHAR����Ϊwchar_t����   
  ��wchar_t����Ϊunsigned   short���ͣ�����16λ�������ֽڣ��޷��Ŷ�����   
    
  ��UNICODE���Ƿ�UNICODEȡ������ı���ѡ������[����]-[ѡ��]-[C/C++]��[Ԥ���������]��������_UNICODE����ô�������wchar_tָ��������LPSTR�����û��_UNICODE����ô�������charָ��������LPSTR������������������ǣ�����ܵ���pText�е��ֽ������ǲ�һ���ģ�[i����you]����ִ������û�ж���_UNICODE������£���8���ֽڣ����ڶ�����_UNICODE���������12���ֽڡ���ӳ�������о��ǣ����û�ж���_UNICODE����ô��Ҫ��Ӣ���ַ�����1���ֽ��������������ַ��ı����ǲ���GB2312����淶���ģ����������_UNICODE����ôӢ���ַ�Ҫ����2���ֽ��������������ַ��ı����ǲ���UNICODE�������ġ�������˵��win98������UNICODE��������õ���DBCS���룬Ϊ�����ҵĳ���ȿ�����XP�������ֿ�����Win98�����У���û�ж���_UNICODE�������ҵĳ�������Ҫ���ַ�������DBCS����������Ҳ����Ӣ���ַ���1���ֽڣ������ַ���2���ֽڣ����ı������GB2312���롣��Justify��˵����   
  �Ҹ�pText������[��]������֣���ôpTextӦ���������ֽ������[��]����֣����öε�����һ��pText�����ݡ�   
  void   Justify   (HDC   hdc,   PTSTR   pText,   RECT   *   prc,   int   iAlign)   
  {   
  static   TCHAR   szText[]   =   {TEXT   ("��")}   ;   
    
  pText=szText;   
  }   
  ��ϵ��Իᷢ��*pText=-60����ô�������أ�ԭ����û�а�unsigned   char��ת��*pText��ֵ���޸Ĵ������£�   
  void   Justify   (HDC   hdc,   PTSTR   pText,   RECT   *   prc,   int   iAlign)   
  {   
  static   TCHAR   szText[]   =   {TEXT   ("��")}   ;   
  unsigned   char   sqChar[20]; //�����������Ϊ��ǿ��ת�������õ�   
    
  pText=szText;   
  sqChar[0]=*pText;   
  sqChar[1]=*(pText+1);   
  }   
  ��ʱ�¶ϲ鿴sqChar[0]=196,sqChar[1]=227�Ͷ��ˣ�Ϊʲô�أ���Ϊ����GB2312�ı�����һ���ġ�   
  GB2312-80����ı��뷶Χ�Ǹ�λ0xa1��0xfe����λ��   0xa1-0xfe   �����к��ַ�ΧΪ   0xb0a1   ��   0xf7fe�����ֻ�Ǽ򵥵��жϺ��֣���ֻҪ�鿴���ֽ��Ƿ���ڵ���0xa1�Ϳ����ˣ����о��ǣ�ȫ���ַ��ĸ��ֽ�ͳͳ����0xa3�����Ժ����׿��������ȫ���ַ�����   
    
  �����ϣ����ĳ�����֧�ֵ�GB18030����ô��ȥ��GB18030�Ĺ淶��������   
    
  �����кܶ��жϺ��ֵ�˵������ֻҪ��ס����Ҫ֧�ֵı������ĸ���GB2312��GBK��GB18030��ÿ���������Լ��ı��뷶Χ���߹淶������֮�����в�ͬ��˵����������Ϊ���ǻ���˵�Ĳ���ͬһ�ֱ��뷽ʽ��������˵����GB2312�ı��룬�����ĳ���Ҫ�����֧��UNICODE�Ļ�����ô��δ����Ҫ�޸ĳɶ�ӦUNICODE�淶�Ĵ����ˡ�   
    
  ���򵥵Ĳ��Ժ��֡�ȫ���ַ���Ӣ�ĵĳ���   
    
  //test.c   
  //Դ�������ߣ��Ŀ�   sequh@126.com   
  //�½�Win32   Application���̣���test.c���룬���У������޸�szText��ֵ�����۲����Ч��   
    
  #include   <windows.h>   
    
  int   WINAPI   WinMain   (HINSTANCE   hInstance,   HINSTANCE   hPrevInstance,   
  PSTR   szCmdLine,   int   iCmdShow)   
  {   
  static   TCHAR   szText[]   =   {TEXT   ("i��,�ˡ�u��")}   ;   
  PTSTR   pText;   
  int   i;   
  unsigned   char   sqChar[20];   
    
  pText=szText;   
  while   (*pText   !=   '\0')   
  {   
  i=IsGB(pText);   
    
  switch(i)   
  {   
  case   0:   
  pText++;   
  MessageBox   (NULL,   TEXT   ("�������֡�Ӣ���ַ���Ӣ�ı��"),   TEXT   ("Hello"),   0);   
  break;   
  case   1:   
  pText++;   
  pText++;   
  MessageBox   (NULL,   TEXT   ("����ȫ���ַ�"),   TEXT   ("Hello"),   0);   
  break;   
  case   2:   
  pText++;   
  pText++;   
  MessageBox   (NULL,   TEXT   ("���ֺ���"),   TEXT   ("Hello"),   0);   
  break;   
  }   
  }   
    
  return   0   ;   
  }   
    
  int   IsGB(PTSTR   pText)   
  {   
  unsigned   char   sqChar[20];   
  sqChar[0]=*pText;   
  if   (sqChar[0]>=0xa1)   
	if   (sqChar[0]==0xa3)   
		return   1; //ȫ���ַ�   
	else   
		return   2; //����   
  else   
  return   0; //Ӣ�ġ����֡�Ӣ�ı��   
  } 
*/

bool IntersectRect(WRect *prcDest, const WRect *prcSrc1, const WRect *prcSrc2)
{
   int aleft, atop, aright, abottom;   // coordinates of prcSrc1
   int bleft, btop, bright, bbottom;   // coordinates of prcSrc2

   // get the left, top, right, bottom coordinates of each source rect
   RCCOORD(prcSrc1, aleft, atop, aright, abottom);
   RCCOORD(prcSrc2, bleft, btop, bright, bbottom);

   // early rejection test
   if (aleft > bright || atop > bbottom || bleft > aright || btop > abottom) {
      if (prcDest) {
         SETAEERECT(prcDest, 0, 0, 0, 0);
      }
      return 0;   // empty intersection
   }

   if (prcDest) {
      prcDest->x  = MAX(aleft, bleft);
      prcDest->y  = MAX(atop, btop);
      prcDest->dx = MIN(aright, bright) - prcDest->x+1;
      prcDest->dy = MIN(abottom, bbottom) - prcDest->y + 1;
   }
   
   return 1;
}

/* returns FALSE if either source rectangle is empty */
void UnionRect(WRect *prcResult, const WRect *prcSrc1, const WRect *prcSrc2)
{

   int aleft, atop, aright, abottom;   // coordinates of prcSrc1
   int bleft, btop, bright, bbottom;   // coordinates of prcSrc2


   // get the left, top, right, bottom coordinates of each source rect
   GETCORNERS(prcSrc1, aleft, atop, aright, abottom);
   GETCORNERS(prcSrc2, bleft, btop, bright, bbottom);

   prcResult->x = MIN(aleft, bleft);
   prcResult->dx = MAX(aright, bright) - prcResult->x;

   prcResult->y = MIN(atop, btop);
   prcResult->dy = MAX(abottom, bbottom) - prcResult->y;
}

void* EG_MALLOC(unsigned int len)
{
	void* p=malloc(len);
//	TRACE("=MEM= allocated of %d @ %x\n",len,(unsigned int)p);
	return p;
}


void EG_FREE(void* p)
{
	free(p);
	//TRACE("=MEM= free @%x\n",(unsigned int)p);
}
/* Minimum UCS2 which UTF8 is 2 bytes form */
#define CONV_2BYTE_UTF8_MIN_UCS2     0x0080

/* Minimum UCS2 which UTF8 is 3 bytes form */
#define CONV_3BYTE_UTF8_MIN_UCS2     0x0800

/* if a code is unrecognized,  it will be converted to blackbox(UCS2 0x220E) */
#define DL_UCS2_UNRECOGNIZED_CODE 0x220E

uint16
DL_Get_word_at_odd_address( uint8 * input_ptr)
{
    uint8 tmp[2];
    uint16 output_word;
 
    tmp[0] = *input_ptr;
    tmp[1] = *(input_ptr+1);
 
    output_word = *(uint16 *)tmp;
 
    return( output_word );
}


uint8
ConvertUCS2toUTF8Code(uint16  ucs2_code,
                           uint8 * target_ptr)
{
    uint8 *out = target_ptr;
    uint8  target_len = 0;

    if( ucs2_code < CONV_2BYTE_UTF8_MIN_UCS2 )
    /* This UTF8 character is 1 byte.*/
    {
        if(target_ptr != NULL)
        {
            *out = (uint8) ucs2_code;
        }
        target_len++;
    }
    else if ( ucs2_code < CONV_3BYTE_UTF8_MIN_UCS2 )
    /* This UTF8 character is 2 byte.*/
    {
        if(target_ptr != NULL)
        {
            *out = ((uint8)(ucs2_code >> 6)) | 0xC0;
            *(out+1) = ((uint8)(ucs2_code & 0x003F)) | 0x80;
        }
        target_len += 2;
    }  
    else
    /* This UTF8 character is 3 byte.*/
    {
        if(target_ptr != NULL)
        {
            *out = ((uint8)(ucs2_code >> 12)) | 0xE0;
            *(out+1) = ((uint8)((ucs2_code & 0x0FC0)>> 6)) | 0x80;
            *(out+2) = ((uint8)(ucs2_code & 0x003F)) | 0x80;
        }
        target_len += 3;               
    }

    return ( target_len );    
}

uint16
ConvertUCS2toUTF8String( const uint8 * source_ptr,
                              uint16  source_len,
                              uint8 * target_ptr,
                              uint16  target_buf_len)
{
    uint16 ucs2_code;
    
    const uint8 *in  = source_ptr;
    uint8 *out = target_ptr;
    
    uint8 utf8_code_len = 0;
    uint16 target_len = 0;

    if (source_ptr == NULL)
    {
        /*the return value is 0.*/
        return ( target_len );
    }

    /* if source_len is an odd number, the last byte will be ignored */
    source_len >>= 1;
    source_len <<= 1;

    while ( in < (source_ptr + source_len))
    {
        /* get one ucs2 code */
        ucs2_code = DL_Get_word_at_odd_address((uint8 *)in);
        if(ucs2_code == 0x0000)
        {
            break;
        }

        /* convert one ucs2 code */
        utf8_code_len = ConvertUCS2toUTF8Code(ucs2_code,out);

        if ((out != NULL) &&
            ((target_len + utf8_code_len) > target_buf_len-1))
        {
            break;
        }
        if (out != NULL)
        {
            out += utf8_code_len;
        }
        target_len += utf8_code_len;
        in += 2;
    }
    if ((target_len < target_buf_len) && (target_ptr != NULL))
    {
        *out=0x00;
    }  
    return ( target_len );    
}

void
DL_Put_word_at_odd_address( uint16 input_word,
                            uint8 * output_ptr)
{
    uint8 tmp[2];
 
    if (output_ptr != NULL)
    {
        *((uint16 *)tmp) = input_word;
 
        *output_ptr     = tmp[0];
        *(output_ptr+1) = tmp[1];
    }
}

uint16
ConvertUTF8toUCS2String(const uint8 * source_ptr,
                             uint16  source_len, 
                             uint8 * target_ptr, 
                             uint16  target_buf_len)
{
    BOOL is_unrecognize_byte = FALSE;
    uint16 ucs2_tmp1,ucs2_tmp2;

    const uint8* in = source_ptr;
    uint8* out = target_ptr;
    
    uint16 target_len = 0;

    while((in < source_ptr + source_len) &&
          (((target_len + 2) <= target_buf_len-2)||
           (target_ptr == NULL)))
    {
        if (*in == 0x00)
        {
            break;
        }
 
        /* This UTF8 character is 1 byte.*/
        if((*in & 0x80) == 0x00)
        {
            if(out != NULL)
            {
                DL_Put_word_at_odd_address((uint16)(*in),out);
                out += 2;
            }
            in++;
            target_len += 2;

            is_unrecognize_byte = FALSE;
        }
        /* This UTF8 character is 2 byte.*/
        else if((in < (source_ptr + source_len - 1)) &&
                ((*in     & 0xE0) == 0xC0) &&
                ((*(in+1) & 0xC0) == 0x80))
        {
            ucs2_tmp1 = (uint16)(*in & 0x1F);
            ucs2_tmp1 <<= 6;
            ucs2_tmp1 |= (uint16)(*(in+1) & 0x3F);
            
            if(out != NULL)
            {
                DL_Put_word_at_odd_address(ucs2_tmp1,out);
                out += 2;
            }
            in += 2;
            target_len +=2; 

            is_unrecognize_byte = FALSE;
        }
        /* This UTF8 character is 3 byte.*/ 
        else if((in < (source_ptr + source_len - 2)) &&
                ((*in     & 0xF0) == 0xE0) &&
                ((*(in+1) & 0xC0) == 0x80) &&
                ((*(in+2) & 0xC0) == 0x80))
        {
            ucs2_tmp1 = (uint16)(*in & 0x0F);
            ucs2_tmp1 <<= 12;
            ucs2_tmp2 = (uint16)(*(in+1) & 0x3F);
            ucs2_tmp2 <<= 6;
            ucs2_tmp1 = ucs2_tmp1 | ucs2_tmp2 | (uint16)(*(in+2) & 0x3F);

            if(out != NULL)
            {
                DL_Put_word_at_odd_address(ucs2_tmp1,out);
                out +=2;
            }
            in +=3;
            target_len +=2;

            is_unrecognize_byte = FALSE;
        }
        /* This is an unrecognize byte. */ 
        else 
        {
            in++;
            
            /*Continuous un-recognized bytes will only add one blackbox*/
            if( is_unrecognize_byte == FALSE )
            {
                if(out != NULL)
                {
                    DL_Put_word_at_odd_address(DL_UCS2_UNRECOGNIZED_CODE,out);
                    out +=2;
                }
                target_len +=2;      
            }
            is_unrecognize_byte = TRUE;
        }           
    }
    if (( target_len+2 <= target_buf_len) && (target_ptr != NULL))
    {
        *out = 0x00;
        *(out+1) = 0x00;
    } 
    return ( target_len );
}
uint16
util_get_wchar ( const uint16 * s )
{

#if (BIG_ENDIAN == TRUE)
    return ( ( ( ( uint16 ) ( * ( ( uint8 * ) s     ) ) ) << 8 ) |
             ( ( ( uint16 ) ( * ( ( uint8 * ) s + 1 ) ) )      ) );
#else
    return ( ( ( ( uint16 ) ( * ( ( uint8 * ) s + 1 ) ) ) << 8 ) |
             ( ( ( uint16 ) ( * ( ( uint8 * ) s     ) ) )      ) );
#endif
             
}
void
util_put_wchar ( uint16 * dest, uint16 c )
{
    memcpy ( ( uint8 * ) dest, ( uint8 * ) & c, sizeof ( uint16 ) );
}

UINT32
util_wstrlen ( const uint16 * s )
{
    UINT32 i = 0;

    /* iterate through entire string */
    while ( util_get_wchar ( s++ ) != UNICODE_NULL )
    {
        i++;
    }

    return ( i );
}
int   IsGB(uint8 ch)   
{   


if   (ch>=0xa1)   
if   (ch==0xa3)   
	return   1; //ȫ���ַ�   
else   
	return   2; //����   
else   
return   0; //Ӣ�ġ����֡�Ӣ�ı��   
} 
int str2wstr(uint8* str,int slen,uint16* wstr,int wslen)
{

	int len=0;
	while(slen>0)
	{
		uint8 ch = *(str++);
		if(IsGB(ch)==0)
		{
			*(wstr++)=ch;
			
		}
		else
		{
			uint16* wch=(wstr++);
			uint8 ch2=*(str++);

			*wch=ch2<<8 | ch;
			slen--;
		}
		len++;
		slen--;
	}
	*wstr=UNICODE_NULL;
	return len;
	
}



uint16 *
util_wstrcpy ( uint16 * dest, const uint16 * source )
{
    /* save the start of result string */
    uint16 * str_ptr = dest;
    uint16   curr_char;

    /* iterate through both strings, copying one character at a time until the
       end is reached */
    while ( ( curr_char = util_get_wchar ( source++ ) ) != UNICODE_NULL )
    {
        util_put_wchar ( dest++, curr_char );
    }

    /* null-terminate output string */
   util_put_wchar ( dest, UNICODE_NULL );

    return ( str_ptr );
}

uint16 *
util_wstrncpy ( uint16 * dest, const uint16 * source, UINT32 num )
{
    /* save the start of result string */
    uint16 * str_ptr = dest;
    uint16   curr_char;

    /* only proceed if 'n' is valid */
    if ( num > 0 )
    {
        /* iterate through both strings, copying one character at a time until
           the end or limit is reached */
        while ( ( ( curr_char = util_get_wchar ( source++ ) ) != UNICODE_NULL ) &&
                ( num-- ) )
        {
            util_put_wchar ( dest++, curr_char );
        }

        /* null-terminate output string */
        util_put_wchar ( dest, UNICODE_NULL );
    }

    return ( str_ptr );
}


 