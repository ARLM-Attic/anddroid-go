
#include "common.h"

#include <stdlib.h>
#include <stdio.h>

#include <afx.h>


/* 关于汉字编码

夏克 http://topic.csdn.net/t/20061025/17/5108934.html

在处理套打程序时用到TextOut，但TextOut无法自动换行，只好动手写代码，可是处理到汉字部分时卡住了，搜索网上判断汉字的帖子，都语焉不详，经过一下午的研究，终于整理出一个简单的教程。   
    
  其实判断汉字的代码很简单，简单到只有一行就可以了，但不理解汉字编码的基础，这一行代码却万万无法理解。   
    
  常用的字符编码有以下三种：   
  1、ASCII   只支持英文，全部为8位   
  2、DBCS     支持英文和中文，但中文需要两个字节（16位）   
  3、UNICODE   支持英文和中文，英文和中文都需要两个字节   
    
  ASCII是DOS时代的，无法支持中文。DBCS是Win9x支持的字符集。UNICODE是win2k和xp支持的字符集。   
  而汉字的编码目前有GB2312-1980和GB18030-2000，GB2312就是DBCS类型的汉字编码，GB18030就是UNICODE的汉字编码，当然GB18030兼容GB2312，也就是说GB18030也支持DBCS的字符处理方式。虽然GB18030是2000年后强制执行的国家标准，但目前使用最多的还是GB2312编码，而且GB2312也足够处理你所用到的汉字了。还有GBK编码是GB2312的增强版。   
    
  这里我仅支持GB2312。由于GB2312是中国大陆制定的标准，所以繁体中文并不在GB2312的编码中，如果你的程序需要支持繁体中文，则还需要处理Big5编码。其实也很简单。   
    
  下面说一下在C中如何处理GB2312编码的汉字。以VC6.0为例，如果声明变量的类型为wchar_t则是UNICODE编码，如果是char则是DBCS编码比如我的一个函数声明是：   
  void   Justify   (HDC   hdc,   PTSTR   pText,   RECT   *   prc,   int   iAlign)           
  其中的pText是PTSTR类型，PTSTR在WINNT.H中有两个定义（WINNT.H中的这段代码我删掉了中间无关的部分）   
  #ifdef     UNICODE   
  typedef   LPWSTR   PTSTR,   LPTSTR; //如果是UNICODE编码，则定义PTSTR为LPWSTR类型   
  #else   
  typedef   LPSTR   PTSTR,   LPTSTR; //如果是不是UNICODE编码，则定义PTSTR为LPSTR类型   
  #endif   
    
  LPSTR定义为CHAR的指针   
  LPSWSTR定义为WCHAR的指针   
  CHAR定义为char类型   
  WCHAR定义为wchar_t类型   
  而wchar_t定义为unsigned   short类型，它是16位，两个字节，无符号短整数   
    
  是UNICODE还是非UNICODE取决于你的编译选项，如果在[工程]-[选项]-[C/C++]的[预处理程序定义]中填入了_UNICODE，那么程序会用wchar_t指针来定义LPSTR，如果没有_UNICODE，那么程序会用char指针来定义LPSTR，这样带来的区别就是，你接受到的pText中的字节内容是不一样的，[i服了you]这个字串如果在没有定义_UNICODE的情况下，是8个字节，而在定义了_UNICODE的情况下是12个字节。反映到程序中就是，如果没有定义_UNICODE，那么就要把英文字符当成1个字节来处理，而汉字字符的编码是采用GB2312编码规范来的；如果定义了_UNICODE，那么英文字符要当成2个字节来处理，而汉字字符的编码是采用UNICODE编码来的。举例来说，win98不采用UNICODE编码而采用的是DBCS编码，为了让我的程序既可以在XP下运行又可以在Win98下运行，我没有定义_UNICODE。这样我的程序代码就要把字符串当成DBCS编码来处理，也就是英文字符是1个字节，中文字符是2个字节，中文编码采用GB2312编码。用Justify来说明：   
  我给pText传递来[你]这个汉字，那么pText应该有两个字节来存放[你]这个字，设置段点来读一下pText的内容。   
  void   Justify   (HDC   hdc,   PTSTR   pText,   RECT   *   prc,   int   iAlign)   
  {   
  static   TCHAR   szText[]   =   {TEXT   ("你")}   ;   
    
  pText=szText;   
  }   
  设断调试会发现*pText=-60，怎么会这样呢，原因是没有按unsigned   char来转换*pText的值，修改代码如下：   
  void   Justify   (HDC   hdc,   PTSTR   pText,   RECT   *   prc,   int   iAlign)   
  {   
  static   TCHAR   szText[]   =   {TEXT   ("你")}   ;   
  unsigned   char   sqChar[20]; //这个变量就是为了强制转换类型用的   
    
  pText=szText;   
  sqChar[0]=*pText;   
  sqChar[1]=*(pText+1);   
  }   
  这时下断查看sqChar[0]=196,sqChar[1]=227就对了，为什么呢，因为它和GB2312的编码是一样的。   
  GB2312-80编码的编码范围是高位0xa1－0xfe，低位是   0xa1-0xfe   ，其中汉字范围为   0xb0a1   和   0xf7fe，如果只是简单地判断汉字，则只要查看高字节是否大于等于0xa1就可以了，还有就是，全角字符的高字节统统等于0xa3，所以很容易可以区别出全角字符来。   
    
  如果你希望你的程序能支持到GB18030，那么就去找GB18030的规范来看看。   
    
  网上有很多判断汉字的说法，你只要记住，你要支持的编码是哪个？GB2312、GBK、GB18030？每个编码有自己的编码范围或者规范，网上之所以有不同的说法，正是因为他们互相说的不是同一种编码方式。我这里说的是GB2312的编码，如果你的程序要编译成支持UNICODE的话，那么这段代码就要修改成对应UNICODE规范的代码了。   
    
  附简单的测试汉字、全角字符、英文的程序：   
    
  //test.c   
  //源代码作者：夏克   sequh@126.com   
  //新建Win32   Application工程，把test.c加入，运行，试着修改szText的值，来观察代码效果   
    
  #include   <windows.h>   
    
  int   WINAPI   WinMain   (HINSTANCE   hInstance,   HINSTANCE   hPrevInstance,   
  PSTR   szCmdLine,   int   iCmdShow)   
  {   
  static   TCHAR   szText[]   =   {TEXT   ("i服,了。uｙ")}   ;   
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
  MessageBox   (NULL,   TEXT   ("发现数字、英文字符或英文标点"),   TEXT   ("Hello"),   0);   
  break;   
  case   1:   
  pText++;   
  pText++;   
  MessageBox   (NULL,   TEXT   ("发现全角字符"),   TEXT   ("Hello"),   0);   
  break;   
  case   2:   
  pText++;   
  pText++;   
  MessageBox   (NULL,   TEXT   ("发现汉字"),   TEXT   ("Hello"),   0);   
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
		return   1; //全角字符   
	else   
		return   2; //汉字   
  else   
  return   0; //英文、数字、英文标点   
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
	return   1; //全角字符   
else   
	return   2; //汉字   
else   
return   0; //英文、数字、英文标点   
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


 